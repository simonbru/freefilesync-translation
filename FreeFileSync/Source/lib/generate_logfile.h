// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef GENERATE_LOGFILE_H_931726432167489732164
#define GENERATE_LOGFILE_H_931726432167489732164

#include <zen/error_log.h>
#include <zen/file_io.h>
#include <zen/format_unit.h>
#include "ffs_paths.h"
#include "../fs/abstract.h"


namespace zen
{
struct SummaryInfo
{
    std::wstring jobName; //may be empty
    std::wstring finalStatus;
    int itemsSynced;
    std::int64_t dataSynced;
    int itemsTotal;
    std::int64_t dataTotal;
    int64_t totalTime; //unit: [sec]
};

void streamToLogFile(const SummaryInfo& summary, //throw FileError
                     const ErrorLog& log,
                     FileOutput& fileOut,
                     const std::function<void(std::int64_t bytesDelta)>& onUpdateSaveStatus);

void saveToLastSyncsLog(const SummaryInfo& summary,  //throw FileError
                        const ErrorLog& log,
                        size_t maxBytesToWrite,
                        const std::function<void(std::int64_t bytesDelta)>& onUpdateSaveStatus);

Zstring getLastSyncsLogfilePath();



struct OnUpdateLogfileStatusNoThrow
{
    OnUpdateLogfileStatusNoThrow(ProcessCallback& pc, const std::wstring& logfileDisplayPath) : pc_(pc),
        msg(replaceCpy(_("Saving file %x..."), L"%x", fmtPath(logfileDisplayPath))) {}

