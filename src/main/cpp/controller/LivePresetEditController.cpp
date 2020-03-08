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

#include <controller/LivePresetEditController.h>
#include <controller/ConfirmationController.h>
#include <LivePresetsExtension.h>
#include <resources/resource.h>

LivePresetEditController::LivePresetEditController(LivePreset* preset)
        : ModalWindow(IDD_LIVEPRESET, preset->mName.data(), "LivePresetsEditController", 0),
        mPreset(preset)
{}

void LivePresetEditController::onInitDlg() {
    mResizer.init_item(IDC_TREE, 0.0, 0.0, 1.0, 1.0);
    mResizer.init_item(IDC_SAVE, 0.0, 1.0, 0.0, 1.0);
    mResizer.init_item(IDC_CANCEL, 0.0, 1.0, 0.0, 1.0);
    mResizer.init_item(IDC_COMBO, 1.0, 0.0, 1.0, 0.0);
    mResizer.init_item(IDC_LABEL1, 1.0, 0.0, 1.0, 0.0);
    mResizer.init_item(IDC_SETTINGS, 1.0, 0.0, 1.0, 0.0);

    //fills TextEdits
    SetDlgItemText(mHwnd, IDC_NAME, mPreset->mName.data());
    SetDlgItemText(mHwnd, IDC_DESC, mPreset->mDescription.data());
    SetDlgItemText(mHwnd, IDC_ID, mPreset->mRecallId.data());

    //create TreeView and add event listeners
    mTree = std::make_unique<TreeView>(GetDlgItem(mHwnd, IDC_TREE));
    auto treeAdapter = std::make_unique<LivePresetsTreeAdapter>(mPreset);
    mTree->setAdapter(std::move(treeAdapter));

    //hide combo in ce version
    if (!Licensing_IsUltimate()) {
        ShowWindow(GetDlgItem(mHwnd, IDC_LABEL1), SW_HIDE);
        ShowWindow(GetDlgItem(mHwnd, IDC_COMBO), SW_HIDE);
        ShowWindow(GetDlgItem(mHwnd, IDC_SETTINGS), SW_HIDE);
    }

    //create combobox
    mCombo = std::make_unique<ComboBox>(GetDlgItem(mHwnd, IDC_COMBO));
    auto comboAdapter = std::make_unique<FilterPresetsComboAdapter>(FilterPreset_GetNames(g_lpe->mModel.mFilterPresets));
    mCombo->setAdapter(std::move(comboAdapter));
}

void LivePresetEditController::cancel() {
    close();
}

void LivePresetEditController::save() {
    char buf[512];
    GetDlgItemText(mHwnd, IDC_NAME, buf, sizeof(buf));
    mPreset->mName = buf;

    GetDlgItemText(mHwnd, IDC_DESC, buf, sizeof(buf));
    mPreset->mDescription = buf;

    GetDlgItemText(mHwnd, IDC_ID, buf, sizeof(buf));
    //surround with try/catch because stoi can fail when there is no valid id
    try {
        int id = std::stoi(buf);
        g_lpe->mModel.setRecallIdForPreset(mPreset, id);
    } catch (std::exception&) {}

    keepPreset = true;
    close();
}

LPARAM LivePresetEditController::result() {
    return (LPARAM) mPreset;
}

void LivePresetEditController::onCommand(WPARAM wparam, LPARAM lparam) {
    if (HIWORD(wparam) == CBN_SELCHANGE) {
        int index = SendMessage((HWND) lparam, CB_GETCURSEL, 0, 0);
        char name[256];
        SendMessage((HWND) lparam, CB_GETLBTEXT, index, (LPARAM) name);
        auto tempName = std::string(name);
        if (FilterPreset* preset = FilterPreset_GetFilterByName(g_lpe->mModel.mFilterPresets, &tempName)) {
            mPreset->applyFilterPreset(preset);
            mTree->invalidate();
        }
        return;
    }
    switch (wparam) {
        case IDC_CANCEL:
            cancel();
            break;
        case IDC_SAVE:
            save();
            break;
        case IDC_SETTINGS:
            showFilterSettings();
            break;
        case IDC_ADD: {
            auto filter = mPreset->extractFilterPreset();
            std::string name = "New preset";
            auto dlg = ConfirmationController("Save filter...", &name);
            if (bool result = dlg.show()) {
                //When name is entered that exists ask for overwrite
                if (FilterPreset_GetFilterByName(g_lpe->mModel.mFilterPresets, &name)) {
                    char title[256];
                    sprintf(title, "Overwrite %s?", name.data());
                    char msg[256];
                    sprintf(msg, "The filter preset %s already exists.\n Do you want to overwrite it?", name.data());
                    if (MessageBox(mHwnd, msg, title, MB_YESNO) != IDYES) {
                        return;
                    }
                }
                FilterPreset_SetName(filter, name);
                FilterPreset_AddPreset(g_lpe->mModel.mFilterPresets, filter);
                mCombo->getAdapter()->mItems = FilterPreset_GetNames(g_lpe->mModel.mFilterPresets);
                mTree->invalidate();
                mCombo->invalidate();
            }
            break;
        }
        default: {}
    }
}

int LivePresetEditController::onKey(MSG* msg, int keyState) {
    HWND focus = GetFocus();
    if (focus == GetDlgItem(mHwnd, IDC_TREE)) {
        int handled = mTree->onKey(msg, keyState);

        if (handled)
            return handled;
    }

    if (msg->message == WM_KEYDOWN) {
        if (!keyState) {
            switch(msg->wParam) {
                case VK_RETURN:
                    save();
                    return 1;
                case VK_ESCAPE:
                    cancel();
                    return 1;
            }
        }
    }
    return 0;
}

void LivePresetEditController::onClose() {
    ModalWindow::onClose();
    if (!keepPreset) {
        delete mPreset;
        mPreset = nullptr;
    }
}

int LivePresetEditController::onNotify(WPARAM wParam, LPARAM lParam) {
    HWND controlHwnd = ((NMHDR*) lParam)->hwndFrom;
    if (mTree) {
        if (mTree->mHwnd == controlHwnd) {
            mTree->onNotify(wParam, lParam);
        }
    }
    if (mCombo) {
        if (mCombo->mHwnd == controlHwnd) {
            mCombo->onNotify(wParam, lParam);
        }
    }
    return 0;
}

void LivePresetEditController::showFilterSettings() {
    HMENU menu = CreatePopupMenu();

    MENUITEMINFO mii{}; MENUITEMINFO();

    //show closing option
    mii = MENUITEMINFO();
    mii.fMask |= MIIM_TYPE | MIIM_ID;
    mii.fType |= MFT_STRING;
    mii.cbSize = sizeof(MENUITEMINFO);

    std::string text = "Save filter...";
    mii.dwTypeData = text.data();
    mii.cch = text.size();

    //wID specifies the commandId returned to sWndProc as wParam and WM_COMMAND onClick
    mii.wID = IDC_ADD;
    InsertMenuItem(menu, 1, true, &mii);

    RECT r{};
    GetWindowRect(GetDlgItem(mHwnd, IDC_SETTINGS), &r);
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN, r.left, r.bottom, 0, mHwnd, nullptr);
}


