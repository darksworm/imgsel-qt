#pragma once

#include <QtWidgets/QWidget>
#include <QKeyEvent>
#include "../input/handler/InputHandler.h"
#include "picker/ImagePickerDrawer.h"

class MainWindow : public QWidget {
    Q_OBJECT
    void paintEvent(QPaintEvent *event) override;

public:
    explicit MainWindow();

public slots:
    void display(bool invalidateConfig = false);

signals:
    void displayed(WId windowId);
    void exitInstructionReceived();
    void imageCopied();

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
};

