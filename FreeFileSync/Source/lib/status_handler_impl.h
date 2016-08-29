// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef STATUS_HANDLER_IMPL_H_07682758976
#define STATUS_HANDLER_IMPL_H_07682758976

#include <zen/optional.h>
#include <zen/file_error.h>
#include "../process_callback.h"


namespace zen
{
template <typename Function> inline
zen::Opt<std::wstring> tryReportingError(Function cmd, ProcessCallback& handler) //throw X?; return ignored error message if available
{
    for (size_t retryNumber = 0;; ++retryNumber)
        try
        {
            cmd(); //throw FileError
            return zen::NoValue();
        }
        catch (zen::FileError& error)
        {
            switch (handler.reportError(error.toString(), retryNumber)) //throw ?
            {
                case ProcessCallback::IGNORE_ERROR:
                    return error.toString();
                case ProcessCallback::RETRY:
                    break; //continue with loop
            }
        }
}


//manage statistics reporting for a single item of work
class StatisticsReporter
{
public:
    StatisticsReporter(int itemsExpected, std::int64_t bytesExpected, ProcessCallback& cb) :
        itemsExpected_(itemsExpected),
        bytesExpected_(bytesExpected),
        cb_(cb) {}

    ~StatisticsReporter()
    {
        if (taskCancelled)
            cb_.updateTotalData(itemsReported, bytesReported); //=> unexpected increase of total workload
        else
            //update statistics to consider the real amount of data, e.g. more than the "file size" for ADS streams,
            //less for sparse and compressed files,  or file changed in the meantime!
            cb_.updateTotalData(itemsReported - itemsExpected_, bytesReported - bytesExpected_); //noexcept!
    }

    void reportDelta(int itemsDelta, std::int64_t bytesDelta) //may throw!
    {
        cb_.updateProcessedData(itemsDelta, bytesDelta); //nothrow! -> ensure client and service provider are in sync!
        itemsReported += itemsDelta;
        bytesReported += bytesDelta;                      //

        //special rule: avoid temporary statistics mess up, even though they are corrected anyway below:
        if (itemsReported > itemsExpected_)
        {
            cb_.updateTotalData(itemsReported - itemsExpected_, 0);
            itemsReported = itemsExpected_;
        }
        if (bytesReported > bytesExpected_)
        {
            cb_.updateTotalData(0, bytesReported - bytesExpected_); //=> everything above "bytesExpected" adds to both "processed" and "total" data
            bytesReported = bytesExpected_;
        }

        cb_.requestUiRefresh(); //may throw!
    }

    void reportFinished() //nothrow!
    {
        assert(taskCancelled);
        taskCancelled = false;
    }

private:
    bool taskCancelled = true;
    int itemsReported = 0;
    std::int64_t bytesReported = 0;
    const int itemsExpected_;
    const std::int64_t bytesExpected_;
    ProcessCallback& cb_;
};
}

#endif //STATUS_HANDLER_IMPL_H_07682758976
