//
// Created by Frederik on 13.03.2020.
//

#ifndef LIVEPRESETSEXTENSION_MODALTEST_H
#define LIVEPRESETSEXTENSION_MODALTEST_H

#include <ui/base/ModalWindow.h>
#include <resources/resource.h>

class ModalTest : public ModalWindow {
public:
    ModalTest() : ModalWindow(IDD_ABOUT, "", "AboutController", 0) {}

protected:
    LPARAM result() override {
        return 0;
    }
};

#endif //LIVEPRESETSEXTENSION_MODALTEST_H
