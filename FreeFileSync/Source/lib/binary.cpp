// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "binary.h"
#include <vector>
#include <zen/tick_count.h>

using namespace zen;
using AFS = AbstractFileSystem;

namespace
{
/*
1. there seems to be no perf improvement possible when using file mappings instad of ::ReadFile() calls on Windows:
    => buffered   access: same perf
    => unbuffered access: same perf on USB stick, file mapping 30% slower on local disk

2. Tests on Win7 x64 show that buffer size does NOT matter if files are located on different physical disks!
Impact of buffer size when files are on same disk:

buffer  MB/s
------------
64      10
128     19
512     40
1024    48
2048    56
4096    56
8192    56
*/

const size_t BLOCK_SIZE_MAX =  16 * 1024 * 1024;
const std::int64_t TICKS_PER_SEC = ticksPerSec();


struct StreamReader
{
    StreamReader(const AbstractPath& filePath, const std::function<void(std::int64_t bytesDelta)>& notifyProgress, size_t& unevenBytes) :
        stream(AFS::getInputStream(filePath)), //throw FileError, (ErrorFileLocked)
        defaultBlockSize(stream->getBlockSize()),
        dynamicBlockSize(defaultBlockSize),
        notifyProgress_(notifyProgress),
        unevenBytes_(unevenBytes) {}

    void appendChunk(std::vector<char>& buffer) //throw FileError
    {
        assert(!eof);
        if (eof) return;

        const TickVal startTime = getTicks();

        buffer.resize(buffer.size() + dynamicBlockSize);
        const size_t bytesRead = stream->tryRead(&*(buffer.end() - dynamicBlockSize), dynamicBlockSize); //throw FileError; may return short, only 0 means EOF! => CONTRACT: bytesToRead > 0
        buffer.resize(buffer.size() - dynamicBlockSize + bytesRead); //caveat: unsigned arithmetics

        const TickVal stopTime = getTicks();

        //report bytes processed
        if (notifyProgress_)
        {
            const size_t bytesToReport = (unevenBytes_ + bytesRead) / 2;
            notifyProgress_(bytesToReport); //throw X!
            unevenBytes_ = (unevenBytes_ + bytesRead) - bytesToReport * 2; //unsigned arithmetics!
        }

        if (bytesRead == 0)
        {
            eof = true;
            return;
        }

        if (TICKS_PER_SEC > 0)
        {
            size_t proposedBlockSize = 0;
            const std::int64_t loopTimeMs = dist(startTime, stopTime) * 1000 / TICKS_PER_SEC; //unit: [ms]

            if (loopTimeMs >= 100)
                lastDelayViolation = stopTime;

            //avoid "flipping back": e.g. DVD-ROMs read 32MB at once, so first read may be > 500 ms, but second one will be 0ms!
            if (dist(lastDelayViolation, stopTime) / TICKS_PER_SEC >= 2)
            {
                lastDelayViolation = stopTime;
                proposedBlockSize = dynamicBlockSize * 2;
            }
            if (loopTimeMs > 500)
                proposedBlockSize = dynamicBlockSize / 2;

            if (defaultBlockSize <= proposedBlockSize && proposedBlockSize <= BLOCK_SIZE_MAX)
                dynamicBlockSize = proposedBlockSize;
        }
    }

    bool isEof() const { return eof; }

private:
    const std::unique_ptr<AFS::InputStream> stream;
    const size_t defaultBlockSize;
    size_t dynamicBlockSize;
    const std::function<void(std::int64_t bytesDelta)> notifyProgress_;
    size_t& unevenBytes_;
    TickVal lastDelayViolation = getTicks();
    bool eof = false;
};
}


bool zen::filesHaveSameContent(const AbstractPath& filePath1, const AbstractPath& filePath2, const std::function<void(std::int64_t bytesDelta)>& notifyProgress) //throw FileError
{
    size_t unevenBytes = 0;
    StreamReader reader1(filePath1, notifyProgress, unevenBytes); //throw FileError, (ErrorFileLocked)
    StreamReader reader2(filePath2, notifyProgress, unevenBytes); //

    StreamReader* readerLow  = &reader1;
    StreamReader* readerHigh = &reader2;

    std::vector<char> bufferLow;
    std::vector<char> bufferHigh;

    for (;;)
    {
        const size_t bytesChecked = bufferLow.size();

        readerLow->appendChunk(bufferLow); //throw FileError

        if (bufferLow.size() > bufferHigh.size())
        {
            bufferLow.swap(bufferHigh);
            std::swap(readerLow, readerHigh);
        }

        if (!std::equal(bufferLow. begin() + bytesChecked, bufferLow.end(),
                        bufferHigh.begin() + bytesChecked))
            return false;

        if (readerLow->isEof())
        {
            if (bufferLow.size() < bufferHigh.size())
                return false;
            if (readerHigh->isEof())
                break;
            //bufferLow.swap(bufferHigh); not needed
            std::swap(readerLow, readerHigh);
        }

        //don't let sliding buffer grow too large
        bufferHigh.erase(bufferHigh.begin(), bufferHigh.begin() + bufferLow.size());
        bufferLow.clear();
    }

    if (unevenBytes != 0)
        throw std::logic_error("Contract violation! " + std::string(__FILE__) + ":" + numberTo<std::string>(__LINE__));

    return true;
}
