/******************************************************************************
/ LivePresetsExtension
/
/ Base class for dockable windows
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

#include <liblpe/ui/base/DockWindow.h>
#ifdef _WIN32
    #include <windowsx.h>
#else
    #include <liblpe/util/util.h>
#endif
#include <reaper_plugin_functions.h>
#include <liblpe/LivePresetsExtension.h>
#include <liblpe/resources/resource.h>

DockWindow::DockWindow(int iResource, const char* cWndTitle, const char* cId, int iCmdID)
        : mHwnd(nullptr), mCmdId(iCmdID), mLayout(iResource), mTitle(cWndTitle), mId(cId) {

    if (cId && *cId) {// e.g. default constructor
        screenset_unregister((char*) cId);
        screenset_registerNew((char*) cId, screensetCallback, this);
    }

    mAr.translateAccel = keyHandler;
    mAr.isLocal = true;
    mAr.user = this;
    plugin_register("accelerator", &mAr);
}

DockWindow::~DockWindow() {
    plugin_register("-accelerator", &mAr);
    if (mId.length())
        screenset_unregister(mId.data());

    if (mHwnd) {
        DestroyWindow(mHwnd);
    }
}

/**
 * Is called after creation of the dialog. It applies the saved state if there is one.
 * @param hwndDlg the Handle to the dialog
 */
void DockWindow::initDialog(HWND hwndDlg) {
    mHwnd = hwndDlg;
    mResizer.init(mHwnd);

    SetWindowText(mHwnd, mTitle.data());

    //recall saved state
    onInitDlg();
}

/**
 * Saves the maximum size in the passed pointer
 * @param pointer that stores the size infos
 */
void DockWindow::getMinMaxInfo(LPMINMAXINFO info) {
    int w = 150;
    int h = 150;

    info->ptMinTrackSize.x = w;
    info->ptMinTrackSize.y = h;
}

/**
 * Called to resize the window components
 */
void DockWindow::resize() {
    mResizer.onResize();
    onResize();
}

/**
 * Shows the window when it's is not showing and hides it when it is showing
 */
void DockWindow::toggleVisibility() {
    if (isVisible()) {
        close();
    } else {
        show();
    }
}

void DockWindow::focus() {
    if (isDocked()) {
        DockWindowActivate(mHwnd);
    }
    SetFocus(mHwnd);
}

/**
 * Indicates the docking state
 * @return true when docked, false if not
 */
bool DockWindow::isDocked() {
    return DockIsChildOfDock(mHwnd, nullptr) != -1;
}

/**
 * Toggles the docking state and shows the window
 */
void DockWindow::toggleDocking() {
    if (isDocked()) {
        //undocking only works by destroying the window and recreating it
        DockWindowRemove(mHwnd);
        close();
        show(false);
    } else {
        if (!isVisible()) {
            show();
        }
        DockWindowActivate(mHwnd);
        DockWindowAddEx(mHwnd, mTitle.data(), mId.data(), true);
    }
}

/**
 *
 * @return
 */
bool DockWindow::isActive() {
    if (!mHwnd)
        return false;

    HWND hfoc = GetFocus();
    return hfoc == mHwnd || IsChild(mHwnd, hfoc);
}

/**
 * A function called by reaper to implement the screenset functions to this window
 * @param action The type of action called by screenset
 * @param id The id of the window to receive information from
 * @param param User defined value
 * @param actionParm custom data
 * @param actionParmSize custom data
 * @return Return the information reaper queries
 */
LRESULT DockWindow::screensetCallback(int action, const char*, void* param, void* actionParm, int actionParmSize) {
    auto* wnd = (DockWindow*) param;
    switch (action) {
        case SCREENSET_ACTION_GETHWND:
            return wnd->mHwnd != nullptr;
        case SCREENSET_ACTION_IS_DOCKED:
            return wnd->isDocked();
        case SCREENSET_ACTION_SWITCH_DOCK:
            if (wnd->mHwnd)
                wnd->toggleDocking();
            return 0;
        case SCREENSET_ACTION_LOAD_STATE:
            wnd->loadScreensetState((char *) actionParm, actionParmSize);
            return 0;
        case SCREENSET_ACTION_SAVE_STATE:
            return wnd->saveScreensetState((char *) actionParm, actionParmSize);
        default:
            return 0;
    }
}

