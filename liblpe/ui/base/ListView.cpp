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

#include <liblpe/ui/base/ListView.h>
#include <liblpe/data/models/LivePreset.h>
#include <liblpe/lpe_ultimate.h>

/**
 * A c++ wrapper class for winapi ListView. Use ListViewAdapter to customize
 * @param hwnd the handle of the winapi ListView
 */
template<typename T>
ListView<T>::ListView(HWND hwnd) : mHwnd(hwnd) {
    //style the list
    auto dw = LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
    ListView_SetExtendedListViewStyleEx(hwnd, dw, dw);
}
template ListView<LivePreset>::ListView(HWND hwnd);
template ListView<Control>::ListView(HWND hwnd);

template<typename T>
ListView<T>::~ListView() {
    //needed for mac to not crash
    mAdapter.release();
}
template ListView<LivePreset>::~ListView();
template ListView<Control>::~ListView();

/**
* Adds a callback to the ListView for different ui actions
* @param listener a reference to a callback.
*/
template<typename T>
void ListView<T>::addListViewEventListener(const ListView::Callback & listener) {
    listeners.push_back(listener);
}
template void ListView<LivePreset>::addListViewEventListener(const ListView::Callback & listener);
template void ListView<Control>::addListViewEventListener(const ListView::Callback & listener);

/**
 * Removes the callback from the ListView
 * @param listener a reference to the callback that was once added.
 */
template<typename T>
void ListView<T>::removeListViewEventListener(const ListView::Callback& listener) {
    auto functor = [listener](ListView::Callback& mListener) -> bool {
        return &mListener == &listener;
    };
    listeners.erase(remove_if(listeners.begin(), listeners.end(), functor), listeners.end());
}
template void ListView<LivePreset>::removeListViewEventListener(const ListView::Callback& listener);
template void ListView<Control>::removeListViewEventListener(const ListView::Callback& listener);

/**
 * Let the ListView pull all item updates from the adapter and update its ui.
 */
template<typename T>
void ListView<T>::invalidate() {
    if (!mAdapter) {
        ListView_DeleteAllItems(mHwnd);
        return;
    }
    mAdapter->filterAndSort();

    // check for changes in items. Old state is saved in ListView hwnd object, new state is saved in adapter

    std::vector<int> oldSelectedIndices = getSelectedIndices();
    std::set<T*> oldSelectedItems;
    for (auto oldSelectedIndex : oldSelectedIndices) {
        oldSelectedItems.insert(mAdapter->getItem(oldSelectedIndex));
    }

    std::set<T*> stayingItems;
    std::set<T*> deletedItems;

    for (auto i = 0; i < ListView_GetItemCount(mHwnd); i++) {
        LVITEM lvItem = {};
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = i;
        ListView_GetItem(mHwnd, &lvItem);

        T* item = (T*) lvItem.lParam;
        auto index = mAdapter->getIndex(item);

        if (index == -1) {
            // item was removed
            deletedItems.insert(item);
        } else {
            stayingItems.insert(item);
        }
    }

    //preset item count for better performance
    ListView_SetItemCount(mHwnd, mAdapter->getCount());

    //set values for all cells, index = row, colIndex = column
    for (int index = 0; index < mAdapter->getCount(); index++) {
        T* item = mAdapter->getItem(index);

        //colIndex 0 specifies the row lvItem
        LVITEM lvItem = mAdapter->getLvItem(index);
        lvItem.mask |= LVIF_STATE;
        lvItem.state = oldSelectedItems.find(item) != oldSelectedItems.end();
        lvItem.stateMask = LVIS_SELECTED;

        if (stayingItems.find(item) != stayingItems.end()) {
            ListView_SetItem(mHwnd, &lvItem);
        } else {
            ListView_InsertItem(mHwnd, &lvItem);
        }

        //colIndex 1+ specifies the subItems
        for (int colIndex = 0; colIndex < columns.size(); colIndex++) {
            const char* text = mAdapter->getLvItemText(index, colIndex);

#ifndef _WIN32
            ListView_SetItemText(mHwnd, index, colIndex, text);
#else
            ListView_SetItemText(mHwnd, index, colIndex, (char*) text);
#endif
        }
    }

    for (auto* item : deletedItems) {
        ListView_DeleteItem(mHwnd, ListView_GetItemCount(mHwnd) - 1);
    }
}
template void ListView<LivePreset>::invalidate();
template void ListView<Control>::invalidate();

