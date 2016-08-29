// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef HTTP_h_879083425703425702
#define HTTP_h_879083425703425702

#include <zen/file_error.h>

namespace zen
{
/*
    TREAD-SAFETY
    ------------
    Windows: WinInet-based   => may be called from worker thread
    Linux:   wxWidgets-based => don't call from worker thread
*/
std::string sendHttpPost(const std::wstring& url, const std::wstring& userAgent, const std::vector<std::pair<std::string, std::string>>& postParams); //throw FileError
std::string sendHttpGet (const std::wstring& url, const std::wstring& userAgent); //throw FileError
bool internetIsAlive(); //noexcept
}

#endif //HTTP_h_879083425703425702
