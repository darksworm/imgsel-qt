#pragma once

#include <QtWidgets/QWidget>
#include <QKeyEvent>
#include "../input/handler/InputHandler.h"
#include "picker/ImagePickerDrawer.h"

class MainWindow : public QWidget {
    void paintEvent(QPaintEvent *event) override;

public:
    explicit MainWindow(void (*fun)(WId));

public Q_SLOTS:
    void display();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void handleInstruction(InputInstruction *instruction);

    std::unique_ptr<InputHandler> inputHandler;

    void focusOutEvent(QFocusEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;

private:
    InputMode inputMode;
    QPixmap screenBuffer;
    ImagePickerDrawer *imagePickerDrawer;
    bool focused = false;
    std::function<void(WId)> shownCB;
};

