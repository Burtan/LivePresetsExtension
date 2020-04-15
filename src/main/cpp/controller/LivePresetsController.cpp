/******************************************************************************
/ LivePresetsExtension
/
/ Main extension window that shows a list of presets and buttons to other menus
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

#include <utility>
#include <controller/LivePresetsController.h>
#include <controller/LivePresetEditController.h>
#include <controller/SettingsController.h>
#include <resources/resource.h>
#include <plugins/reaper_plugin_functions.h>
#include <ui/LivePresetsListAdapter.h>
#include <LivePresetsExtension.h>
#include <data/models/StringProjectStateContext.h>

/*
Constructor for the LivePresetsController. Takes the layout file from resource.rc IDD_LIVEPRESETS
*/
LivePresetsController::LivePresetsController()
        : DockWindow(IDD_LIVEPRESETS, "LivePresets", "LivePresetsController", 0) {

}

void LivePresetsController::onInitDlg() {
    mResizer.init_item(IDC_LIST, 0.0, 0.0, 1.0, 1.0);
    mResizer.init_item(IDC_ADD, 0.0, 1.0, 0.0, 1.0);
    mResizer.init_item(IDC_UPDATE, 0.0, 1.0, 0.0, 1.0);
    mResizer.init_item(IDC_REMOVE, 0.0, 1.0, 0.0, 1.0);
    mResizer.init_item(IDC_EDIT, 0.0, 1.0, 0.0, 1.0);
    mResizer.init_item(IDC_SETTINGS, 1.0, 1.0, 1.0, 1.0);

    //create ListView and add event listeners
    mList = std::make_unique<ListView<LivePreset>>(GetDlgItem(mHwnd, IDC_LIST));
    auto adapter = std::make_unique<LivePresetsListAdapter>(&g_lpe->mModel.mPresets);
    mList->setAdapter(std::move(adapter));
    mList->addListViewEventListener([this](NMLISTVIEW* event) -> void {
        switch (event->hdr.code)         {
            case NM_DBLCLK: {
                auto *const preset = mList->getAdapter()->getItem(event->iItem);
                recallLivePreset(preset);
                break;
            }
        }
    });
}

void LivePresetsController::recallLivePreset(LivePreset* preset) {
    g_lpe->mModel.recallPreset(preset);
}

/**
 * Opens the edit dialog and creates the preset when the result is not cancelled
 */
void LivePresetsController::createPreset() const {
    auto *preset = g_lpe->mModel.getCurrentSettingsAsPreset();
    if (auto *filter = FilterPreset_GetFilterByName(g_lpe->mModel.mFilterPresets, &g_lpe->mModel.mDefaultFilterPreset)) {
        preset->applyFilterPreset(filter);
    }

    //keep track of the recallCmdId to remove it if the created preset is removed
    auto tempCmdId = preset->mRecallCmdId;
    auto *editedPreset = editPreset(preset);

    if (editedPreset != nullptr) {
        g_lpe->mModel.addPreset(editedPreset, false);
        if (!mList)
            return;
        mList->invalidate();
        int index = mList->getAdapter()->getIndex(editedPreset);
        mList->selectIndex(index);
    } else {
        g_lpe->mActions.remove(tempCmdId);
    }
}

/**
 * Opens a new modal window which can modify the preset and return it afterwards if not cancelled, then its null
 */
LivePreset* LivePresetsController::editPreset(LivePreset* preset) {
    auto dlg = LivePresetEditController(preset);
    auto *result = (LivePreset*) dlg.show();
    return result;
}

void LivePresetsController::updateSelectedPreset() const {
    if (!mList)
        return;

    auto indices = mList->getSelectedIndices();
    if (indices.size() != 1)
        return;

    auto *preset = mList->getAdapter()->getItem(indices.front());
    preset->saveCurrentState(true);
    Undo_OnStateChangeEx2(nullptr, "Update LivePreset", UNDO_STATE_MISCCFG, -1);
}

void LivePresetsController::removeSelectedPresets() const {
    if (!mList)
        return;

    std::vector<LivePreset*> presets;
    for (auto index : mList->getSelectedIndices()) {
        presets.push_back(mList->getAdapter()->getItem(index));
    }
    g_lpe->mModel.removePresets(presets);
    mList->invalidate();
}

/**
 * Creates a copy of the currently selected preset and opens the edit dialog for it. When the dialog is confirmed
 * The copy is saved in place of the original one
 */
