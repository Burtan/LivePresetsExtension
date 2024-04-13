/******************************************************************************
/ LivePresetsExtension
/
/ C++ wrapper for winapi Combobox, use ComboBoxAdapter to customize
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

#include <ui/base/ComboBox.h>

ComboBox::ComboBox(HWND hwnd) : mHwnd(hwnd) {
}

void ComboBox::invalidate() {
    SendMessage(mHwnd, CB_RESETCONTENT, 0, 0);
    if (mAdapter != nullptr) {
        for (auto& item : mAdapter->mItems) {
            SendMessage(mHwnd, CB_ADDSTRING, 0, (LPARAM) item->data());
        }
    }
}

void ComboBox::setAdapter(std::unique_ptr<FilterPresetsComboAdapter> adapter) {
    mAdapter.swap(adapter);
    invalidate();
}

FilterPresetsComboAdapter* ComboBox::getAdapter() {
    return mAdapter.get();
}

int ComboBox::onNotify(WPARAM wParam, LPARAM lParam) {
    return 0;
}
