/******************************************************************************
/ LivePresetsExtension
/
/ Adapter for Comboboxes
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

#ifndef LPE_COMBOBOXADAPTER_H
#define LPE_COMBOBOXADAPTER_H

#ifdef _WIN32
    #include <Windows.h>
#else
    #define WDL_NO_DEFINE_MINMAX
    #include <third_party/WDL/WDL/swell/swell-types.h>
    #include <third_party/WDL/WDL/swell/swell-functions.h>
#endif

#include <vector>
#include <utility>

/**
 * An extension class for ComboBox that defines its items and behaviour
 */
template<typename T>
class ComboBoxAdapter {
public:
    explicit ComboBoxAdapter(std::vector<T> items) : mItems(std::move(items)) {};
    std::vector<T> mItems;
};


#endif //LPE_COMBOBOXADAPTER_H