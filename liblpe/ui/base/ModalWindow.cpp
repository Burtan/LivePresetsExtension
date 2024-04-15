/******************************************************************************
/ LivePresetsExtension
/
/ Base class for modal windows
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

#include <liblpe/ui/base/ModalWindow.h>
#ifdef _WIN32
    #include <WinUser.h>
    #include <windowsx.h>
#else

#endif
#include <reaper_plugin_functions.h>
#include <liblpe/LivePresetsExtension.h>

ModalWindow::ModalWindow(int iResource, const char* cWndTitle, const char* cId, int iCmdID)
        : mHwnd(nullptr), mLayout(iResource), mTitle(cWndTitle), mId(cId), mCmdId(iCmdID) {

    if (cId && *cId) {// e.g. default constructor
        screenset_unregister((char*) cId);
        screenset_registerNew((char*) cId, screensetCallback, this);
    }
}

ModalWindow::~ModalWindow() {
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
void ModalWindow::initDialog(HWND hwndDlg) {
    mHwnd = hwndDlg;
    mResizer.init(mHwnd);

    SetWindowText(mHwnd, mTitle.data());

    //recall saved state
    ModalWindowState state{};
    GetPrivateProfileStruct("LPE", mId.data(), &state, sizeof(state), get_ini_file());
    loadStateFromPersistance(state);

    onInitDlg();
    resize();
}

/**
 * Save the minimum and maximum size in the passed pointer. It is not dpi aware.
 * @param pointer that stores the size infos
 */
void ModalWindow::getMinMaxInfo(LPMINMAXINFO info) {
    int w = 150;
    int h = 150;

    info->ptMinTrackSize.x = w;
    info->ptMinTrackSize.y = h;
}

/**
 * Called to resize the window components
 */
void ModalWindow::resize() {
    mResizer.onResize();
    onResize();
}

void ModalWindow::focus() {
    SetFocus(mHwnd);
}

/**
 *
 * @return
 */
bool ModalWindow::isActive() {
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
LRESULT ModalWindow::screensetCallback(int action, const char*, void* param, void* actionParm, int actionParmSize) {
    auto* wnd = (ModalWindow*) param;
    switch (action) {
        case SCREENSET_ACTION_GETHWND:
            return wnd->mHwnd != nullptr;
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
 * Shows the window by creating it or making it visible if it already exists
 */
LPARAM ModalWindow::show() {
    if (!mHwnd) {
        const char* resId = MAKEINTRESOURCE(mLayout);
        //DialogBoxParam is blocking
        DialogBoxParam(g_lpe->mInstance, resId, g_lpe->mMainHwnd, dlgProc, (LPARAM) this);
        return result();
    }
    return 0;
}

/**
 * Closes the window by destroying it but keeping this class alive when its not modal
 */
void ModalWindow::close() {
    EndDialog(mHwnd, 1);
}

ModalWindowState ModalWindow::getStateForPersistance() {
    ModalWindowState state{};
    RECT r = RECT();
    GetWindowRect(mHwnd, &r);
    state.pos = r;
    return state;
}

void ModalWindow::loadStateFromPersistance(ModalWindowState state) {
    RECT r = state.pos;
    EnsureNotCompletelyOffscreen(&r);
    SetWindowPos(mHwnd, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top, 0);
}

/**
 * Called when WM_DESTROY was received. Resets the window parameters of this dialog.
 */
void ModalWindow::onDestroy() {
    ModalWindowState state = getStateForPersistance();
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
int ModalWindow::saveScreensetState(char* cStateBuf, int iMaxLen) {
    ModalWindowState state = getStateForPersistance();
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
void ModalWindow::loadScreensetState(const char* cStateBuf, int iLen) {
    ModalWindowState state{};
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
INT_PTR WINAPI ModalWindow::dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto* wnd = reinterpret_cast<ModalWindow*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
    if (!wnd && uMsg == WM_INITDIALOG) {
        wnd = reinterpret_cast<ModalWindow*>(lParam);
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
        wnd->initDialog(hwndDlg);
    }

    if (wnd) {
        switch (uMsg) {
            case WM_SHOWWINDOW:
                break;
            case WM_COMMAND:
                wnd->onCommand(wParam, lParam);
                //first look for standard actions, then look for reaper actions
                switch (wParam) {
                    case IDCANCEL:
                        wnd->close();
                        break;
                    default:
                        return g_lpe->mActions.run(wParam);
                }
                break;
            case WM_NOTIFY:
                wnd->onNotify(wParam, lParam);
                break;
            case WM_CONTEXTMENU: {
                HMENU menu = CreatePopupMenu();
                wnd->onContextMenu(menu);
                wnd->showContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), menu);
                break;
            }
            //define a minimum/maximum size
            case WM_GETMINMAXINFO: {
                auto info = (LPMINMAXINFO) lParam;
                wnd->getMinMaxInfo(info);

                //adjust to window decorations
                RECT rClient, rWnd;
                GetClientRect(wnd->mHwnd, &rClient);
                GetWindowRect(wnd->mHwnd, &rWnd);
                info->ptMinTrackSize.x += (rWnd.right - rWnd.left) - rClient.right;
                info->ptMinTrackSize.y += (rWnd.bottom - rWnd.top) - rClient.bottom;
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
            case WM_KEYDOWN: {
                //certain keys are used to navigate between controls on windows os and are not passed to the dlgProc
                //VK_UP, VK_LEFT, VK_RIGHT, VK_DOWN, VK_TAB
                //which keys are passed to dlgProc depends on the control having the focus, the os and a custom wndProc
                //of that control. On windows controls eat all keys. On linux and mac only used keys are eaten. On linux
                //and windows, modal dialogs with controls always give the focus to any control, on mac no control can
                //have the focus
                MSG msg{};
                msg.hwnd = hwndDlg;
                msg.message = uMsg;
                msg.wParam = wParam;
                msg.lParam = lParam;
                return wnd->onKey(&msg, lParam & 24);
            }
            default:
                wnd->onUnhandledMsg(uMsg, wParam, lParam);
        }
    }
    return 0;
}

void ModalWindow::showContextMenu(int x, int y, HMENU menu) {
    //show docking/undocking option
    MENUITEMINFO mii{}; MENUITEMINFO();

    //show closing option
    mii = MENUITEMINFO();
    mii.fMask |= MIIM_TYPE | MIIM_ID;
    mii.fType |= MFT_STRING;
    mii.cbSize = sizeof(MENUITEMINFO);

    std::string text = "Close " + mTitle;
    mii.dwTypeData = text.data();
    mii.cch = text.size();

    //wID specifies the commandId returned to sWndProc as wParam and WM_COMMAND onClick
    mii.wID = IDCANCEL;
    InsertMenuItem(menu, 1, true, &mii);

    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, mHwnd, nullptr);
}