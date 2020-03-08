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

    /**
     * A c++ wrapper class for winapi ListView. Use ListViewAdapter to customize
     * @param hwnd the handle of the winapi ListView
     */
    explicit ListView(HWND hwnd) : mHwnd(hwnd) {
        //style the list
        auto dw = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
        ListView_SetExtendedListViewStyleEx(hwnd, dw, dw);

/*    //set colors
    int sz;
    auto cTheme = (ColorTheme*) GetColorThemeStruct(&sz);
    if (!cTheme || sz < sizeof(ColorTheme))
        return;

    ListView_SetBkColor(hwnd, cTheme->genlist_bg);
    ListView_SetTextBkColor(hwnd, cTheme->genlist_bg);
    ListView_SetTextColor(hwnd, cTheme->genlist_fg);
#ifndef _WIN32
    ListView_SetGridColor(hwnd, cTheme->genlist_gridlines);
    int selColors[] = {
            cTheme->genlist_sel[0],
            cTheme->genlist_sel[1],
            cTheme->genlist_selinactive[0],
            cTheme->genlist_selinactive[1]
    };
    ListView_SetSelColors(hwnd, selColors, 4);
#endif*/
    }

    HWND mHwnd;

    /**
     * Adds a callback to the ListView for different ui actions
     * @param listener a reference to a callback.
     */
    void addListViewEventListener(const ListView::Callback & listener) {
        listeners.push_back(listener);
    }

    /**
     * Removes the callback from the ListView
     * @param listener a reference to the callback that was once added.
     */
    void removeListViewEventListener(const ListView::Callback& listener) {
        auto functor = [listener](ListView::Callback& mListener) -> bool {
            return &mListener == &listener;
        };
        listeners.erase(remove_if(listeners.begin(), listeners.end(), functor), listeners.end());
    }

    /**
     * Let the ListView pull all item updates from the adapter and update its ui.
     */
    void invalidate() {
        if (!mAdapter) {
            ListView_DeleteAllItems(mHwnd);
            return;
        }

        std::vector<int> selectedIndices = getSelectedIndices();
        std::set<void*> selectedItems;
        for (auto selectedIndex : selectedIndices) {
            selectedItems.insert(mAdapter->getItem(selectedIndex));
        }

        ListView_DeleteAllItems(mHwnd);
        mAdapter->filterAndSort();

        //preset item count for better performance
        ListView_SetItemCount(mHwnd, mAdapter->getCount());

        //set values for all cells, index = row, colIndex = column
        for (int index = 0; index < mAdapter->getCount(); index++) {
            void* item = mAdapter->getItem(index);
            //colIndex 0 specifies the row lvItem
            LVITEM lvItem = mAdapter->getLvItem(index);
            lvItem.mask |= LVIF_STATE;
            lvItem.iItem = index;
            lvItem.iSubItem = 0;
            lvItem.state = selectedItems.find(item) != selectedItems.end();
            lvItem.stateMask = LVIS_SELECTED;
            ListView_InsertItem(mHwnd, &lvItem);

            //colIndex 1+ specifies the subItems
            for (int colIndex = 0; colIndex < mAdapter->getColumns().size(); colIndex++) {
                const char* text = mAdapter->getLvItemText(index, colIndex);

#ifndef _WIN32
                ListView_SetItemText(mHwnd, index, colIndex, text);
#else
                ListView_SetItemText(mHwnd, index, colIndex, (char*) text);
#endif
            }
        }
    }

    void selectIndex(int index) {
        ListView_SetItemState(mHwnd, -1, 0, 1);
        ListView_SetItemState(mHwnd, index, LVIS_SELECTED, LVIS_SELECTED);
    }

    /**
     * Returns the indices of the selected ListView rows.
     * @return vector of indices
     */
    std::vector<int> getSelectedIndices() {
        std::vector<int> selectedIndices;
        if (!mAdapter)
            return selectedIndices;

        for (int index = 0; index < mAdapter->getCount(); index++) {
            if (ListView_GetItemState(mHwnd, index, LVIS_SELECTED)) {
                selectedIndices.push_back(index);
            }
        }
        return selectedIndices;
    }

    /**
     * Must be called by the parent window
     * @param wParam The identifier of the common control sending the message. This identifier is not guaranteed to be
     * unique. An application should use the hwndFrom or idFrom member of the NMHDR structure (passed as the lParam
     * parameter) to identify the control.
     * @param lParam A pointer to an NMHDR structure that contains the notification code and additional information. For
     * some notification messages, this parameter points to a larger structure that has the NMHDR structure as its first
     * member.
     * @return The return value is ignored except for notification messages that specify otherwise.
     */
    int onNotify(WPARAM, LPARAM lParam) {
        auto event = (NMLISTVIEW*) lParam;
        switch (event->hdr.code) {
            case NM_DBLCLK: {
                //forward to listeners
                for (auto& listener : listeners) {
                    listener.operator()(event);
                }
                break;
            }
            case LVN_ITEMCHANGED: {
                //selection changed
                for (auto& listener : listeners) {
                    listener.operator()(event);
                }
                break;
            }
            case LVN_COLUMNCLICK: {
                //click on a headers of a column
                sortByColumn(event->iSubItem);
                break;
            }
            case NM_RCLICK: {
                //right click on any place of the list view
                for (auto& listener : listeners) {
                    listener.operator()(event);
                }
                break;
            }
        }
        return 0;
    }

    /**
     * Sets the adapter for the ListView that defines behaviour and items.
     * @param adapter a unique pointer to the adapter, has to be called with std::move(adapter)
     */
    void setAdapter(std::unique_ptr<ListViewAdapter<T>> adapter) {
        mAdapter.swap(adapter);
        updateColumns();
        sortByColumn(-1);
    }

    /**
     * Returns the current adapter of the ListView.
     * @return A pointer to the adapter whose life time is managed by the ListView.
     */
    ListViewAdapter<T>* getAdapter() {
        return mAdapter.get();
    }

    /**
     * Should be called by the parent window onDestroy
     */
    void onDestroy() {
        if (mAdapter) {
            //save settings
            mAdapter->saveColumnWidths(mHwnd);
            mAdapter->saveSortedColumnIndex(mHwnd, sortedColumnIndex);
        }
    }