void LivePresetsController::editSelectedPreset() const {
    if (!mList)
        return;

    auto indices = mList->getSelectedIndices();
    if (indices.size() != 1)
        return;

    auto *preset = mList->getAdapter()->getItem(indices.front());

    WDL_FastString str;
    preset->persist(str);
    auto ctx = StringProjectStateContext(str);
    auto *presetToEdit = new LivePreset((ProjectStateContext*) &ctx, preset->mRecallCmdId);

    auto *editedPreset = editPreset(presetToEdit);

    if (editedPreset != nullptr) {
        g_lpe->mModel.replacePreset(preset, editedPreset);
        //Undo_OnStateChangeEx2(nullptr, "Updated LivePreset", UNDO_STATE_MISCCFG, -1);
        mList->invalidate();
        int index = mList->getAdapter()->getIndex(editedPreset);
        mList->selectIndex(index);
    }
}

void LivePresetsController::showSettings() {
    auto dlg = SettingsController();
    dlg.show();
}

void LivePresetsController::onContextMenu(HMENU menu) {
    auto indices = mList->getSelectedIndices();
    auto filters = FilterPreset_GetNames(g_lpe->mModel.mFilterPresets);

    if (!indices.empty()) {

        //show list of filter presets
        HMENU subMenu = CreatePopupMenu();

        MENUITEMINFO mii{};
        mii.fMask |= MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
        mii.fType |= MFT_STRING;
        mii.cbSize = sizeof(MENUITEMINFO);

        std::string text = "Apply filter to selected presets";
        mii.dwTypeData = text.data();
        mii.cch = text.size();

        mii.hSubMenu = subMenu;

        InsertMenuItem(menu, 0, true, &mii);

        int index = 0;
        for (auto *filter : filters) {
            mii = MENUITEMINFO();

            mii.fMask |= MIIM_TYPE | MIIM_ID | MIIM_DATA;
            mii.fType |= MFT_STRING;
            mii.cbSize = sizeof(MENUITEMINFO);

            mii.dwTypeData = filter->data();
            mii.cch = filter->size();

            //wID specifies the commandId returned to sWndProc as wParam and WM_COMMAND onClick
            mii.wID = ID_APPLY_FILTER + index;

            InsertMenuItem(subMenu, index, true, &mii);
            index++;
        }
    }
}

void LivePresetsController::applyFilterToSelectedTracks(int filterIndex) const {
    for (auto index : mList->getSelectedIndices()) {
        mList->getAdapter()->getItem(index)->applyFilterPreset(g_lpe->mModel.mFilterPresets[filterIndex]);
    }
}

void LivePresetsController::onCommand(WPARAM wParam, LPARAM) {
    switch (wParam) {
        case IDC_ADD:
            createPreset();
            break;
        case IDC_UPDATE:
            updateSelectedPreset();
            break;
        case IDC_REMOVE:
            removeSelectedPresets();
            break;
        case IDC_EDIT:
            editSelectedPreset();
            break;
        case IDC_SETTINGS:
            showSettings();
            break;
        default: {
            if (wParam >= ID_APPLY_FILTER) {
                applyFilterToSelectedTracks((int) wParam - ID_APPLY_FILTER);
            }
        }
    }
}

/**
 * Called by WM_NOTIFY messages that are received from the root window. They are passed to the child windows.
 * @param wParam The identifier of the common control sending the message. This identifier is not guaranteed to be unique. An application should use the hwndFrom or idFrom member of the NMHDR structure (passed as the lParam parameter) to identify the control.
 * @param lParam A pointer to an NMHDR structure that contains the notification code and additional information. For some notification messages, this parameter points to a larger structure that has the NMHDR structure as its first member.
 * @return The return value is ignored except for notification messages that specify otherwise.
 */
int LivePresetsController::onNotify(WPARAM wParam, LPARAM lParam) {
    HWND controlHwnd = ((NMHDR*) lParam)->hwndFrom;
    if (mList) {
        if (mList->mHwnd == controlHwnd) {
            return mList->onNotify(wParam, lParam);
        }
    }
    return 0;
}

int LivePresetsController::onKey(MSG* msg, int keyState) {
    if (msg->message == WM_KEYDOWN) {
        if (!keyState) {
            switch(msg->wParam) {
                case VK_F2:
                    editSelectedPreset();
                    return 1;
                case VK_DELETE:
                    removeSelectedPresets();
                    return 1;
                case VK_INSERT:
                    createPreset();
                    return 1;
            }
        }
    }
    return 0;
}

void LivePresetsController::onClose() {
    if (mList)
        mList->onDestroy();
}

void LivePresetsController::reset() const {
    if (mList) {
        auto adapter = std::make_unique<LivePresetsListAdapter>(&g_lpe->mModel.mPresets);
        mList->setAdapter(std::move(adapter));
        mList->invalidate();
    }
}

void LivePresetsController::getMinMaxInfo(LPMINMAXINFO info) {
    info->ptMinTrackSize.x = 1284;
    info->ptMinTrackSize.y = 625;
}