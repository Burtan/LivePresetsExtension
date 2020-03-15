/******************************************************************************
/ LivePresetsExtension
/
/ Some util functions
/
/ Copyright (c) 2020 and later Dr. med. Frederik Bertling
/
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/

#include <algorithm>
#include "util/util.h"

bool GuidsEqual(const GUID g1, const GUID g2) {
    return g1.Data1 == g2.Data1 &&
           g1.Data2 == g2.Data2 &&
           g1.Data3 == g2.Data3 &&
           std::equal(std::begin(g1.Data4), std::end(g1.Data4), std::begin(g2.Data4));
}

MediaTrack* GetTrackByGUID(GUID g1) {
    for (int i = 0; i < GetNumTracks(); i++) {
        MediaTrack* track = GetTrack(nullptr, i);
        if (GuidsEqual(*GetTrackGUID(track), g1)) {
            return track;
        }
    }
    return nullptr;
}

void AddControl(HWND parentHwnd, const char *name, int id, const char *widget, int style, int exStyle,
                int x, int y, int width, int height, bool adjustDpi) {

    //TODO get dpi scaling
    int dpiFactor = adjustDpi ? 4 : 1;

#ifdef _WIN32
    auto hwnd = CreateWindowEx(
            exStyle, widget, name, style,
            x * dpiFactor, y * dpiFactor, width * dpiFactor, height * dpiFactor,
            parentHwnd, nullptr, nullptr, nullptr
    );
    SetWindowLongPtrA(hwnd, GWL_ID, id);
#else
    SWELL_MakeSetCurParms(1, 1, 0, 0, parentHwnd, false, false);
    SWELL_MakeControl(
            name, id, widget, style,
            x * dpiFactor, y * dpiFactor, width * dpiFactor, height * dpiFactor,
            exStyle
    );
    SWELL_MakeSetCurParms(1, 1, 0, 0, nullptr, false, false);
#endif
}

void RemoveControl(HWND hwnd) {
#ifdef _WIN32
    CloseWindow(hwnd);
#else
    SWELL_CloseWindow(hwnd);
#endif
}

void GuidToInts(GUID g1, int inOut[4]) {
    inOut[0] = g1.Data1;
    inOut[1] = ((g1.Data2 << 16) | ((g1.Data3 & 0xFFF)));
}

GUID IntsToGuid(int i1, int i2, int i3, int i4) {
    unsigned data1 = i1;
    unsigned short data2 = LOWORD(i2);
    unsigned short data3 = HIWORD(i2);
    char data4[8];

    return GUID{data1, data2, data3, data4};
}