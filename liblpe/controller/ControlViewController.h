//
// Created by frederik on 01.02.20.
//

#ifndef LPE_CONTROLVIEWCONTROLLER_H
#define LPE_CONTROLVIEWCONTROLLER_H

#include <set>
#include <liblpe/ui/base/DockWindow.h>
#include <liblpe/ui/base/ListView.h>
#include <liblpe/data/models/Hardware.h>

class ControlViewController : public DockWindow {
public:
    explicit ControlViewController();

    void reset();
protected:
    void onCommand(WPARAM wParam, LPARAM lParam) override;
    void onContextMenu(HMENU menu) override;
    int onKey(MSG* msg, int iKeyState) override;
    int onNotify(WPARAM wParam, LPARAM lParam) override;
    void onClose() override;
    void onResize() override;
    void onInitDlg() override;
    LONG getMinWidth() override { return 416; }
    LONG getMinHeight() override { return 180; }
private:
    HWND tabHwnd = nullptr;
    std::set<int> mControlIds;
    std::unique_ptr<ListView<Control>> mList = nullptr;
    WDL_WndSizer mTabResizer;

    void addHardware();
    void removeCurrentHardware();
    void addControl();
    void removeSelectedControls();
    void createList();
    void showTab(int index);
    void showControl(Hardware* hw, int index);
    void assignParamToControl();
};

inline void ControlViewController_Reset(ControlViewController* controller) {
    controller->reset();
}
inline void ControlViewController_ToggleVisibility(ControlViewController* controller) {
    controller->toggleVisibility();
}
inline bool ControlViewController_IsVisible(ControlViewController* controller) {
    return controller->isVisible();
}


#endif //LPE_CONTROLVIEWCONTROLLER_H