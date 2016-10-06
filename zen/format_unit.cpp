// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#include "format_unit.h"
#include <cwchar> //swprintf
#include <ctime>
#include <cstdio>
#include "basic_math.h"
#include "i18n.h"
#include "time.h"
#include "globals.h"

#ifdef ZEN_WIN
    #include "int64.h"
    #include "win.h" //includes "windows.h"
    #include "win_ver.h"

#elif defined ZEN_LINUX || defined ZEN_MAC
    #include <clocale> //thousands separator
    #include "utf.h"   //
#endif

using namespace zen;


std::wstring zen::formatTwoDigitPrecision(double value)
{
    //print two digits: 0,1 | 1,1 | 11
    if (numeric::abs(value) < 9.95) //9.99 must not be formatted as "10.0"
        return printNumber<std::wstring>(L"%.1f", value);
    return numberTo<std::wstring>(numeric::round(value));
}


std::wstring zen::formatThreeDigitPrecision(double value)
{
    //print three digits: 0,01 | 0,11 | 1,11 | 11,1 | 111
    if (numeric::abs(value) < 9.995) //9.999 must not be formatted as "10.00"
        return printNumber<std::wstring>(L"%.2f", value);
    if (numeric::abs(value) < 99.95) //99.99 must not be formatted as "100.0"
        return printNumber<std::wstring>(L"%.1f", value);
    return numberTo<std::wstring>(numeric::round(value));
}


std::wstring zen::filesizeToShortString(std::int64_t size)
{
    //if (size < 0) return _("Error"); -> really?

    if (numeric::abs(size) <= 999)
        return _P("1 byte", "%x bytes", static_cast<int>(size));

    double sizeInUnit = static_cast<double>(size);

    auto formatUnit = [&](const std::wstring& unitTxt) { return replaceCpy(unitTxt, L"%x", formatThreeDigitPrecision(sizeInUnit)); };

    sizeInUnit /= 1024;
    if (numeric::abs(sizeInUnit) < 999.5)
        return formatUnit(_("%x KB"));

    sizeInUnit /= 1024;
    if (numeric::abs(sizeInUnit) < 999.5)
        return formatUnit(_("%x MB"));

    sizeInUnit /= 1024;
    if (numeric::abs(sizeInUnit) < 999.5)
        return formatUnit(_("%x GB"));

    sizeInUnit /= 1024;
    if (numeric::abs(sizeInUnit) < 999.5)
        return formatUnit(_("%x TB"));

    sizeInUnit /= 1024;
    return formatUnit(_("%x PB"));
}


namespace
{
enum UnitRemTime
{
    URT_SEC,
    URT_MIN,
    URT_HOUR,
    URT_DAY
};


std::wstring formatUnitTime(int val, UnitRemTime unit)
{
    switch (unit)
    {
        case URT_SEC:
            return _P("1 sec", "%x sec", val);
        case URT_MIN:
            return _P("1 min", "%x min", val);
        case URT_HOUR:
            return _P("1 hour", "%x hours", val);
        case URT_DAY:
            return _P("1 day", "%x days", val);
    }
    assert(false);
    return _("Error");
}


template <int M, int N>
std::wstring roundToBlock(double timeInHigh,
                          UnitRemTime unitHigh, const int (&stepsHigh)[M],
                          int unitLowPerHigh,
                          UnitRemTime unitLow, const int (&stepsLow)[N])
{
    assert(unitLowPerHigh > 0);
    const double granularity = 0.1;
    const double timeInLow = timeInHigh * unitLowPerHigh;
    const int blockSizeLow = granularity * timeInHigh < 1 ?
                             numeric::nearMatch(granularity * timeInLow,  std::begin(stepsLow),  std::end(stepsLow)):
                             numeric::nearMatch(granularity * timeInHigh, std::begin(stepsHigh), std::end(stepsHigh)) * unitLowPerHigh;
    const int roundedtimeInLow = numeric::round(timeInLow / blockSizeLow) * blockSizeLow;

    std::wstring output = formatUnitTime(roundedtimeInLow / unitLowPerHigh, unitHigh);
    if (unitLowPerHigh > blockSizeLow)
        output += L" " + formatUnitTime(roundedtimeInLow % unitLowPerHigh, unitLow);
    return output;
};
}