/**
 * Shows the visibility of the window
 * @return true if visible, false if not
 */
bool DockWindow::isVisible() {
    return IsWindowVisible(mHwnd);
}

/**
 * Shows the window by creating it or making it visible if it already exists
 */
void DockWindow::show(bool loadState) {
    if (!mHwnd) {
        const char* resId = MAKEINTRESOURCE(mLayout);

        //CreateDialogParam is NOT blocking
        CreateDialogParam(g_lpe->mInstance, resId, g_lpe->mMainHwnd, sWndProc, (LPARAM) this);

        ShowWindow(mHwnd, SW_SHOW);

        if (loadState) {
            DockWindowState state{};
            GetPrivateProfileStruct("LPE", mId.data(), &state, sizeof(state), get_ini_file());
            loadStateFromPersistance(state);
        }
    }
}

/**
 * Closes the window by destroying it but keeping this class alive
 */
void DockWindow::close() {
    DockWindowRemove(mHwnd);
    DestroyWindow(mHwnd);
}

int DockWindow::keyHandler(MSG* msg, accelerator_register_t* ctx) {
    auto* wnd = reinterpret_cast<DockWindow*>(ctx->user);

    if (wnd && wnd->mHwnd && wnd->isActive()) {
        int iKeys = GetAsyncKeyState(VK_CONTROL) & 0x8000 ? LVKF_CONTROL : 0;
        iKeys |= GetAsyncKeyState(VK_MENU) & 0x8000 ? LVKF_ALT : 0;
        iKeys |= GetAsyncKeyState(VK_SHIFT) & 0x8000 ? LVKF_SHIFT : 0;

        //pass the keys together with additional keystates
        int iRet = wnd->onKey(msg, iKeys);
        return iRet ? iRet : -666;
    }
    return 0;
}

DockWindowState DockWindow::getStateForPersistance() {
    DockWindowState state{};
    state.dockId = DockIsChildOfDock(mHwnd, nullptr);
    RECT r = RECT();
    GetWindowRect(mHwnd, &r);
    state.pos = r;
    state.visible = isVisible();
    return state;
}

void DockWindow::loadStateFromPersistance(DockWindowState state) {
    if (state.visible == 0) {
        close();
    } else {
        if (state.visible == 1) {
            show(false);
        }
        if (state.dockId != -1) {
            DockWindowActivate(mHwnd);
            DockWindowAddEx(mHwnd, mTitle.data(), mId.data(), true);
            Dock_UpdateDockID(mId.data(), state.dockId);
        } else {
            RECT r = state.pos;
            EnsureNotCompletelyOffscreen(&r);
            SetWindowPos(mHwnd, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, 0);
        }
    }
}

/**
 * Called when WM_DESTROY was received. Resets the window parameters of this dialog.
 */
void DockWindow::onDestroy() {
    DockWindowState state = getStateForPersistance();
    state.visible = -1; //don't save visibility
    WritePrivateProfileStruct("LPE", mId.data(), &state, sizeof(state), get_ini_file());

    mHwnd = nullptr;
    mResizer.init(nullptr);
}

/**
 * Save the state of this window for the screenset function
 * @param cStateBuf the string used to save the data in
 * @param iMaxLen the maximum size of the saved data
 * @return size of written data
 */
int DockWindow::saveScreensetState(char* cStateBuf, int iMaxLen) {
    DockWindowState state = getStateForPersistance();
    int len = sizeof(state);

    if (iMaxLen < len)
        return -1;

    memcpy(cStateBuf, &state, len);
    return len;
}

