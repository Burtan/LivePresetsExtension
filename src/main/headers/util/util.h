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

#ifndef LPE_UTIL_H
#define LPE_UTIL_H

#include <iostream>
#include <set>
#include <vector>
#ifdef _WIN32
    #include <guiddef.h>
#else
    #include <WDL/swell/swell-types.h>
#endif
#include <plugins/reaper_plugin_functions.h>


// Stuff to do in swell someday
#ifndef _WIN32
    #define LVKF_ALT 1
    #define LVKF_CONTROL 2
    #define LVKF_SHIFT 4
#endif

bool GuidsEqual(GUID g1, GUID g2);
MediaTrack* GetTrackByGUID(GUID g1);
void AddControl(HWND parentHwnd, const char* name, int id, const char* widget, int style, int exStyle,
                int x, int y, int width, int height, bool adjustDpi = true);
void RemoveControl(HWND hwnd);

template<typename T>
void getPointerVector(std::vector<T>& base, std::vector<T*>& out) {
    for (auto& t : base) {
        out.push_back(&t);
    }
}

template<typename T, typename I>
void compareVectors(std::vector<T>& as, std::vector<I>& bs, std::vector<T>& uniqueAs, std::vector<I>& uniqueBs,
                    std::vector<std::pair<T, I>>& equals, bool (*compare)(T&, I&))
{
    //iterate through all a's and compare to all b's
    //a's that are NOT found in b's are going to uniqueAs and removed from a's
    //a's that are found in b's go to equals and removed from a's and b's
    //b's that are left go to uniqueBs and are removed from b's
    for (int i = 0; i < as.size(); i++) {
        int index = -1;
        auto a = as[i];
        for (int j = 0; j < bs.size(); j++) {
            auto b = bs[j];
            if (compare(a, b)) {
                index = j;
                equals.push_back(std::pair<T, I>(std::move(a), std::move(b)));
                goto foundEqual;
            }
        }
        uniqueAs.push_back(std::move(a));
        continue;
        foundEqual:;
        bs.erase(bs.begin() + index);
    }
    as.clear();
    for (auto itBs = bs.begin(); itBs != bs.end(); itBs++) {
        auto b = *itBs;
        uniqueBs.push_back(std::move(b));
    }
    bs.clear();
}

template<typename T, typename I>
void compareLists(std::vector<T>& as, std::vector<I>& bs, std::vector<T>& uniqueAs, std::vector<I>& uniqueBs,
                  std::vector<std::pair<T, I>>& equals, bool (*compare)(T, I))
{
    static auto cmp = compare;
    bool (*inverseCmp)(T&, I&) = [](T& a, I& b) -> bool {
        return cmp(*b, *a);
    };

    compareVectors(as, bs, uniqueAs, uniqueBs, equals, *inverseCmp);
}


#endif //LPE_UTIL_H