/******************************************************************************
/ LivePresetsExtension
/
/ Controller for the Control view
/
/ Copyright (c) 2020 and later Dr. med. Frederik Bertling
******************************************************************************/

#include <liblpe/controller/ControlViewController.h>
#include <liblpe/resources/resource.h>
#include <liblpe/ui/ControlsListAdapter.h>
#include <liblpe/LivePresetsExtension.h>
#include <liblpe/controller/ConfirmationController.h>
#include <liblpe/util/util.h>

/*
Constructor for the ControlViewController. Takes the layout file from resource.rc IDD_CONTROLVIEW
*/
ControlViewController::ControlViewController()
        : DockWindow(IDD_CONTROLVIEW, "Control View", "ControlViewController", 0) {}

void ControlViewController::onInitDlg() {
    mResizer.init_item(IDC_TAB, 0.0, 0.0, 1.0, 1.0);
    mResizer.init_item(IDC_ADD2, 1.0, 0.0, 1.0, 0.0);
    mResizer.init_item(IDC_REMOVE2, 1.0, 0.0, 1.0, 0.0);

    tabHwnd = GetDlgItem(mHwnd, IDC_TAB);
    reset();
}

void ControlViewController::onCommand(WPARAM wParam, LPARAM) {
    switch (wParam) {
        case IDC_ADD:
            addHardware();
            break;
        case IDC_REMOVE:
            removeCurrentHardware();
            break;
        case IDC_ADD2:
            addControl();
            break;
        case IDC_REMOVE2:
            removeSelectedControls();
            break;
        case IDC_ASSIGNINFO:
            assignParamToControl();
            break;
        default: {}
    }
}

void ControlViewController::addHardware() {
    if (!tabHwnd)
        return;

    std::string name = "New hardware";
    auto dlg = ConfirmationController("Create hardware...", &name);

    if (dlg.show()) {
        auto* hw = new Hardware();
        hw->mName = name;
        g_lpe->mModel->mHardwares.push_back(hw);

        TCITEM tcitem{};
        tcitem.mask = TCIF_TEXT | TCIF_IMAGE;
        tcitem.iImage = -1;

        tcitem.pszText = hw->mName.data();
        tcitem.cchTextMax = hw->mName.length();

        TabCtrl_InsertItem(tabHwnd, TabCtrl_GetItemCount(tabHwnd), &tcitem);

        //show new hardware
        showTab(TabCtrl_GetItemCount(tabHwnd) - 1);
    }

}

void ControlViewController::removeCurrentHardware() {
    int index = TabCtrl_GetCurSel(tabHwnd);

    if (index > -1) {
        g_lpe->mModel->mHardwares.erase(g_lpe->mModel->mHardwares.begin() + index);
        TabCtrl_DeleteItem(tabHwnd, index);

        if (index > 0) {
            showTab(index - 1);
        } else if (TabCtrl_GetItemCount(tabHwnd) > 0) {
            showTab(0);
        }
    }
}

void ControlViewController::addControl() {
    if (TabCtrl_GetItemCount(tabHwnd) > 0) {
        auto *hw = g_lpe->mModel->mHardwares.at(TabCtrl_GetCurSel(tabHwnd));
        hw->mControls.push_back(new Control());
        mList->invalidate();
    }
}

void ControlViewController::removeSelectedControls() {
    auto *hw = g_lpe->mModel->mHardwares.at(TabCtrl_GetCurSel(tabHwnd));
    auto indices = mList->getSelectedIndices();
    //indices are always sorted from low to high
    for (int i = indices.size() - 1; i >= 0; i--) {
        hw->mControls.erase(hw->mControls.begin() + indices[i]);
        mList->invalidate();
    }
}

int ControlViewController::onNotify(WPARAM wParam, LPARAM lParam) {
    if (mList) {
        if (mList->mHwnd == ((NMHDR*) lParam)->hwndFrom) {
            return mList->onNotify(wParam, lParam);
        }
    }

    switch (((LPNMHDR) lParam)->code) {
        case TCN_SELCHANGE: {
            int index = TabCtrl_GetCurSel(tabHwnd);
            showTab(index);
            return true;
        }
    }

    return 0;
}

void ControlViewController::onContextMenu(HMENU menu) {
}

int ControlViewController::onKey(MSG *msg, int iKeyState) {
    return DockWindow::onKey(msg, iKeyState);
}

void ControlViewController::onResize() {
    mTabResizer.onResize();
}

void ControlViewController::onClose() {
    if (mList)
        mList->onDestroy();
}

