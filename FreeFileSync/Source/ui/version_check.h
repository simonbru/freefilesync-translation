// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef VERSION_CHECK_H_324872374893274983275
#define VERSION_CHECK_H_324872374893274983275

#include <functional>
#include <memory>
#include <wx/window.h>


namespace zen
{
bool updateCheckActive (time_t  lastUpdateCheck);
void disableUpdateCheck(time_t& lastUpdateCheck);
bool haveNewerVersionOnline(const std::wstring& onlineVersion);

//periodic update check:
bool shouldRunPeriodicUpdateCheck(time_t lastUpdateCheck);

struct UpdateCheckResultPrep;
struct UpdateCheckResultAsync;

//run on main thread:
std::shared_ptr<UpdateCheckResultPrep> periodicUpdateCheckPrepare();
//run on worker thread: (long-running part of the check)
std::shared_ptr<UpdateCheckResultAsync> periodicUpdateCheckRunAsync(const UpdateCheckResultPrep* resultPrep);
//run on main thread:
void periodicUpdateCheckEval(wxWindow* parent, time_t& lastUpdateCheck,
                             std::wstring& lastOnlineVersion,
                             std::wstring& lastOnlineChangeLog,
                             const UpdateCheckResultAsync* resultAsync);

//----------------------------------------------------------------------------

//call from main thread:
void checkForUpdateNow(wxWindow* parent, std::wstring& lastOnlineVersion, std::wstring& lastOnlineChangeLog);
}

#endif //VERSION_CHECK_H_324872374893274983275
