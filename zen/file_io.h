// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef FILE_IO_H_89578342758342572345
#define FILE_IO_H_89578342758342572345

#include "file_error.h"
#include "serialize.h"

#ifdef ZEN_WIN
    #include "win.h" //includes "windows.h"
#endif


namespace zen
{
#ifdef ZEN_WIN
    static const char LINE_BREAK[] = "\r\n";
#elif defined ZEN_LINUX || defined ZEN_MAC
    static const char LINE_BREAK[] = "\n"; //since OS X apple uses newline, too
#endif

//OS-buffered file IO optimized for sequential read/write accesses + better error reporting + long path support + following symlinks

#ifdef ZEN_WIN
    using FileHandle = HANDLE;
#elif defined ZEN_LINUX || defined ZEN_MAC
    using FileHandle = int;
#endif

class FileBase
{
public:
    const Zstring& getFilePath() const { return filename_; }

protected:
    FileBase(const Zstring& filename) : filename_(filename)  {}

private:
    FileBase           (const FileBase&) = delete;
    FileBase& operator=(const FileBase&) = delete;

    const Zstring filename_;
};

//-----------------------------------------------------------------------------------------------

class FileInput : public FileBase
{
public:
    FileInput(const Zstring& filepath);                    //throw FileError, ErrorFileLocked
    FileInput(FileHandle handle, const Zstring& filepath); //takes ownership!
    ~FileInput();

    //Windows: better use 64kB ?? https://technet.microsoft.com/en-us/library/cc938632
    //Linux: use st_blksize?
    size_t getBlockSize() const { return 128 * 1024; }
    size_t tryRead(void* buffer, size_t bytesToRead); //throw FileError; may return short, only 0 means EOF! =>  CONTRACT: bytesToRead > 0!

    FileHandle getHandle() { return fileHandle; }

private:
    FileHandle fileHandle;
};


class FileOutput : public FileBase
{
public:
    enum AccessFlag
    {
        ACC_OVERWRITE,
        ACC_CREATE_NEW
    };

    FileOutput(const Zstring& filepath, AccessFlag access); //throw FileError, ErrorTargetExisting
    FileOutput(FileHandle handle, const Zstring& filepath); //takes ownership!
    ~FileOutput();

    FileOutput(FileOutput&& tmp);

    size_t getBlockSize() const { return 128 * 1024; }
    size_t tryWrite(const void* buffer, size_t bytesToWrite); //throw FileError; may return short! CONTRACT: bytesToWrite > 0

    void close(); //throw FileError   -> optional, but good place to catch errors when closing stream!
    FileHandle getHandle() { return fileHandle; }

private:
    FileHandle fileHandle;
};


//native stream I/O convenience functions:

template <class BinContainer> inline
BinContainer loadBinContainer(const Zstring& filePath, //throw FileError
                              const std::function<void(std::int64_t bytesDelta)>& notifyProgress) //optional
{
    FileInput streamIn(filePath); //throw FileError, ErrorFileLocked
    return unbufferedLoad<BinContainer>(streamIn, notifyProgress); //throw FileError
}


template <class BinContainer> inline
void saveBinContainer(const Zstring& filePath, const BinContainer& buffer, //throw FileError
                      const std::function<void(std::int64_t bytesDelta)>& notifyProgress) //optional
{
    FileOutput fileOut(filePath, FileOutput::ACC_OVERWRITE); //
    unbufferedSave(buffer, fileOut, notifyProgress);         //throw FileError
    fileOut.close();                                         //
}
}

#endif //FILE_IO_H_89578342758342572345