std::wstring zen::remainingTimeToString(double timeInSec)
{
    const int steps10[] = { 1, 2, 5, 10 };
    const int steps24[] = { 1, 2, 3, 4, 6, 8, 12, 24 };
    const int steps60[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

    //determine preferred unit
    double timeInUnit = timeInSec;
    if (timeInUnit <= 60)
        return roundToBlock(timeInUnit, URT_SEC, steps60, 1, URT_SEC, steps60);

    timeInUnit /= 60;
    if (timeInUnit <= 60)
        return roundToBlock(timeInUnit, URT_MIN, steps60, 60, URT_SEC, steps60);

    timeInUnit /= 60;
    if (timeInUnit <= 24)
        return roundToBlock(timeInUnit, URT_HOUR, steps24, 60, URT_MIN, steps60);

    timeInUnit /= 24;
    return roundToBlock(timeInUnit, URT_DAY, steps10, 24, URT_HOUR, steps24);
    //note: for 10% granularity steps10 yields a valid blocksize only up to timeInUnit == 100!
    //for larger time sizes this results in a finer granularity than expected: 10 days -> should not be a problem considering "usual" remaining time for synchronization
}


std::wstring zen::fractionToString(double fraction)
{
    return printNumber<std::wstring>(L"%.2f", fraction * 100.0) + L'%'; //no need to internationalize fraction!?
}


#ifdef ZEN_WIN
namespace
{
class IntegerFormat
{
public:
    static std::shared_ptr<const IntegerFormat> instance()
    {
        static Global<const IntegerFormat> inst(std::make_unique<const IntegerFormat>());
        return inst.get();
    }

    bool isValid() const { return valid; }
    const NUMBERFMT& get() const { return fmt; }

    IntegerFormat()
    {
        //all we want is default NUMBERFMT, but set NumDigits to 0
        fmt.NumDigits = 0;

        //what a disgrace:
        std::wstring grouping;
        if (getUserSetting(LOCALE_ILZERO,     fmt.LeadingZero) &&
            getUserSetting(LOCALE_SGROUPING,  grouping)        &&
            getUserSetting(LOCALE_SDECIMAL,   decimalSep)      &&
            getUserSetting(LOCALE_STHOUSAND,  thousandSep)     &&
            getUserSetting(LOCALE_INEGNUMBER, fmt.NegativeOrder))
        {
            fmt.lpDecimalSep  = &decimalSep[0]; //don't need it
            fmt.lpThousandSep = &thousandSep[0];

            //convert LOCALE_SGROUPING to Grouping: https://blogs.msdn.microsoft.com/oldnewthing/20060418-11/?p=31493/
            replace(grouping, L';', L"");
            if (endsWith(grouping, L'0'))
                grouping.pop_back();
            else
                grouping += L'0';
            fmt.Grouping = stringTo<UINT>(grouping);
            valid = true;
        }
    }

private:
    IntegerFormat           (const IntegerFormat&) = delete;
    IntegerFormat& operator=(const IntegerFormat&) = delete;

    static bool getUserSetting(LCTYPE lt, UINT& setting)
    {
        return ::GetLocaleInfo(LOCALE_USER_DEFAULT,                  //__in   LCID Locale,
                               lt | LOCALE_RETURN_NUMBER,            //__in   LCTYPE LCType,
                               reinterpret_cast<LPTSTR>(&setting),   //__out  LPTSTR lpLCData,
                               sizeof(setting) / sizeof(TCHAR)) > 0; //__in   int cchData
    }
	
    static bool getUserSetting(LCTYPE lt, std::wstring& setting)
    {
        const int bufferSize = ::GetLocaleInfo(LOCALE_USER_DEFAULT, lt, nullptr, 0);
        if (bufferSize > 0)
        {
            std::vector<wchar_t> buffer(bufferSize);
            if (::GetLocaleInfo(LOCALE_USER_DEFAULT, //__in   LCID Locale,
                                lt,                  //__in   LCTYPE LCType,
                                &buffer[0],          //__out  LPTSTR lpLCData,
                                bufferSize) > 0)     //__in   int cchData
            {
                setting = &buffer[0]; //GetLocaleInfo() returns char count *including* 0-termination!
                return true;
            }
        }
        return false;
    }

    NUMBERFMT fmt = {};
    std::wstring thousandSep;
    std::wstring decimalSep;
    bool valid = false;
};
}
#endif


std::wstring zen::ffs_Impl::includeNumberSeparator(const std::wstring& number)
{
#ifdef ZEN_WIN
    if (std::shared_ptr<const IntegerFormat> fmt = IntegerFormat::instance())
        if (fmt->isValid())
        {
            const int bufferSize = ::GetNumberFormat(LOCALE_USER_DEFAULT, 0, number.c_str(), &fmt->get(), nullptr, 0);
            if (bufferSize > 0)
            {
                std::vector<wchar_t> buffer(bufferSize);
                if (::GetNumberFormat(LOCALE_USER_DEFAULT, //__in       LCID Locale,
                                      0,                   //__in       DWORD dwFlags,
                                      number.c_str(),      //__in       LPCTSTR lpValue,
                                      &fmt->get(),         //__in_opt   const NUMBERFMT *lpFormat,
                                      &buffer[0],          //__out_opt  LPTSTR lpNumberStr,
                                      bufferSize) > 0)     //__in       int cchNumber
                    return &buffer[0]; //GetNumberFormat() returns char count *including* 0-termination!
            }
        }
	assert(false); //what's the problem?
    return number;

#elif defined ZEN_LINUX || defined ZEN_MAC
    //we have to include thousands separator ourselves; this doesn't work for all countries (e.g india), but is better than nothing

    //::setlocale (LC_ALL, ""); -> implicitly called by wxLocale
    const lconv* localInfo = ::localeconv(); //always bound according to doc
    const std::wstring& thousandSep = utfCvrtTo<std::wstring>(localInfo->thousands_sep);

    // THOUSANDS_SEPARATOR = std::use_facet<std::numpunct<wchar_t>>(std::locale("")).thousands_sep(); - why not working?
    // DECIMAL_POINT       = std::use_facet<std::numpunct<wchar_t>>(std::locale("")).decimal_point();

    std::wstring output(number);
    size_t i = output.size();
    for (;;)
    {
        if (i <= 3)
            break;
        i -= 3;
        if (!isDigit(output[i - 1])) //stop on +, - signs
            break;
        output.insert(i, thousandSep);
    }
    return output;
#endif
}


std::wstring zen::utcToLocalTimeString(std::int64_t utcTime)
{
    auto errorMsg = [&] { return _("Error") + L" (time_t: " + numberTo<std::wstring>(utcTime) + L")"; };

#ifdef ZEN_WIN
    const FILETIME lastWriteTimeUtc = timetToFileTime(utcTime); //convert ansi C time to FILETIME

    SYSTEMTIME systemTimeLocal = {};

    static const bool useNewLocalTimeCalculation = zen::vistaOrLater();

    //https://msdn.microsoft.com/en-us/library/ms724277
    if (useNewLocalTimeCalculation) //DST conversion  like in Windows 7: NTFS stays fixed, but FAT jumps by one hour
    {
        SYSTEMTIME systemTimeUtc = {};
        if (!::FileTimeToSystemTime(&lastWriteTimeUtc, //__in   const FILETIME *lpFileTime,
                                    &systemTimeUtc))   //__out  LPSYSTEMTIME lpSystemTime
            return errorMsg();

        if (!::SystemTimeToTzSpecificLocalTime(nullptr,           //__in_opt  LPTIME_ZONE_INFORMATION lpTimeZone,
                                               &systemTimeUtc,    //__in      LPSYSTEMTIME lpUniversalTime,
                                               &systemTimeLocal)) //__out     LPSYSTEMTIME lpLocalTime
            return errorMsg();
    }
    else //DST conversion like in Windows 2000 and XP: FAT times stay fixed, while NTFS jumps
    {
        FILETIME fileTimeLocal = {};
        if (!::FileTimeToLocalFileTime(&lastWriteTimeUtc, //_In_   const FILETIME *lpFileTime,
                                       &fileTimeLocal))   //_Out_  LPFILETIME lpLocalFileTime
            return errorMsg();

        if (!::FileTimeToSystemTime(&fileTimeLocal,    //__in   const FILETIME *lpFileTime,
                                    &systemTimeLocal)) //__out  LPSYSTEMTIME lpSystemTime
            return errorMsg();
    }

    zen::TimeComp loc;
    loc.year   = systemTimeLocal.wYear;
    loc.month  = systemTimeLocal.wMonth;
    loc.day    = systemTimeLocal.wDay;
    loc.hour   = systemTimeLocal.wHour;
    loc.minute = systemTimeLocal.wMinute;
    loc.second = systemTimeLocal.wSecond;

#elif defined ZEN_LINUX || defined ZEN_MAC
    zen::TimeComp loc = zen::localTime(utcTime);
#endif

    std::wstring dateString = formatTime<std::wstring>(L"%x  %X", loc);
    return !dateString.empty() ? dateString : errorMsg();
}


#ifdef ZEN_WIN_VISTA_AND_LATER
Opt<std::int64_t> zen::mtpVariantTimetoUtc(double localVarTime) //returns empty on error
{
    SYSTEMTIME localSystemTime = {};
    if (!::VariantTimeToSystemTime(localVarTime,      //_In_  DOUBLE       vtime,
                                   &localSystemTime)) //_Out_ LPSYSTEMTIME lpSystemTime
        return NoValue();

    /*
    Windows Explorer isn't even consistent within itself: the modification time shown in the details list is calculated differently than the
    one shown in MTP file properties => different result shown for files from a DST interval distinct from the current one.
    -> Variant 1 matches the calculation of Explorer's details list and produces stable results irrespective of currently selected DST
    -> Explorer uses different algorithms for MTP and FAT file systems! FAT's local time jumps between DST intervals in Explorer (since Vista)!
    */
#if 1
    SYSTEMTIME utcSystemTime = {};
    if (!::TzSpecificLocalTimeToSystemTime(nullptr,          //_In_opt_ LPTIME_ZONE_INFORMATION lpTimeZoneInformation,
                                           &localSystemTime, //_In_     LPSYSTEMTIME            lpLocalTime,
                                           &utcSystemTime))  //_Out_    LPSYSTEMTIME            lpUniversalTime
        return NoValue();

    FILETIME utcFiletime = {};
    if (!::SystemTimeToFileTime(&utcSystemTime, //_In_  const SYSTEMTIME *lpSystemTime,
                                &utcFiletime))  //_Out_       LPFILETIME lpFileTime
        return NoValue();

#else
    FILETIME localFiletime = {};
    if (!::SystemTimeToFileTime(&localSystemTime, //_In_  const SYSTEMTIME *lpSystemTime,
                                &localFiletime))  //_Out_       LPFILETIME lpFileTime
        return NoValue();

    FILETIME utcFiletime = {};
    if (!LocalFileTimeToFileTime(&localFiletime, //_In_  const FILETIME   *lpLocalFileTime,
                                 &utcFiletime))  //_Out_       LPFILETIME lpFileTime
        return NoValue();

#endif
    return filetimeToTimeT(utcFiletime);
}


Opt<double> zen::utcToMtpVariantTime(std::int64_t utcTime) //returns empty on error
{
    const FILETIME lastWriteTimeUtc = timetToFileTime(utcTime); //convert ansi C time to FILETIME

    SYSTEMTIME systemTimeUtc = {};
    if (!::FileTimeToSystemTime(&lastWriteTimeUtc, //__in   const FILETIME *lpFileTime,
                                &systemTimeUtc))   //__out  LPSYSTEMTIME lpSystemTime
        return NoValue();

    SYSTEMTIME systemTimeLocal = {};
    if (!::SystemTimeToTzSpecificLocalTime(nullptr,           //__in_opt  LPTIME_ZONE_INFORMATION lpTimeZone,
                                           &systemTimeUtc,    //__in      LPSYSTEMTIME lpUniversalTime,
                                           &systemTimeLocal)) //__out     LPSYSTEMTIME lpLocalTime
        return NoValue();

    double localVarTime = 0;
    if (!::SystemTimeToVariantTime(&systemTimeLocal, //_In_  LPSYSTEMTIME lpSystemTime,
                                   &localVarTime))   //_Out_ DOUBLE       *pvtime
        return NoValue();

    return localVarTime;
}
#endif