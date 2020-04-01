/******************************************************************************
/ LivePresetsExtension
/
/ C++ wrapper around winapi TreeView, use TreeViewAdapter to customize
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

#ifndef LPE_TREEVIEW_H
#define LPE_TREEVIEW_H

#include <memory>
#ifdef _WIN32
    #include <Windows.h>
#else

#endif
#include <ui/base/TreeViewAdapter.h>
#include <ui/LivePresetsTreeAdapter.h>
#include <map>


class TreeView {
public:
    explicit TreeView(HWND hwnd);
    virtual ~TreeView() = default;

    HWND mHwnd;

    void invalidate();
    virtual int onNotify(WPARAM wParam, LPARAM lParam) { return 0; };
    int onKey(MSG* msg, int iKeyState);
    void setAdapter(std::unique_ptr<LivePresetsTreeAdapter> adapter);
    LivePresetsTreeAdapter* getAdapter();
    virtual void onDestroy() {};
private:
    //save HTREEITEMS for LPARAMS as a work around for missing TreeView_GetParent function on SWELL
    std::map<LPARAM, HTREEITEM> mTreeItems;
    std::unique_ptr<LivePresetsTreeAdapter> mAdapter = nullptr;
    void invalidateChilds(HTREEITEM parent);
    void addItem(TVITEM tvi, HTREEITEM parent, bool update = false);
};


#endif //LPE_TREEVIEW_H