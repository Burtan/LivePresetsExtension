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
    #include <shellscalingapi.h>
#else

#endif
#include <reaper_plugin_functions.h>
#include <liblpe/LivePresetsExtension.h>

ModalWindow::ModalWindow(int iResource, const char* cWndTitle, const char* cId, int iCmdID)
        : mHwnd(nullptr), mCmdId(iCmdID), mLayout(iResource), mTitle(cWndTitle), mId(cId) {

    // screensets enable reaper to persist layouts
    if (cId && *cId) {
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

    // recall default saved state
    ModalWindowState state{};
    GetPrivateProfileStruct("LPE", mId.data(), &state, sizeof(state), get_ini_file());
    loadStateFromPersistance(state);

    onInitDlg();
    resize();
}

/**
 * Save the minimum and maximum size in the passed pointer. It is not dpi aware.
 * @param info pointer that stores the size infos
 */
void ModalWindow::getMinMaxInfo(LPMINMAXINFO info) {
    double dpiFactor = 1.5;
    int staticX = 0;
    int staticY = 0;

#ifdef _WIN32
    // 310/92 in .rc scales to 465/481/150/189 at 100 %, 930/958/288/359 at 200 %,
    // 100 % is 1,5 * x + 0/16 and 1,5 * y + 12/51 (16 width and 39 height for window decorations at 100 %)
    // 200 % is 3 * x + 0/28 and 3 * y + 12/83 (28 width and 71 height for window decorations at 200 %)
    DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext());
    if (dpiAwareness == DPI_AWARENESS_PER_MONITOR_AWARE) {
        dpiFactor = 3 * GetDpiForWindow(mHwnd) / 192.0;
    } else if (dpiAwareness == DPI_AWARENESS_UNAWARE) {
        // only works on DPI_AWARENESS_UNAWARE
        auto monitor = MonitorFromWindow(mHwnd, MONITOR_DEFAULTTONEAREST);
        DEVICE_SCALE_FACTOR scale;
        auto result = GetScaleFactorForMonitor(monitor, &scale);
        if (result == S_OK) {
            dpiFactor = 3 * scale / 200.0;
        } else {
            dpiFactor = 1.5;
        }
    } else {
        dpiFactor = 1.5;
    }

    staticY = 12;
#elif __APPLE__
    // 310/92 in .rc scales to 527/156/184 at 100 %
    // 100 % is 1,7 * x and 1,7 * y + 28 (28 height for window decorations at 100 %)
    // TODO
    /*RECT wnd, clnt;
    GetClientRect(mHwnd, &clnt);
    GetWindowRect(mHwnd, &wnd);

    char msg[512];
    snprintf(msg, 512,
        "client: %d, %d, %d, %d\nwnd: %d, %d, %d, %d\nfactor: %f\n",
        clnt.left, clnt.top, clnt.right - clnt.left, clnt.bottom - clnt.top,
        wnd.left, wnd.top, wnd.right - wnd.left, wnd.bottom - wnd.top,
        dpiFactor
    );
    ShowConsoleMsg(msg);*/
    dpiFactor = 1.7;
#else
    // LPMINMAX corresponds to Window rect 1:2 for 100 % and scales linear
    // 310/92 in .rc scales to 620/184 at 100 % and 1240/368 at 200 % for client and window rect
    dpiFactor = SWELL_GetScaling256() / 128.0;
#endif

    info->ptMinTrackSize.x = getMinWidth() * dpiFactor + staticX;
    info->ptMinTrackSize.y = getMinHeight() * dpiFactor + staticY;
}

/**
 * Called to resize the window components
 */
void ModalWindow::resize() {
    mResizer.onResize();
    onResize();
}

[[maybe_unused]] void ModalWindow::focus() {
    SetFocus(mHwnd);
}

/**
 *
 * @return
 */
[[maybe_unused]] bool ModalWindow::isActive() {
    if (!mHwnd)
        return false;

    HWND hfoc = GetFocus();
    return hfoc == mHwnd || IsChild(mHwnd, hfoc);
}

/**
 * A function called by reaper to implement the screenset functions to this window. Screensets are
 * recallable layouts within reaper
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
    auto rect = RECT();
    GetWindowRect(mHwnd, &rect);
    state.top = rect.top;
    state.left = rect.left,
    state.width = rect.right - rect.left;
    state.height = rect.bottom - rect.top;
    return state;
}

void ModalWindow::loadStateFromPersistance(const ModalWindowState& state) {
    auto rect = RECT{state.left, state.top, state.left + state.width, state.top + state.height};
    EnsureNotCompletelyOffscreen(&rect);
    SetWindowPos(mHwnd, nullptr, state.left, state.top, state.width, state.height, 0);
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
    if (iLen <= 0)
        return;

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
                //certain keys are used to navigate between controls on Windows os and are not passed to the dlgProc
                //VK_UP, VK_LEFT, VK_RIGHT, VK_DOWN, VK_TAB
                //which keys are passed to dlgProc depends on the control having the focus, the os and a custom wndProc
                //of that control. On windows controls eat all keys. On linux and mac only used keys are eaten. On linux
                //and windows, modal dialogs with controls always give the focus to any control, on Mac no control can
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