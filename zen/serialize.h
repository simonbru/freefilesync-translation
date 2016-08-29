// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef SERIALIZE_H_839405783574356
#define SERIALIZE_H_839405783574356

#include <functional>
#include <cstdint>
#include "string_base.h"
//keep header clean from specific stream implementations! (e.g.file_io.h)! used by abstract.h!


namespace zen
{
//high-performance unformatted serialization (avoiding wxMemoryOutputStream/wxMemoryInputStream inefficiencies)

/*
--------------------------
|Binary Container Concept|
--------------------------
binary container for data storage: must support "basic" std::vector interface (e.g. std::vector<char>, std::string, Zbase<char>)
*/

//binary container reference implementations
using Utf8String = Zbase<char>; //ref-counted + COW text stream + guaranteed performance: exponential growth
class ByteArray;                //ref-counted       byte stream + guaranteed performance: exponential growth -> no COW, but 12% faster than Utf8String (due to no null-termination?)


class ByteArray //essentially a std::vector<char> with ref-counted semantics, but no COW! => *almost* value type semantics, but not quite
{
public:
    using value_type     = std::vector<char>::value_type;
    using iterator       = std::vector<char>::iterator;
    using const_iterator = std::vector<char>::const_iterator;

    iterator begin() { return buffer->begin(); }
    iterator end  () { return buffer->end  (); }

    const_iterator begin() const { return buffer->begin(); }
    const_iterator end  () const { return buffer->end  (); }

    void resize(size_t len) { buffer->resize(len); }
    size_t size() const { return buffer->size(); }
    bool  empty() const { return buffer->empty(); }

    inline friend bool operator==(const ByteArray& lhs, const ByteArray& rhs) { return *lhs.buffer == *rhs.buffer; }

private:
    std::shared_ptr<std::vector<char>> buffer { std::make_shared<std::vector<char>>() }; //always bound!
    //perf: shared_ptr indirection irrelevant: less than 1% slower!
};

/*
---------------------------------
|Unbuffered Input Stream Concept|
---------------------------------
struct UnbufferedInputStream
{
    size_t getBlockSize();
    size_t tryRead(void* buffer, size_t bytesToRead); //may return short, only 0 means EOF! => CONTRACT: bytesToRead > 0
};

----------------------------------
|Unbuffered Output Stream Concept|
----------------------------------
struct UnbufferedOutputStream
{
    size_t getBlockSize();
    size_t tryWrite(const void* buffer, size_t bytesToWrite); //may return short! CONTRACT: bytesToWrite > 0
};
*/
//functions based on unbuffered stream abstraction

template <class UnbufferedInputStream, class UnbufferedOutputStream>
void unbufferedStreamCopy(UnbufferedInputStream& streamIn, UnbufferedOutputStream& streamOut, const std::function<void(std::int64_t bytesDelta)>& notifyProgress); //throw X

template <class BinContainer, class UnbufferedOutputStream>
void unbufferedSave(const BinContainer& buffer, UnbufferedOutputStream& streamOut, const std::function<void(std::int64_t bytesDelta)>& notifyProgress); //throw X

template <class BinContainer, class UnbufferedInputStream>
BinContainer unbufferedLoad(UnbufferedInputStream& streamIn,                       const std::function<void(std::int64_t bytesDelta)>& notifyProgress); //throw X

/*
-------------------------------
|Buffered Input Stream Concept|
-------------------------------
struct BufferedInputStream
{
    size_t read(void* buffer, size_t bytesToRead); //return "len" bytes unless end of stream! throw ?
};

--------------------------------
|Buffered Output Stream Concept|
--------------------------------
struct BufferedOutputStream
{
    void write(const void* buffer, size_t bytesToWrite); //throw ?
};
*/
//functions based on buffered stream abstraction
template <class N, class BufferedOutputStream> void writeNumber   (BufferedOutputStream& stream, const N& num);                 //
template <class C, class BufferedOutputStream> void writeContainer(BufferedOutputStream& stream, const C& str);                 //throw ()
template <         class BufferedOutputStream> void writeArray    (BufferedOutputStream& stream, const void* data, size_t len); //

//----------------------------------------------------------------------
class UnexpectedEndOfStreamError {};
template <class N, class BufferedInputStream> N    readNumber   (BufferedInputStream& stream); //throw UnexpectedEndOfStreamError (corrupted data)
template <class C, class BufferedInputStream> C    readContainer(BufferedInputStream& stream); //
template <         class BufferedInputStream> void readArray    (BufferedInputStream& stream, void* data, size_t len); //

//buffered input/output stream reference implementations:
template <class BinContainer>
struct MemoryStreamIn
{
    MemoryStreamIn(const BinContainer& cont) : buffer(cont) {} //this better be cheap!