void ControlViewController::createList() {
    Hardware* hw = g_lpe->mModel->mHardwares.at(TabCtrl_GetCurSel(tabHwnd));
    mTabResizer.init(tabHwnd);

    RECT r = RECT();
    GetWindowRect(tabHwnd, &r);

    AddControl(
            tabHwnd, "ControlList", IDC_LIST, "SysListView32",
            WS_CHILD | WS_VISIBLE | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS, 0,
            0, 12, 86, (r.bottom - r.top - 32) / 2
    );

    mTabResizer.init_item(IDC_LIST, 0.0, 0.0, 0.0, 1.0);

    mList = std::make_unique<ListView<Control>>(GetDlgItem(tabHwnd, IDC_LIST));
    mList->addListViewEventListener([this, hw](NMLISTVIEW* event) -> void {
        switch (event->hdr.code) {
            case LVN_ITEMCHANGED: {
                auto indices = mList->getSelectedIndices();
                if (indices.size() != 1)
                    return;

                showControl(hw, indices.front());
                break;
            }
        }
    });
}

void ControlViewController::showTab(int index) {
    //make sure the tab to show is selected
    TabCtrl_SetCurSel(tabHwnd, index);

    //remove all dynamic controls
    //listview, remove Id for scaling
    SetWindowLongPtr(GetDlgItem(tabHwnd, IDC_LIST), GWL_ID, 0);
    RemoveControl(GetDlgItem(tabHwnd, IDC_LIST));

    //fader
    for (int id : mControlIds) {
        RemoveControl(GetDlgItem(mHwnd, id));
    }
    mControlIds.clear();

    //add dynamic controls
    if (index >= 0) {
        createList();

        auto *hw = g_lpe->mModel->mHardwares.at(index);
        auto adatper = std::make_unique<ControlsListAdapter>(&hw->mControls);
        mList->setAdapter(std::move(adatper));

        if (!hw->mControls.empty()) {
            showControl(hw, 0);
        }
    } else {

    }

}

void ControlViewController::showControl(Hardware* hw, int index) {
    Control* control = hw->mControls.at(index);

    //make sure the control to show is selected
    if (mList->getSelectedIndices().empty() || mList->getSelectedIndices().front() != index) {
        mList->selectIndex(index);
    }

    //label for currently assigned controlinfo
    const char* lblText = "Nothing assigned";
    auto info = hw->getControlInfoForControl(control);
    if (info) {
        lblText = info->getTreeText();
    }

    AddControl(
            tabHwnd, lblText, IDC_LABEL1, "static",
            0, 0,
            94, 16, 128, 8
    );

    AddControl(
            tabHwnd, "Reassign", IDC_ASSIGNINFO, "BUTTON",
            0, 0,
            94, 28, 72, 16
    );


    /*
     *
   int id = ID_CONTROLS + mControlIds.size();
   mControlIds.emplace(id);
   AddControl(
           tabHwnd, "", id, "REAPERhfader",
           TBS_NOTICKS, 0,
           100, 100, 700, 20
   );

    */
}

void ControlViewController::assignParamToControl() {
    auto *hw = g_lpe->mModel->mHardwares.at(TabCtrl_GetCurSel(tabHwnd));
    if (mList->getSelectedIndices().size() != 1)
        return;

    auto index = mList->getSelectedIndices().front();
    Control* control = hw->mControls.at(index);

    int track = 0;
    int fx = 0;
    int param = 0;

    if (!GetLastTouchedFX(&track, &fx, &param))
        return;

    GUID mTrackGuid = *GetTrackGUID(GetTrack(nullptr, track));
    GUID mFxGuid = *TrackFX_GetFXGUID(GetTrack(nullptr, track), fx);
    int mParamIndex = param;

    auto info = std::make_shared<ControlInfo>(ControlInfo(nullptr, hw->mGuid, control->mGuid, mTrackGuid, mFxGuid, mParamIndex));
    hw->addControlInfo(info);

    //invalidate
    showControl(hw, index);
}

void ControlViewController::reset() {
    //reset data
    for (int id : mControlIds) {
        RemoveControl(GetDlgItem(mHwnd, id));
    }
    mControlIds.clear();

    if (tabHwnd) {
        while (TabCtrl_GetItemCount(tabHwnd) > 0) {
            TabCtrl_DeleteItem(tabHwnd, 0);
        }
    }

    //init data
    if (g_lpe->mModel) {
        for (auto *hw : g_lpe->mModel->mHardwares) {
            //add tab
            TCITEM tcitem{};
            tcitem.mask = TCIF_TEXT | TCIF_IMAGE;
            tcitem.iImage = -1;

            tcitem.pszText = hw->mName.data();
            tcitem.cchTextMax = hw->mName.length();

            TabCtrl_InsertItem(tabHwnd, TabCtrl_GetItemCount(tabHwnd), &tcitem);
        }
    }

    if (TabCtrl_GetItemCount(tabHwnd) > 0) {
        showTab(0);
    } else {
        showTab(-1);
    }

}