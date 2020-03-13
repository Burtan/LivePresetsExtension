/******************************************************************************
/ LivePresetsExtension
/
/ 
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

#include <ui/base/ListViewAdapter.h>
#include <data/models/LivePreset.h>

/**
 * Add a comparator to enable sorting
 * @param compare a function pointer that defines the comparing algorythm. Should return true for higher order
 * of a compared to b, or false for lower order of a compared to b.
 */
template<typename T>
void ListViewAdapter<T>::setComparator(bool (*compare)(T* a, T* b)) {
    mCompare = compare;
};
template void ListViewAdapter<LivePreset>::setComparator(bool (*compare)(LivePreset* a, LivePreset* b));

template<typename T>
ListViewAdapter<T>::~ListViewAdapter() {
}
template ListViewAdapter<LivePreset>::~ListViewAdapter();

/**
 * Add a filter to only show a subset of the items, e.g. for searching.
 * @param filter a function pointer that defines the filtering algorythm. Should return true to show the item.
 */
template<typename T>
void ListViewAdapter<T>::setFilter(bool (*filter)(T* a)) {
    mFilter = filter;
};
template void ListViewAdapter<LivePreset>::setFilter(bool (*filter)(LivePreset* a));

/*
 * Do not call this manually, is automatically called by the ListView when necessary.
 */
template<typename T>
void ListViewAdapter<T>::filterAndSort() {
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
template void ListViewAdapter<LivePreset>::filterAndSort();