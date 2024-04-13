/******************************************************************************
/ LivePresetsExtension
/
/ A class that holds a parameter that can be filtered and persisted
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

#include <data/models/base/Parameter.h>
#include <data/models/FilterPreset.h>

template<typename T>
FilterPreset* Parameter<T>::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};
    id.key = mKey;
    return new FilterPreset(id, PARAM, mFilter);
}
template FilterPreset* Parameter<double>::extractFilterPreset();
template FilterPreset* Parameter<int>::extractFilterPreset();
template FilterPreset* Parameter<bool>::extractFilterPreset();
template FilterPreset* Parameter<std::string>::extractFilterPreset();

template<typename T>
bool Parameter<T>::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == PARAM && preset->mId.key == mKey) {
        mFilter = preset->mFilter;
        return true;
    }
    return false;
}
template bool Parameter<double>::applyFilterPreset(FilterPreset *preset);
template bool Parameter<int>::applyFilterPreset(FilterPreset *preset);
template bool Parameter<bool>::applyFilterPreset(FilterPreset *preset);
template bool Parameter<std::string>::applyFilterPreset(FilterPreset *preset);

template<>
char * Parameter<double>::getTreeText() const {
    std::string newText = getFilterText() + " " + mKey;
    if (!isFilteredInChain()) {
        newText = newText + " = " + std::to_string(mValue);
    }
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}
template<>
char * Parameter<int>::getTreeText() const {
    std::string newText = getFilterText() + " " + mKey;
    if (!isFilteredInChain()) {
        newText = newText + " = " + std::to_string(mValue);
    }
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}
template<>
char * Parameter<bool>::getTreeText() const {
    std::string newText = getFilterText() + " " + mKey;
    if (!isFilteredInChain()) {
        newText = newText + " = " + std::to_string(mValue);
    }
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}
template<>
char * Parameter<std::string>::getTreeText() const {
    std::string newText = getFilterText() + " " + mKey;
    if (!isFilteredInChain()) {
        newText = newText + " = " + mValue;
    }
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}