    void operator()(std::int64_t bytesDelta)
    {
        bytesWritten += bytesDelta;
        try { pc_.reportStatus(msg + L" (" + filesizeToShortString(bytesWritten) + L")"); /*throw X*/ }
        catch (...) {}
    }

private:
    ProcessCallback& pc_;
    std::int64_t bytesWritten = 0;
    const std::wstring msg;
};



//####################### implementation #######################
namespace
{
std::wstring generateLogHeader(const SummaryInfo& s)
{
    assert(s.itemsSynced <= s.itemsTotal);
    assert(s.dataSynced  <= s.dataTotal);

    std::wstring output;

    //write header
    std::wstring headerLine = formatTime<std::wstring>(FORMAT_DATE);
    if (!s.jobName.empty())
        headerLine += L" - " + s.jobName;
    headerLine += L": " + s.finalStatus;

    //assemble results box
    std::vector<std::wstring> results;
    results.push_back(headerLine);
    results.push_back(L"");

    const wchar_t tabSpace[] = L"    ";

    std::wstring itemsProc = tabSpace + _("Items processed:") + L" " + toGuiString(s.itemsSynced); //show always, even if 0!
    if (s.itemsSynced != 0 || s.dataSynced != 0) //[!] don't show 0 bytes processed if 0 items were processed
        itemsProc += + L" (" + filesizeToShortString(s.dataSynced) + L")";
    results.push_back(itemsProc);

    if (s.itemsTotal != 0 || s.dataTotal != 0) //=: sync phase was reached and there were actual items to sync
    {
        if (s.itemsSynced != s.itemsTotal ||
            s.dataSynced  != s.dataTotal)
            results.push_back(tabSpace + _("Items remaining:") + L" " + toGuiString(s.itemsTotal - s.itemsSynced) + L" (" + filesizeToShortString(s.dataTotal - s.dataSynced) + L")");
    }

    results.push_back(tabSpace + _("Total time:") + L" " + copyStringTo<std::wstring>(wxTimeSpan::Seconds(s.totalTime).Format()));

    //calculate max width, this considers UTF-16 only, not true Unicode...but maybe good idea? those 2-char-UTF16 codes are usually wider than fixed width chars anyway!
    size_t sepLineLen = 0;
    for (const std::wstring& str : results) sepLineLen = std::max(sepLineLen, str.size());

    output.resize(output.size() + sepLineLen + 1, L'_');
    output += L'\n';

    for (const std::wstring& str : results) { output += L'|'; output += str; output += L'\n'; }

    output += L'|';
    output.resize(output.size() + sepLineLen, L'_');
    output += L'\n';

    return output;
}
}


inline
void streamToLogFile(const SummaryInfo& summary, //throw FileError
                     const ErrorLog& log,
                     AFS::OutputStream& streamOut,
                     const std::function<void(std::int64_t bytesDelta)>& notifyProgress)
{
    //write log items in blocks instead of creating one big string: memory allocation might fail; think 1 million entries!
    const size_t blockSize = streamOut.getBlockSize();
    std::string buffer;

    auto flushBlock = [&]
    {
        size_t bytesRemaining = buffer.size();
        while (bytesRemaining >= blockSize)
        {
            const size_t bytesWritten = streamOut.tryWrite(&*(buffer.end() - bytesRemaining), blockSize); //throw FileError; may return short! CONTRACT: bytesToWrite > 0
            bytesRemaining -= bytesWritten;
            if (notifyProgress) notifyProgress(bytesWritten); //throw X!
        }
        buffer.erase(buffer.begin(), buffer.end() - bytesRemaining);
    };

    buffer += replaceCpy(utfCvrtTo<std::string>(generateLogHeader(summary)), '\n', LINE_BREAK); //don't replace line break any earlier
    buffer += LINE_BREAK;

    for (const LogEntry& entry : log)
    {
        buffer += replaceCpy(utfCvrtTo<std::string>(formatMessage<std::wstring>(entry)), '\n', LINE_BREAK);
        buffer += LINE_BREAK;
        flushBlock(); //throw FileError
    }
    unbufferedSave(buffer, streamOut, notifyProgress); //throw FileError
}


inline
Zstring getLastSyncsLogfilePath() { return getConfigDir() + Zstr("LastSyncs.log"); }


inline
void saveToLastSyncsLog(const SummaryInfo& summary, //throw FileError
                        const ErrorLog& log,
                        size_t maxBytesToWrite, //log may be *huge*, e.g. 1 million items; LastSyncs.log *must not* create performance problems!
                        const std::function<void(std::int64_t bytesDelta)>& onUpdateSaveStatus)
{
    const Zstring filepath = getLastSyncsLogfilePath();

    Utf8String newStream = utfCvrtTo<Utf8String>(generateLogHeader(summary));
    replace(newStream, '\n', LINE_BREAK); //don't replace line break any earlier
    newStream += LINE_BREAK;

    //check size of "newStream": memory allocation might fail - think 1 million entries!
    for (const LogEntry& entry : log)
    {
        newStream += replaceCpy(utfCvrtTo<Utf8String>(formatMessage<std::wstring>(entry)), '\n', LINE_BREAK);
        newStream += LINE_BREAK;

        if (newStream.size() > maxBytesToWrite)
        {
            newStream += "[...]";
            newStream += LINE_BREAK;
            break;
        }
    }

    //fill up the rest of permitted space by appending old log
    if (newStream.size() < maxBytesToWrite)
    {
        Utf8String oldStream;
        try
        {
            oldStream = loadBinContainer<Utf8String>(filepath, onUpdateSaveStatus); //throw FileError
            //Note: we also report the loaded bytes via onUpdateSaveStatus()!
        }
        catch (FileError&) {}

        if (!oldStream.empty())
        {
            newStream += LINE_BREAK;
            newStream += LINE_BREAK;
            newStream += oldStream; //impliticly limited by "maxBytesToWrite"!

            //truncate size if required
            if (newStream.size() > maxBytesToWrite)
            {
                //but do not cut in the middle of a row
                auto it = std::search(newStream.cbegin() + maxBytesToWrite, newStream.cend(), std::begin(LINE_BREAK), std::end(LINE_BREAK) - 1);
                if (it != newStream.cend())
                {
                    newStream.resize(it - newStream.cbegin());
                    newStream += LINE_BREAK;

                    newStream += "[...]";
                    newStream += LINE_BREAK;
                }
            }
        }
    }

    saveBinContainer(filepath, newStream, onUpdateSaveStatus); //throw FileError
}
}

#endif //GENERATE_LOGFILE_H_931726432167489732164
