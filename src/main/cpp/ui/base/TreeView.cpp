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

#include "ui/base/TreeView.h"
#include "ui/LivePresetsTreeAdapter.h"

TreeView::TreeView(HWND hwnd) : mHwnd(hwnd) {

/*    //set colors
    int sz;
    auto cTheme = (ColorTheme*) GetColorThemeStruct(&sz);
    if (!cTheme || sz < sizeof(ColorTheme))
        return;

    TreeView_SetBkColor(hwnd, cTheme->genlist_bg);
    TreeView_SetTextColor(hwnd, cTheme->genlist_fg);*/
}

void TreeView::invalidate() {
    TreeView_DeleteAllItems(mHwnd);

    if (!mAdapter) {
        return;
    }

    for (auto child : mAdapter->getChilds(nullptr)) {
        addItem(child, TVI_ROOT);
    }
}

void TreeView::addItem(TVITEM tvi, HTREEITEM parent) {
    TV_INSERTSTRUCT info;
    info.hInsertAfter = TVI_LAST;
    info.hParent = parent;
    auto itemHwnd = TreeView_InsertItem(mHwnd, &info);

    auto childs = mAdapter->getChilds(&tvi);
    tvi.mask |= TVIF_HANDLE | TVIF_CHILDREN;
    tvi.hItem = itemHwnd;
    tvi.cChildren = childs.size();
    TreeView_SetItem(mHwnd, &tvi);

    for (auto child : childs) {
        addItem(child, itemHwnd);
    }
}

int TreeView::onKey(MSG* msg, int iKeyState) {
    if (msg->message == WM_KEYDOWN) {
        if (!iKeyState) {
            auto selected = TreeView_GetSelection(mHwnd);
            switch(msg->wParam) {
                case VK_RETURN: {
                    if (mAdapter) {
                        mAdapter->onAction(mHwnd, selected);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/**
 * Sets the adapter for the TreeView that defines behaviour and items.
 * @param adapter a unique pointer to the adapter, has to be called with std::move(adapter)
 */
void TreeView::setAdapter(std::unique_ptr<LivePresetsTreeAdapter> adapter) {
    mAdapter.swap(adapter);
    invalidate();
}

/**
 * Returns the current adapter of the TreeView.
 * @return A pointer to the adapter whose life time is managed by the TreeView.
 */
LivePresetsTreeAdapter* TreeView::getAdapter() {
    return mAdapter.get();
}