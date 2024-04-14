/******************************************************************************
/ LivePresetsExtension
/
/ Adapter for ListView
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

#ifndef LPE_LISTVIEWADAPTER_H
#define LPE_LISTVIEWADAPTER_H

#include <vector>
#ifdef _WIN32
    #include <Windows.h>
    #include <CommCtrl.h>
#else
    #define WDL_NO_DEFINE_MINMAX
    #include <third_party/WDL/WDL/swell/swell-types.h>
#endif

/**
 * An extension class for ListView that defines its items and behaviour
 * @tparam T The class type the ListView should hold.
 */
template<typename T>
class ListViewAdapter {
public:
    explicit ListViewAdapter(std::vector<T*>* items) : mItems(items) {};
    ~ListViewAdapter();

    virtual void saveColumnWidths(HWND hwnd) = 0;
    virtual void saveSortedColumnIndex(HWND hwnd, int index) = 0;
    virtual void onChangedSortingColumn(LVCOLUMN col, bool reverse) = 0;
    virtual std::vector<LVCOLUMN> getColumns() = 0;
    virtual int getCount() = 0;
    virtual T* getItem(int index) = 0;
    virtual int getIndex(T *item) = 0;
    virtual LVITEM getLvItem(int index) = 0;
    virtual const char* getLvItemText(int index, int column) = 0;
    void setComparator(bool (*compare)(T* a, T* b));
    void setFilter(bool (*filter)(T* a));
    void filterAndSort();
protected:
    std::vector<T*>* mItems;
    std::vector<T*> mShowingItems;
private:
    bool (*mCompare)(T* a, T* b) = nullptr;
    bool (*mFilter)(T* a) = nullptr;
};


#endif //LPE_LISTVIEWADAPTER_H