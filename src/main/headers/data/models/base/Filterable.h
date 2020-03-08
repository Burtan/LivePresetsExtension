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

#ifndef LPE_FILTERABLE_H
#define LPE_FILTERABLE_H


#include <string>
#include <climits>
#include <memory>
class FilterPreset;

enum TYPE {
    NOTHING = INT_MAX,
    MASTERTRACK = 1,
    TRACK = 2,
    PARAMS = 3,
    SEND = 4,
    FX = 5,
    PARAM = 6,
    LIVEPRESET = 7,
    CTRL = 8
};

enum FilterMode {
    RECALLED,
    IGNORED,
    CHILD
};

class Filterable {
public:
    static FilterMode Merge(int num...);

    explicit Filterable(FilterMode mFilter = RECALLED);

    FilterMode mFilter = RECALLED;
    void shuffleFilter();
    virtual FilterPreset * extractFilterPreset() = 0;
    virtual bool applyFilterPreset(FilterPreset *preset) = 0;
    [[nodiscard]] virtual char* getTreeText() const = 0;
protected:
    mutable char mTreeText[256];
    [[nodiscard]] std::string getFilterText() const;
};


#endif //LPE_FILTERABLE_H