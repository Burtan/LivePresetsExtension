/******************************************************************************
/ LivePresetsExtension
/
/ Window to edit a single LivePreset
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

#ifndef LPE_LIVEPRESETEDITCONTROLLER_H
#define LPE_LIVEPRESETEDITCONTROLLER_H


#include <functional>
#include <liblpe/ui/base/ModalWindow.h>
#include <liblpe/ui/base/TreeView.h>
#include <liblpe/ui/base/ComboBox.h>
#include <liblpe/data/models/LivePreset.h>

class LivePresetEditController : public ModalWindow {
public:
    explicit LivePresetEditController(LivePreset* preset);

    std::unique_ptr<ComboBox> mCombo = nullptr;
    std::unique_ptr<TreeView> mTree = nullptr;
protected:
    void onInitDlg() override;
    void onCommand(WPARAM wparam, LPARAM lParam) override;
    int onKey(MSG* msg, int iKeyState) override;
    void onClose() override;
    LPARAM result() override;
    void getMinMaxInfo(LPMINMAXINFO info) override;
    int onNotify(WPARAM wParam, LPARAM lParam) override;
private:
    static WNDPROC defWndProc;
    static LRESULT WINAPI wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    bool keepPreset = false;
    LivePreset* mPreset;
    void cancel();
    void save();
    void showFilterSettings();
};


#endif //LPE_LIVEPRESETEDITCONTROLLER_H