/******************************************************************************
/ LivePresetsExtension
/
/ Modal window that shows some extension settings
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

#include <controller/SettingsController.h>
#include <ui/FilterPresetsComboAdapter.h>
#include <resources/resource.h>
#include <plugins/reaper_plugin_functions.h>
#include <LivePresetsExtension.h>

/*
Constructor for the LivePresetsController. Takes the layout file from resource.rc IDD_LIVEPRESETS
*/
SettingsController::SettingsController()
        : ModalWindow(IDD_SETTINGS, "LivePresets Settings", "SettingsController", 0)
{}

void SettingsController::onInitDlg() {
    mResizer.init_item(IDC_SAVE, 0.0, 1.0, 0.0, 1.0);

    //init checkboxes
    CheckDlgButton(mHwnd, IDC_UNDO, g_lpe->mModel.mDoUndo);
    CheckDlgButton(mHwnd, IDC_RECALL_ACTIVE_PLUGINPRESETS, g_lpe->mModel.mIsReselectFxPreset);
    CheckDlgButton(mHwnd, IDC_RECALL_MUTED_PLUGINS, g_lpe->mModel.mIsLoadStateOnMute);
    CheckDlgButton(mHwnd, IDC_RECALL_ACTIVE_PRESETS, g_lpe->mModel.mIsReselectLivePresetByValueRecall);
    CheckDlgButton(mHwnd, IDC_HIDE_MUTED_TRACKS, g_lpe->mModel.mIsHideMutedTracks);

    //create combo add FilterPreset names and select default
    mCombo = std::make_unique<ComboBox>(GetDlgItem(mHwnd, IDC_COMBO));
    std::vector<std::string*> filterNames = FilterPreset_GetNames(g_lpe->mModel.mFilterPresets);
    auto comboAdapter = std::make_unique<FilterPresetsComboAdapter>(filterNames);
    mCombo->setAdapter(std::move(comboAdapter));
    int index = 0;
    for (auto *filter : filterNames) {
        if (*filter == g_lpe->mModel.mDefaultFilterPreset) {
            SendMessage(mCombo->mHwnd, CB_SETCURSEL, 0, index);
        }
        index++;
    }

}

void SettingsController::onCommand(WPARAM wParam, LPARAM lparam) {
    //change default FilterPreset
    if (HIWORD(wParam) == CBN_SELCHANGE) {
        int index = SendMessage((HWND) lparam, CB_GETCURSEL, 0, 0);
        char name[256];
        SendMessage((HWND) lparam, CB_GETLBTEXT, index, (LPARAM) name);
        auto tempName = std::string(name);
        if (FilterPreset_GetFilterByName(g_lpe->mModel.mFilterPresets, &tempName)) {
            g_lpe->mModel.mDefaultFilterPreset = tempName;
        }
        return;
    }
    switch (wParam) {
        case IDC_RECALL: {
            auto cmdId = NamedCommandLookup("_LPE_SELECTPRESET");
            auto *section = SectionFromUniqueID(0);
            DoActionShortcutDialog(mHwnd, section, cmdId, 0);
            break;
        }
        case IDC_TOGGLETRACKVISIBILITY: {
            auto cmdId = NamedCommandLookup("_LPE_TOGGLEMUTEDVISIBILITY");
            auto *section = SectionFromUniqueID(0);
            DoActionShortcutDialog(mHwnd, section, cmdId, 0);
            break;
        }
        case IDC_REMOVE: {
            int index = SendMessage(mCombo->mHwnd, CB_GETCURSEL, 0, 0);
            if (index == CB_ERR)
                break;
            g_lpe->mModel.mFilterPresets.erase(g_lpe->mModel.mFilterPresets.begin() + index);
            SendMessage(mCombo->mHwnd, CB_SETCURSEL, -1, 0);
            mCombo->getAdapter()->mItems = FilterPreset_GetNames(g_lpe->mModel.mFilterPresets);
            mCombo->invalidate();
            break;
        }
        case IDC_SAVE:
            close();
            break;
        case IDC_UNDO:
            g_lpe->mModel.mDoUndo = IsDlgButtonChecked(mHwnd, IDC_UNDO);
            break;
        case IDC_RECALL_ACTIVE_PLUGINPRESETS:
            g_lpe->mModel.mIsReselectFxPreset = IsDlgButtonChecked(mHwnd, IDC_RECALL_ACTIVE_PLUGINPRESETS);
            break;
        case IDC_RECALL_MUTED_PLUGINS:
            g_lpe->mModel.mIsLoadStateOnMute = IsDlgButtonChecked(mHwnd, IDC_RECALL_MUTED_PLUGINS);
            break;
        case IDC_RECALL_ACTIVE_PRESETS:
            g_lpe->mModel.mIsReselectLivePresetByValueRecall = IsDlgButtonChecked(mHwnd, IDC_RECALL_ACTIVE_PRESETS);
            break;
        case IDC_HIDE_MUTED_TRACKS:
            g_lpe->mModel.mIsHideMutedTracks = IsDlgButtonChecked(mHwnd, IDC_HIDE_MUTED_TRACKS);
            break;
        case IDC_UPDATE: {
            SetTimer(mHwnd, 1, 0, static_cast<TIMERPROC>(updateAllPresets));
            break;
        }
        default: {}
    }
}

void CALLBACK SettingsController::updateAllPresets(HWND hwnd, UINT, UINT_PTR i, DWORD) {
    if (i - 1 < g_lpe->mModel.mPresets.size()) {
        LivePreset* preset = g_lpe->mModel.mPresets.at(i - 1);
        fprintf(stderr, "Updating: %s \n", preset->mName.data());
        g_lpe->mModel.recallPreset(preset);
        preset->saveCurrentState(true);

        SetTimer(hwnd, i + 1, 100, static_cast<TIMERPROC>(updateAllPresets));
    }
    KillTimer(hwnd, i);
}

LPARAM SettingsController::result() {
    return (LPARAM) 0;
}

/**
 * Called by WM_NOTIFY messages that are received from the root window. They are passed to the child windows.
 * @param wParam The identifier of the common control sending the message. This identifier is not guaranteed to be unique. An application should use the hwndFrom or idFrom member of the NMHDR structure (passed as the lParam parameter) to identify the control.
 * @param lParam A pointer to an NMHDR structure that contains the notification code and additional information. For some notification messages, this parameter points to a larger structure that has the NMHDR structure as its first member.
 * @return The return value is ignored except for notification messages that specify otherwise.
 */
int SettingsController::onNotify(WPARAM, LPARAM) {
    return 0;
}

int SettingsController::onKey(MSG*, int) {
    return 0;
}

void SettingsController::onClose() {
}