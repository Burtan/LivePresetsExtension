/******************************************************************************
/ LivePresetsListView.cpp
/ listview to show all presets
/
/ Copyright (c) 2019 and later Frederik
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

#include <liblpe/ui/ControlsListAdapter.h>
#include <cstdlib> //needed for WDL/lineparse
#include <cstring> //needed for WDL/lineparse
#include <algorithm>
#include <wdlstring.h>
#include <lineparse.h>
#include <reaper_plugin_functions.h>

enum COLUMN {
    NAME = 0
};

int ControlsListAdapter::getCount() {
    return mShowingItems.size();
}

Control* ControlsListAdapter::getItem(int index) {
    return mShowingItems.at(index);
}

int ControlsListAdapter::getIndex(Control* item) {
    auto index = std::find(mShowingItems.begin(), mShowingItems.end(), item);
    return std::distance(mShowingItems.begin(), index);
}

LVITEM ControlsListAdapter::getLvItem(int index) {
    auto preset = mShowingItems.at(index);
    LVITEM item;
    item.mask = LVIF_PARAM;
    item.lParam = (LPARAM) preset;
    return item;
}

const char* ControlsListAdapter::getLvItemText(int index, int column) {
    auto control = mShowingItems.at(index);

    switch (column) {
        default:
        case COLUMN::NAME: {
            return control->getName(index);
        }
    }
}

std::vector<LVCOLUMN> ControlsListAdapter::getColumns() {
    //Get saved settings
    char str[256];
    GetPrivateProfileString("LPE", "ControlsListColumns", "328", str, 256, get_ini_file());
    LineParser lp;
    lp.parse(str);

#ifdef _WIN32
    unsigned int mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
#else
    auto mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
#endif

    std::vector<LVCOLUMN> cols = {
            {mask, LVCFMT_CENTER, lp.gettoken_int(0), (char*) "Name"},
    };
    return cols;
}

void ControlsListAdapter::onChangedSortingColumn(LVCOLUMN col, bool reverse) {
    switch (col.iSubItem) {
        case COLUMN::NAME:
            if (reverse) {
                setComparator([](Control* a, Control* b) -> bool {
                    return 1;
                });
            } else {
                setComparator([](Control* a, Control* b) -> bool {
                    return 1;
                });
            }
            break;

        default: {
            setComparator(nullptr);
        }
    }
}

void ControlsListAdapter::saveColumnWidths(HWND hwnd) {
    auto str = WDL_FastString();
    for (int i = 0; i < (int) getColumns().size(); i++) {
        str.AppendFormatted(4096, "%i ", ListView_GetColumnWidth(hwnd, i));
    }

    WritePrivateProfileString("LPE", "ControlsListColumn", str.Get(), get_ini_file());
}

void ControlsListAdapter::saveSortedColumnIndex(HWND hwnd, int index) {
    auto str = WDL_FastString();
    str.AppendFormatted(4096, "%i", index);

    WritePrivateProfileString("LPE", "ControlsListSortedColumnIndex", str.Get(), get_ini_file());
}