    size_t read(void* data, size_t len) //return "len" bytes unless end of stream!
    {
        static_assert(sizeof(typename BinContainer::value_type) == 1, ""); //expect: bytes
        const size_t bytesRead = std::min(len, buffer.size() - pos);
        auto itFirst = buffer.begin() + pos;
        std::copy(itFirst, itFirst + bytesRead, static_cast<char*>(data));
        pos += bytesRead;
        return bytesRead;
    }

private:
    const BinContainer buffer;
    size_t pos = 0;
};

template <class BinContainer>
struct MemoryStreamOut
{
    void write(const void* data, size_t len)
    {
        static_assert(sizeof(typename BinContainer::value_type) == 1, ""); //expect: bytes
        const size_t oldSize = buffer.size();
        buffer.resize(oldSize + len);
        std::copy(static_cast<const char*>(data), static_cast<const char*>(data) + len, buffer.begin() + oldSize);
    }

    const BinContainer& ref() const { return buffer; }

private:
    BinContainer buffer;
};








//-----------------------implementation-------------------------------
template <class UnbufferedInputStream, class UnbufferedOutputStream> inline
void unbufferedStreamCopy(UnbufferedInputStream& streamIn,   //throw X
                          UnbufferedOutputStream& streamOut, //
                          const std::function<void(std::int64_t bytesDelta)>& notifyProgress) //optional
{
    size_t unevenBytes = 0;
    auto reportBytesProcessed = [&](size_t bytesReadOrWritten)
    {
        if (notifyProgress)
        {
            const size_t bytesToReport = (unevenBytes + bytesReadOrWritten) / 2;
            notifyProgress(bytesToReport); //throw X!
            unevenBytes = (unevenBytes + bytesReadOrWritten) - bytesToReport * 2; //unsigned arithmetics!
        }
    };

    const size_t blockSizeIn  = streamIn .getBlockSize();
    const size_t blockSizeOut = streamOut.getBlockSize();
    if (blockSizeIn == 0 || blockSizeOut == 0)
        throw std::logic_error("Contract violation! " + std::string(__FILE__) + ":" + numberTo<std::string>(__LINE__));

    std::vector<char> buffer;
    for (;;)
    {
        buffer.resize(buffer.size() + blockSizeIn);
        const size_t bytesRead = streamIn.tryRead(&*(buffer.end() - blockSizeIn), blockSizeIn); //throw X; may return short, only 0 means EOF! => CONTRACT: bytesToRead > 0
        buffer.resize(buffer.size() - blockSizeIn + bytesRead); //caveat: unsigned arithmetics

        reportBytesProcessed(bytesRead); //throw X!

        size_t bytesRemaining = buffer.size();
        while (bytesRemaining >= blockSizeOut)
        {
            const size_t bytesWritten = streamOut.tryWrite(&*(buffer.end() - bytesRemaining), blockSizeOut); //throw X; may return short! CONTRACT: bytesToWrite > 0
            bytesRemaining -= bytesWritten;
            reportBytesProcessed(bytesWritten); //throw X!
        }
        buffer.erase(buffer.begin(), buffer.end() - bytesRemaining);

        if (bytesRead == 0) //end of file
            break;
    }

    for (size_t bytesRemaining = buffer.size(); bytesRemaining > 0;)
    {
        const size_t bytesWritten = streamOut.tryWrite(&*(buffer.end() - bytesRemaining), bytesRemaining); //throw X; may return short! CONTRACT: bytesToWrite > 0
        bytesRemaining -= bytesWritten;
        reportBytesProcessed(bytesWritten); //throw X!
    }

    if (unevenBytes != 0)
        throw std::logic_error("Contract violation! " + std::string(__FILE__) + ":" + numberTo<std::string>(__LINE__));
}


template <class BinContainer, class UnbufferedOutputStream> inline
void unbufferedSave(const BinContainer& buffer,
                    UnbufferedOutputStream& streamOut, //throw X
                    const std::function<void(std::int64_t bytesDelta)>& notifyProgress) //optional
{
    const size_t blockSize = streamOut.getBlockSize();
    if (blockSize == 0)
        throw std::logic_error("Contract violation! " + std::string(__FILE__) + ":" + numberTo<std::string>(__LINE__));

    static_assert(sizeof(typename BinContainer::value_type) == 1, ""); //expect: bytes

    for (size_t bytesRemaining = buffer.size(); bytesRemaining > 0;)
    {
        const size_t bytesToWrite = std::min(bytesRemaining, blockSize);
        const size_t bytesWritten = streamOut.tryWrite(&*(buffer.end() - bytesRemaining), bytesToWrite); //throw X; may return short! CONTRACT: bytesToWrite > 0
        bytesRemaining -= bytesWritten;
        if (notifyProgress) notifyProgress(bytesWritten); //throw X!
    }
}


template <class BinContainer, class UnbufferedInputStream> inline
BinContainer unbufferedLoad(UnbufferedInputStream& streamIn, //throw X
                            const std::function<void(std::int64_t bytesDelta)>& notifyProgress) //optional
{
    const size_t blockSize = streamIn.getBlockSize();
    if (blockSize == 0)
        throw std::logic_error("Contract violation! " + std::string(__FILE__) + ":" + numberTo<std::string>(__LINE__));

    static_assert(sizeof(typename BinContainer::value_type) == 1, ""); //expect: bytes

    BinContainer buffer;
    for (;;)
    {
        buffer.resize(buffer.size() + blockSize);
        const size_t bytesRead = streamIn.tryRead(&*(buffer.end() - blockSize), blockSize); //throw X; may return short, only 0 means EOF! => CONTRACT: bytesToRead > 0
        buffer.resize(buffer.size() - blockSize + bytesRead); //caveat: unsigned arithmetics

        if (notifyProgress) notifyProgress(bytesRead); //throw X!

        if (bytesRead == 0) //end of file
            return buffer;
    }
}


template <class BufferedOutputStream> inline
void writeArray(BufferedOutputStream& stream, const void* data, size_t len)
{
    stream.write(data, len);
}


template <class N, class BufferedOutputStream> inline
void writeNumber(BufferedOutputStream& stream, const N& num)
{
    static_assert(IsArithmetic<N>::value || IsSameType<N, bool>::value, "not a number!");
    writeArray(stream, &num, sizeof(N));
}


template <class C, class BufferedOutputStream> inline
void writeContainer(BufferedOutputStream& stream, const C& cont) //don't even consider UTF8 conversions here, we're handling arbitrary binary data!
{
    const auto len = cont.size();
    writeNumber(stream, static_cast<std::uint32_t>(len));
    if (len > 0)
        writeArray(stream, &*cont.begin(), sizeof(typename C::value_type) * len); //don't use c_str(), but access uniformly via STL interface
}


template <class BufferedInputStream> inline
void readArray(BufferedInputStream& stream, void* data, size_t len) //throw UnexpectedEndOfStreamError
{
    const size_t bytesRead = stream.read(data, len);
    if (bytesRead < len)
        throw UnexpectedEndOfStreamError();
}


template <class N, class BufferedInputStream> inline
N readNumber(BufferedInputStream& stream) //throw UnexpectedEndOfStreamError
{
    static_assert(IsArithmetic<N>::value || IsSameType<N, bool>::value, "");
    N num = 0;
    readArray(stream, &num, sizeof(N)); //throw UnexpectedEndOfStreamError
    return num;
}


template <class C, class BufferedInputStream> inline
C readContainer(BufferedInputStream& stream) //throw UnexpectedEndOfStreamError
{
    C cont;
    auto strLength = readNumber<std::uint32_t>(stream);
    if (strLength > 0)
    {
        try
        {
            cont.resize(strLength); //throw std::bad_alloc
        }
        catch (std::bad_alloc&) //most likely this is due to data corruption!
        {
            throw UnexpectedEndOfStreamError();
        }
        readArray(stream, &*cont.begin(), sizeof(typename C::value_type) * strLength); //throw UnexpectedEndOfStreamError
    }
    return cont;
}
}

#endif //SERIALIZE_H_839405783574356
