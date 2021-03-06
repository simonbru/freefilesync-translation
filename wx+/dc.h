// *****************************************************************************
// * This file is part of the FreeFileSync project. It is distributed under    *
// * GNU General Public License: http://www.gnu.org/licenses/gpl-3.0           *
// * Copyright (C) Zenju (zenju AT freefilesync DOT org) - All Rights Reserved *
// *****************************************************************************

#ifndef DC_H_4987123956832143243214
#define DC_H_4987123956832143243214

#include <unordered_map>
#include <zen/optional.h>
#include <wx/dcbuffer.h> //for macro: wxALWAYS_NATIVE_DOUBLE_BUFFER

namespace zen
{
/*
1. wxDCClipper does *not* stack: another fix for yet another poor wxWidgets implementation

class RecursiveDcClipper
{
    RecursiveDcClipper(wxDC& dc, const wxRect& r) : dc_(dc)
};

------------------------------------------------------------------------------------------------

2. wxAutoBufferedPaintDC skips one pixel on left side when RTL layout is active: a fix for a poor wxWidgets implementation
class BufferedPaintDC
{
    BufferedPaintDC(wxWindow& wnd, std::unique_ptr<wxBitmap>& buffer);
};
*/











//---------------------- implementation ------------------------
class RecursiveDcClipper
{
public:
    RecursiveDcClipper(wxDC& dc, const wxRect& r) : dc_(dc)
    {
        auto it = refDcToAreaMap().find(&dc);
        if (it != refDcToAreaMap().end())
        {
            oldRect = it->second;

            wxRect tmp = r;
            tmp.Intersect(*oldRect);    //better safe than sorry
            dc_.SetClippingRegion(tmp); //
            it->second = tmp;
        }
        else
        {
            dc_.SetClippingRegion(r);
            refDcToAreaMap().emplace(&dc_, r);
        }
    }

    ~RecursiveDcClipper()
    {
        dc_.DestroyClippingRegion();
        if (oldRect)
        {
            dc_.SetClippingRegion(*oldRect);
            refDcToAreaMap()[&dc_] = *oldRect;
        }
        else
            refDcToAreaMap().erase(&dc_);
    }

private:
    //associate "active" clipping area with each DC
    static std::unordered_map<wxDC*, wxRect>& refDcToAreaMap() { static std::unordered_map<wxDC*, wxRect> clippingAreas; return clippingAreas; }

    Opt<wxRect> oldRect;
    wxDC& dc_;
};


#ifndef wxALWAYS_NATIVE_DOUBLE_BUFFER
    #error we need this one!
#endif

#if wxALWAYS_NATIVE_DOUBLE_BUFFER
struct BufferedPaintDC : public wxPaintDC { BufferedPaintDC(wxWindow& wnd, Opt<wxBitmap>& buffer) : wxPaintDC(&wnd) {} };

#else
class BufferedPaintDC : public wxMemoryDC
{
public:
    BufferedPaintDC(wxWindow& wnd, Opt<wxBitmap>& buffer) : buffer_(buffer), paintDc(&wnd)
    {
        const wxSize clientSize = wnd.GetClientSize();
        if (clientSize.GetWidth() > 0 && clientSize.GetHeight() > 0) //wxBitmap asserts this!! width may be 0; test case "Grid::CornerWin": compare both sides, then change config
        {
            if (!buffer_ || clientSize != wxSize(buffer->GetWidth(), buffer->GetHeight()))
                buffer = wxBitmap(clientSize.GetWidth(), clientSize.GetHeight());

            SelectObject(*buffer);

            if (paintDc.IsOk() && paintDc.GetLayoutDirection() == wxLayout_RightToLeft)
                SetLayoutDirection(wxLayout_RightToLeft);
        }
        else
            buffer = NoValue();
    }

    ~BufferedPaintDC()
    {
        if (buffer_)
        {
            if (GetLayoutDirection() == wxLayout_RightToLeft)
            {
                paintDc.SetLayoutDirection(wxLayout_LeftToRight); //workaround bug in wxDC::Blit()
                SetLayoutDirection(wxLayout_LeftToRight);         //
            }

            const wxPoint origin = GetDeviceOrigin();
            paintDc.Blit(0, 0, buffer_->GetWidth(), buffer_->GetHeight(), this, -origin.x, -origin.y);
        }
    }

private:
    Opt<wxBitmap>& buffer_;
    wxPaintDC paintDc;
};
#endif
}

#endif //DC_H_4987123956832143243214
