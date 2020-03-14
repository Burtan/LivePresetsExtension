/******************************************************************************
/ LivePresetsExtension
/
/ List adapter to show all presets
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

#include <ui/LivePresetsListAdapter.h>
#include <algorithm>
#include <plugins/reaper_plugin_functions.h>
#include <LivePresetsExtension.h>

enum COLUMN {
    ACTIVE = 0,
    RECALLID = 1,
    NAME = 2,
    DESCRIPTION = 3,
    TIME = 4
};

int LivePresetsListAdapter::getCount() {
    return mShowingItems.size();
}

LivePreset* LivePresetsListAdapter::getItem(int index) {
    if (index >= 0 && index < mShowingItems.size())
        return mShowingItems.at(index);
    
    return nullptr;
}

int LivePresetsListAdapter::getIndex(LivePreset* item) {
    auto index = std::find(mShowingItems.begin(), mShowingItems.end(), item);
    return std::distance(mShowingItems.begin(), index);
}

LVITEM LivePresetsListAdapter::getLvItem(int index) {
    auto preset = mShowingItems.at(index);
    LVITEM item;
    item.mask = LVIF_PARAM;
    item.lParam = (LPARAM) preset;
    return item;
}

const char* LivePresetsListAdapter::getLvItemText(int index, int column) {
    auto preset = mShowingItems.at(index);

    switch (column) {
        default:
        case COLUMN::ACTIVE: {
            if (g_lpe->mModel.getActivePreset()) {
                bool isActive = g_lpe->mModel.getActivePreset() == preset;
                if (isActive) {
                    return (char*) "X";
                }
            }
            return (char*) "";
        }
        case COLUMN::RECALLID:
            return std::to_string(preset->mRecallId).data();
        case COLUMN::NAME:
            return preset->mName.data();
        case COLUMN::DESCRIPTION:
            return preset->mDescription.data();
        case COLUMN::TIME:
            return ctime(&preset->mDate);
    }
}

std::vector<LVCOLUMN> LivePresetsListAdapter::getColumns() {
    //Get saved settings
    char str[256];
    GetPrivateProfileString("LPE", "PresetsListColumns", "200 200 200 200 200", str, 256, get_ini_file());
    LineParser lp;
    lp.parse(str);

#ifdef _WIN32
    unsigned int mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
#else
    auto mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
#endif

    std::vector<LVCOLUMN> cols = {
            {mask, LVCFMT_CENTER, lp.gettoken_int(0), (char*) ""},
            {mask, LVCFMT_LEFT, lp.gettoken_int(1), (char*) "#"},
            {mask, LVCFMT_LEFT, lp.gettoken_int(2), (char*) "Preset name"},
            {mask, LVCFMT_LEFT, lp.gettoken_int(3), (char*) "Description"},
            {mask, LVCFMT_LEFT, lp.gettoken_int(4), (char*) "Date"}
    };
    return cols;
}

void LivePresetsListAdapter::onChangedSortingColumn(LVCOLUMN col, bool reverse) {
    switch (col.iSubItem) {
        case COLUMN::ACTIVE:
            if (reverse) {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    bool aIsActive = g_lpe->mModel.getActivePreset() == a;
                    bool bIsActive = g_lpe->mModel.getActivePreset() == b;

                    return aIsActive < bIsActive;
                });
            } else {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    bool aIsActive = g_lpe->mModel.getActivePreset() == a;
                    bool bIsActive = g_lpe->mModel.getActivePreset() == b;

                    return aIsActive > bIsActive;
                });
            }
            break;
        case COLUMN::RECALLID:
            if (reverse) {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mRecallId < b->mRecallId;
                });
            } else {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mRecallId > b->mRecallId;
                });
            }
            break;
        case COLUMN::NAME:
            if (reverse) {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mName < b->mName;
                });
            } else {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mName > b->mName;
                });
            }
            break;
        case COLUMN::DESCRIPTION:
            if (reverse) {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mDescription < b->mDescription;
                });
            } else {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mDescription > b->mDescription;
                });
            }
            break;
        case COLUMN::TIME:
            if (reverse) {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mDate < b->mDate;
                });
            } else {
                setComparator([](LivePreset* a, LivePreset* b) -> bool {
                    return a->mDate > b->mDate;
                });
            }
        default: {
                setComparator(nullptr);
            }
    }
}

void LivePresetsListAdapter::saveColumnWidths(HWND hwnd) {
    auto str = WDL_FastString();
    for (int i = 0; i < getColumns().size(); i++) {
        str.AppendFormatted(4096, "%i ", ListView_GetColumnWidth(hwnd, i));
    }

    WritePrivateProfileString("LPE", "PresetsListColumns", str.Get(), get_ini_file());
}

void LivePresetsListAdapter::saveSortedColumnIndex(HWND, int index) {
    auto str = WDL_FastString();
    str.AppendFormatted(4096, "%i", index);

    WritePrivateProfileString("LPE", "PresetsListSortedColumnIndex", str.Get(), get_ini_file());
}