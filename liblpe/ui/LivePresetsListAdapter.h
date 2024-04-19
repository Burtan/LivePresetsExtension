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

#ifndef LPE_LIVEPRESETSLISTADAPTER_H
#define LPE_LIVEPRESETSLISTADAPTER_H

#include <memory>
#include <vector>
#ifdef _WIN32
    #include <Windows.h>
    #include <CommCtrl.h>
#else

#endif
#include <liblpe/ui/base/ListViewAdapter.h>
#include <liblpe/data/models/LivePreset.h>

class LivePresetsListAdapter final : public ListViewAdapter<LivePreset> {
public:
    explicit LivePresetsListAdapter(std::vector<LivePreset*>* items) : ListViewAdapter(items) {};

    void saveColumnWidths(HWND hwnd) override;
    void saveSortedColumnIndex(HWND hwnd, int index) override;
    void onChangedSortingColumn(LVCOLUMN col, bool reverse) override;
    std::vector<LVCOLUMN> getColumns() override;
    int getCount() override;
    LivePreset* getItem(int index) override;
    int getIndex(LivePreset* item) override;
    LVITEM getLvItem(int index) override;
    const char* getLvItemText(int index, int column) override;
};


#endif //LPE_LIVEPRESETSLISTADAPTER_H