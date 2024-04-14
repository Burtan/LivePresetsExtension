/******************************************************************************
/ LivePresetsExtension
/
/ Adapter for TreeView
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

#ifndef LPE_TREEVIEWADAPTER_H
#define LPE_TREEVIEWADAPTER_H

#include <vector>
#ifdef _WIN32
    #include <Windows.h>
    #include <CommCtrl.h>
#else
    #define WDL_NO_DEFINE_MINMAX
    #include <third_party/WDL/WDL/swell/swell-types.h>
#endif
/**
 * An extension class for TreeView that defines its items and behaviour
 */
class TreeViewAdapter {
public:
    explicit TreeViewAdapter() = default;

    virtual void onAction(HWND hwnd, HTREEITEM item) = 0;
    virtual std::vector<TVITEM> getChilds(TVITEM* parent) = 0;
};


#endif //LPE_TREEVIEWADAPTER_H