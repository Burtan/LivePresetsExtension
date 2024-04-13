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

#ifndef LPE_PARAMINFO_H
#define LPE_PARAMINFO_H

#include <data/models/base/Filterable.h>
#include <cstdio>

template<typename T>
class Parameter : public Filterable {
public:
    explicit Parameter(Filterable* parent, std::string name, T value, FilterMode filter = RECALLED);
    explicit Parameter(Filterable* parent, int name, T value, FilterMode filter = RECALLED);

    std::string mKey;
    T mValue;

    FilterPreset* extractFilterPreset() override;
    bool applyFilterPreset(FilterPreset *preset) override;
    [[nodiscard]] char *getTreeText() const override;
};

template<typename T>
Parameter<T>::Parameter(Filterable* parent, std::string name, T value, FilterMode filter)
        : Filterable(parent, filter), mKey(std::move(name)), mValue(std::move(value))
{}

template<typename T>
Parameter<T>::Parameter(Filterable* parent, int name, T value, FilterMode filter)
        : Filterable(parent, filter), mKey(std::to_string(name)), mValue(std::move(value))
{}


#endif //LPE_PARAMINFO_H