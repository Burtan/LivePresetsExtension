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

#include <memory>
#include <vector>
#ifdef _WIN32
    #include <CommCtrl.h>
#else
    #include <WDL/swell/swell-types.h>
#endif

/**
 * An extension class for ListView that defines its items and behaviour
 * @tparam T The class type the ListView should hold.
 */
template<typename T>
class ListViewAdapter {
public:
    explicit ListViewAdapter(std::vector<T*>* items) : mItems(items) {};

    virtual void saveColumnWidths(HWND hwnd) = 0;
    virtual void saveSortedColumnIndex(HWND hwnd, int index) = 0;
    virtual void onChangedSortingColumn(LVCOLUMN col, bool reverse) = 0;
    virtual std::vector<LVCOLUMN> getColumns() = 0;
    virtual int getCount() = 0;
    virtual T* getItem(int index) = 0;
    virtual int getIndex(T *item) = 0;
    virtual LVITEM getLvItem(int index) = 0;
    virtual const char* getLvItemText(int index, int column) = 0;
    /**
     * Add a comparator to enable sorting
     * @param compare a function pointer that defines the comparing algorythm. Should return true for higher order
     * of a compared to b, or false for lower order of a compared to b.
     */
    void setComparator(bool (*compare)(T* a, T* b)) {
        mCompare = compare;
    };
    /**
     * Add a filter to only show a subset of the items, e.g. for searching.
     * @param filter a function pointer that defines the filtering algorythm. Should return true to show the item.
     */
    void setFilter(bool (*filter)(T* a)) {
        mFilter = filter;
    };
    /*
     * Do not call this manually, is automatically called by the ListView when necessary.
     */
    void filterAndSort() {
        mShowingItems = std::vector(*mItems);

        //filter items
        if (mFilter) {
            mShowingItems.erase(remove_if(mShowingItems.begin(), mShowingItems.end(), mFilter), mShowingItems.end());
        }

        //sort items
        if (mCompare) {
            sort(mShowingItems.begin(), mShowingItems.end(), mCompare);
        }
    };
protected:
    std::vector<T*>* mItems;
    std::vector<T*> mShowingItems;
private:
    bool (*mCompare)(T* a, T* b) = nullptr;
    bool (*mFilter)(T* a) = nullptr;
};


#endif //LPE_LISTVIEWADAPTER_H