private:
    std::unique_ptr<ListViewAdapter<T>> mAdapter = nullptr;
    //one-based column counter, negative value means reversed order
    int sortedColumnIndex = 0;

    /**
     * Deletes all columns and then readds them according to the specified columns in the adapter.
     */
    void updateColumns() {
        //remove all columns, when column 0 is removed, 1 becomes 0
        while (ListView_DeleteColumn(mHwnd, 0)) {}

        if (mAdapter) {
            //add columns
            auto cols = mAdapter->getColumns();
            int index = 0;
            for (auto col : cols) {
                ListView_InsertColumn(mHwnd, index, &col);
                index++;
            }
        }
    }

    /**
     * Updates the header of the selected column to display a sorting order and notifies the adapter to change its sorting.
     * @param columnIndex the index of the column to sort for starting at 0, -1 restores last saved sorting
     */
    void sortByColumn(int columnIndex) {
        if (mAdapter) {

            if (columnIndex == -1) {
                //restore saved sorting
                char str[256];
                GetPrivateProfileString("LPE", "PresetsListSortedColumnIndex", "2", str, 4096, get_ini_file());
                LineParser lp;
                lp.parse(str);
                sortedColumnIndex = lp.gettoken_int(0);

                columnIndex = abs(sortedColumnIndex) - 1;
            } else {
                //determine sorting
                int oneBasedColumnIndex = columnIndex + 1;
                if (oneBasedColumnIndex == abs(sortedColumnIndex)) {
                    sortedColumnIndex = -sortedColumnIndex;
                } else {
                    sortedColumnIndex = -oneBasedColumnIndex;
                }
            }

            LVCOLUMN col = mAdapter->getColumns()[columnIndex];
            col.iSubItem = columnIndex;

            //change ui, draw arrows on sorted column, remove arrows from the rest
            HWND header = ListView_GetHeader(mHwnd);
            for (int i = 0; i < Header_GetItemCount(header); i++) {
                HDITEM hi = { HDI_FORMAT, 0, };
                Header_GetItem(header, i, &hi);

                //remove arrows
                hi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);

                //add arrows on sorted column
                if (abs(sortedColumnIndex) == i + 1) {
                    hi.fmt |= (sortedColumnIndex > 0 ? HDF_SORTUP : HDF_SORTDOWN);
                }
                Header_SetItem(header, i, &hi);
            }

            //perform sorting
            mAdapter->onChangedSortingColumn(col, sortedColumnIndex < 0);
            invalidate();
        }
    }

    std::vector<Callback> listeners;
};


#endif //LPE_LISTVIEW_H