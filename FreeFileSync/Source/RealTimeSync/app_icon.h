// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef APP_ICON_H_8914578394545342
#define APP_ICON_H_8914578394545342

#include <wx/icon.h>
#include <wx+/image_resources.h>

namespace zen
{
inline
wxIcon getRtsIcon()
{
    //wxWidgets' bitmap to icon conversion on OS X can only deal with very specific sizes => check on all platforms!
    assert(getResourceImage(L"RealTimeSync").GetWidth () == getResourceImage(L"RealTimeSync").GetHeight() &&
           getResourceImage(L"RealTimeSync").GetWidth() % 128 == 0);
#ifdef ZEN_WIN
    //for compatibility it seems we need to stick with a "real" icon
    return wxIcon(L"A_RTS_ICON");

#elif defined ZEN_LINUX
    //attention: make sure to not implicitly call "instance()" again => deadlock on Linux
    wxIcon icon;
    icon.CopyFromBitmap(getResourceImage(L"RealTimeSync")); //use big logo bitmap for better quality
    return icon;

#elif defined ZEN_MAC
    wxIcon icon;
    icon.CopyFromBitmap(getResourceImage(L"RealTimeSync").ConvertToImage().Scale(128, 128, wxIMAGE_QUALITY_HIGH)); //"von hinten durch die Brust ins Auge"
    return icon;
#endif
}
}


#endif //APP_ICON_H_8914578394545342
