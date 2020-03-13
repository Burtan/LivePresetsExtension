/******************************************************************************
/ LivePresetsExtension
/
/ C++ wrapper around winapi ListView, use ListViewAdapter to customize
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

#ifndef LPE_LISTVIEW_H
#define LPE_LISTVIEW_H

#include <memory>
#include <vector>
#include <functional>
#ifdef _WIN32
    #include <Windows.h>
#else

#endif

#include <set>
#include <algorithm>
#include <cstdlib> //needed for WDL/lineparse
#include <cstring> //needed for WDL/lineparse
#include <WDL/lineparse.h>
#include "plugins/reaper_plugin_functions.h"
#include "ui/base/ListViewAdapter.h"

template<typename T>
class ListView {
public:
    typedef std::function<void(NMLISTVIEW*)> Callback;

    explicit ListView(HWND hwnd);
    ~ListView();

    HWND mHwnd;

    void addListViewEventListener(const ListView::Callback & listener);
    void removeListViewEventListener(const ListView::Callback& listener);
    void invalidate();
    void selectIndex(int index);
    std::vector<int> getSelectedIndices();
    int onNotify(WPARAM, LPARAM lParam);
    void setAdapter(std::unique_ptr<ListViewAdapter<T>> adapter);
    ListViewAdapter<T>* getAdapter();
    void onDestroy();
private:
    std::unique_ptr<ListViewAdapter<T>> mAdapter = nullptr;
    //one-based column counter, negative value means reversed order
    int sortedColumnIndex = 0;
    std::vector<Callback> listeners;

    void updateColumns();
    void sortByColumn(int columnIndex);
};


#endif //LPE_LISTVIEW_H