template<typename T>
void ListView<T>::selectIndex(int index) {
    ListView_SetItemState(mHwnd, -1, 0, 1);
    ListView_SetItemState(mHwnd, index, LVIS_SELECTED, LVIS_SELECTED);
}
template void ListView<LivePreset>::selectIndex(int index);
template void ListView<Control>::selectIndex(int index);

/**
 * Returns the indices of the selected ListView rows.
 * @return vector of indices
 */
template<typename T>
std::vector<int> ListView<T>::getSelectedIndices() {
    std::vector<int> selectedIndices;
    if (!mAdapter)
        return selectedIndices;

    for (int index = 0; index < ListView_GetItemCount(mHwnd); index++) {
        if (ListView_GetItemState(mHwnd, index, LVIS_SELECTED)) {
            selectedIndices.push_back(index);
        }
    }
    return selectedIndices;
}
template std::vector<int> ListView<LivePreset>::getSelectedIndices();
template std::vector<int> ListView<Control>::getSelectedIndices();

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
template<typename T>
int ListView<T>::onNotify(WPARAM, LPARAM lParam) {
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
template int ListView<LivePreset>::onNotify(WPARAM, LPARAM lParam);
template int ListView<Control>::onNotify(WPARAM, LPARAM lParam);

/**
 * Sets the adapter for the ListView that defines behaviour and items.
 * @param adapter a unique pointer to the adapter, has to be called with std::move(adapter)
 */
template<typename T>
void ListView<T>::setAdapter(std::unique_ptr<ListViewAdapter<T>> adapter) {
    mAdapter.swap(adapter);
    updateColumns();
    sortByColumn(-1);
}
template void ListView<LivePreset>::setAdapter(std::unique_ptr<ListViewAdapter<LivePreset>> adapter);
template void ListView<Control>::setAdapter(std::unique_ptr<ListViewAdapter<Control>> adapter);

/**
 * Returns the current adapter of the ListView.
 * @return A pointer to the adapter whose life time is managed by the ListView.
 */
template<typename T>
ListViewAdapter<T>* ListView<T>::getAdapter() {
    return mAdapter.get();
}
template ListViewAdapter<LivePreset>* ListView<LivePreset>::getAdapter();
template ListViewAdapter<Control>* ListView<Control>::getAdapter();

/**
 * Should be called by the parent window onDestroy
 */
template<typename T>
void ListView<T>::onDestroy() {
    if (mAdapter) {
        //save settings
        mAdapter->saveColumnWidths(mHwnd);
        mAdapter->saveSortedColumnIndex(mHwnd, sortedColumnIndex);
    }
}
template void ListView<LivePreset>::onDestroy();
template void ListView<Control>::onDestroy();

/**
* Deletes all columns and then readds them according to the specified columns in the adapter.
*/
template<typename T>
void ListView<T>::updateColumns() {
    //remove all columns, when column 0 is removed, 1 becomes 0
    while (ListView_DeleteColumn(mHwnd, 0)) {}

    if (mAdapter) {
        // cache columns because adapter function reads filesystem!
        columns = mAdapter->getColumns();
        //add columns
        auto cols = columns;
        int index = 0;
        for (auto col : cols) {
            ListView_InsertColumn(mHwnd, index, &col);
            index++;
        }
    }
}
template void ListView<LivePreset>::updateColumns();
template void ListView<Control>::updateColumns();

/**
 * Updates the header of the selected column to display a sorting order and notifies the adapter to change its sorting.
 * @param columnIndex the index of the column to sort for starting at 0, -1 restores last saved sorting
 */
template<typename T>
void ListView<T>::sortByColumn(int columnIndex) {
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

        LVCOLUMN col = columns[columnIndex];
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
template void ListView<LivePreset>::sortByColumn(int columnIndex);
template void ListView<Control>::sortByColumn(int columnIndex);