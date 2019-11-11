#pragma once

#include <QtWidgets/QWidget>
#include <QKeyEvent>
#include "../input/handler/InputHandler.h"
#include "ImagePickerMove.h"
#include "ImagePickerDrawer.h"

class MainWindow: public QWidget {
    void paintEvent(QPaintEvent *event) override;
public:
    MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void handleInstruction(InputInstruction* instruction);

    std::unique_ptr<InputHandler> inputHandler;

private:
    InputMode inputMode;
    QPixmap screenBuffer;
    ImagePickerDrawer* imagePickerDrawer;
};

