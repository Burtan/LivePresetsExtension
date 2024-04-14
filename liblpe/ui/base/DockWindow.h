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

#ifndef LPE_DOCKWINDOW_H
#define LPE_DOCKWINDOW_H

#include <string>
#ifdef _WIN32
    #include <Windows.h>
    #include <rpc.h>
#else
    #define WDL_NO_DEFINE_MINMAX
    #include <third_party/WDL/WDL/swell/swell-types.h>
    #include <third_party/WDL/WDL/swell/swell-dlggen.h>
#endif
#include <liblpe/plugins/reaper_plugin.h>
#include <third_party/WDL/WDL/wingui/wndsize.h>

typedef struct DockWindowState {
    int visible = 1; //-1 not saved, 0 hidden, 1 showing
    int dockId = -1; //-1 not docked, >= 0 dockId
    RECT pos = RECT{500, 200, 1000, 500}; // position and size
} DockWindowState;

class DockWindow {
public:
    explicit DockWindow(int iResource = 0, const char* cWndTitle = "", const char* cId = "", int iCmdID = 0);
    virtual ~DockWindow();

    void focus();
    bool isDocked();
    bool isVisible();
    void show(bool loadState = true);
    void close();
    void toggleVisibility();
    void toggleDocking();
    int saveScreensetState(char* cStateBuf, int iMaxLen);
    void loadScreensetState(const char* cStateBuf, int iLen);
    virtual bool isActive();
    virtual void onCommand(WPARAM wParam, LPARAM lParam) {}
protected:
    virtual int onNotify(WPARAM wParam, LPARAM lParam) { return 0; }
    virtual int onParentNotify(WPARAM wParam, LPARAM lParam) { return 0; }
    virtual void onContextMenu(HMENU menu) {}
    virtual void onResize() {}
    virtual void onClose() {}
    virtual void onInitDlg() {}
    virtual int onKey(MSG* msg, int iKeyState) { return 0; } // return 1 for "processed key"
    virtual INT_PTR onUnhandledMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }
    virtual void getMinMaxInfo(LPMINMAXINFO info);

    HWND mHwnd;
    int mCmdId;
    int mLayout;
    std::string mTitle;
    std::string mId;
    accelerator_register_t mAr{};
    WDL_WndSizer mResizer;
private:
    static LRESULT screensetCallback(int action, const char *id, void *param, void *actionParm, int actionParmSize);
    static INT_PTR WINAPI sWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static int keyHandler(MSG* msg, accelerator_register_t* ctx);
    void initDialog(HWND hwndDlg);
    void resize();
    void onDestroy();
    DockWindowState getStateForPersistance();
    void loadStateFromPersistance(DockWindowState state);
    void showContextMenu(int x, int y, HMENU menu);
};

#endif //LPE_DOCKWINDOW_H