/**
 * Recall the state from the data given by cStateBuf for screensets
 * @param cStateBuf The data
 * @param iLen Length of useable data
 */
void DockWindow::loadScreensetState(const char* cStateBuf, int iLen) {
    DockWindowState state{};
    memcpy(&state, cStateBuf, iLen);
    loadStateFromPersistance(state);
}

/**
 * A function called by the UI system to notify the user on certain actions
 * @param hwndDlg The handle to the dialog which notifies
 * @param uMsg Notification type
 * @param wParam custom data
 * @param lParam user defined data, only on callbacks to directly executed functions (e.g. CreateDialogParam)
 * @return
 */
INT_PTR WINAPI DockWindow::sWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //fprintf(stderr, "DockMsg: %i\n", uMsg);
    auto* wnd = reinterpret_cast<DockWindow*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
    if (!wnd && uMsg == WM_INITDIALOG) {
        wnd = reinterpret_cast<DockWindow*>(lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
        wnd->initDialog(hwndDlg);
    }

    if (wnd) {
        switch (uMsg) {
            case WM_COMMAND:
                wnd->onCommand(wParam, lParam);
                //first look for standard actions, then look for reaper actions
                switch (wParam) {
                    case IDCANCEL:
                        wnd->close();
                        break;
                    case ID_TOGGLE_DOCK:
                        wnd->toggleDocking();
                        break;
                    default:
                        return g_lpe->mActions.run(wParam);
                }
                break;
            case WM_NOTIFY:
                return wnd->onNotify(wParam, lParam);
            case WM_CONTEXTMENU: {
                HMENU menu = CreatePopupMenu();
                wnd->onContextMenu(menu);
                wnd->showContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), menu);
                break;
            }
            case WM_GETMINMAXINFO: {
                auto info = (LPMINMAXINFO) lParam;
                wnd->getMinMaxInfo(info);

                //adjust to window decorations
                if (!wnd->isDocked()) {
                    RECT rClient, rWnd;
                    GetClientRect(wnd->mHwnd, &rClient);
                    GetWindowRect(wnd->mHwnd, &rWnd);
                    info->ptMinTrackSize.x += (rWnd.right - rWnd.left) - rClient.right;
                    info->ptMinTrackSize.y += (rWnd.bottom - rWnd.top) - rClient.bottom;
                }
                break;
            }
            case WM_SIZE:
                if (wParam != SIZE_MINIMIZED)
                    wnd->resize();
                break;
            case WM_DESTROY:
                //called when reaper is closed and destroys the window
                wnd->onDestroy();
                wnd->onClose();
                break;
            default: {
                wnd->onUnhandledMsg(uMsg, wParam, lParam);
            }
        }
    }
    return false;
}

void DockWindow::showContextMenu(int x, int y, HMENU menu) {
    //show docking/undocking option
    MENUITEMINFO mii = MENUITEMINFO();
    std::string text = (isDocked() ? "Undock " : "Dock ") + mTitle;
    mii.fMask |= MIIM_TYPE | MIIM_ID;
    mii.fType |= MFT_STRING;
    mii.cbSize = sizeof(MENUITEMINFO);

    mii.dwTypeData = text.data();
    mii.cch = text.size();

    //wID specifies the commandId returned to sWndProc as wParam and WM_COMMAND onClick
    mii.wID = ID_TOGGLE_DOCK;
    InsertMenuItem(menu, 0, true, &mii);

    //show closing option
    mii = MENUITEMINFO();
    mii.fMask |= MIIM_TYPE | MIIM_ID;
    mii.fType |= MFT_STRING;
    mii.cbSize = sizeof(MENUITEMINFO);

    text = "Close " + mTitle;
    mii.dwTypeData = text.data();
    mii.cch = text.size();

    //wID specifies the commandId returned to sWndProc as wParam and WM_COMMAND onClick
    mii.wID = IDCANCEL;
    InsertMenuItem(menu, 1, true, &mii);

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, mHwnd, nullptr);
}