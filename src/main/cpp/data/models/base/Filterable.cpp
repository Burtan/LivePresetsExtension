/******************************************************************************
/ LivePresetsExtension
/
/ A class that defines functions to filter recallable data
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

#include <cstdarg>
#include <data/models/base/Filterable.h>

Filterable::Filterable(FilterMode mFilter) : mFilter(mFilter) {}

std::string Filterable::getFilterText() const {
    switch (mFilter) {
        case RECALLED:
            return "[R]";
        case IGNORED:
            return "[I]";
        case CHILD:
            return "[C]";
        default:
            return "";
    }
}

FilterMode Filterable::Merge(int num, ...) {
    va_list list;
    va_start(list, num);
    FilterMode outFilter = CHILD;

    for (int i = 0; i < num; i++) {
        auto filter = (FilterMode) va_arg(list, int);
        switch (filter) {
            case RECALLED:
                outFilter = RECALLED;
                goto filtered;
            case IGNORED:
                outFilter = IGNORED;
                goto filtered;
            case CHILD: {}
        }
    }
    filtered:

    va_end(list);
    return outFilter;
}

void Filterable::shuffleFilter() {
    switch (mFilter) {
        case RECALLED:
            mFilter = CHILD;
            break;
        case CHILD:
            mFilter = IGNORED;
            break;
        case IGNORED:
            mFilter = RECALLED;
            break;
    }
}


