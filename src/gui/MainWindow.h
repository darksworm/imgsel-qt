#pragma once

#include <QTimer>
#include <QtWidgets/QWidget>
#include <QKeyEvent>
#include "../input/handler/InputHandler.h"
#include "picker/ImagePickerDrawer.h"
#include "../util/EmojiZipDownloader.h"

class MainWindow : public QWidget {
    Q_OBJECT
    void paintEvent(QPaintEvent *event) override;

public:
    explicit MainWindow();

public slots:
    void display(bool invalidateConfig = false);

private slots:
    void scrollEnd();

signals:
    void displayed(WId windowId);
    void exitInstructionReceived();
    void imageCopied();
    void noImagesToDisplay();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void handleInstruction(InputInstruction *instruction);

    std::unique_ptr<InputHandler> inputHandler;

    void focusOutEvent(QFocusEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void copyImage(Image* image, PreprocessorFlags preprocessFlags = PreprocessorFlags::None);

private:
    bool focused = false;

    const unsigned scrollTreshold = 20;
    const unsigned scrollEndMilis = 200;

    int xScrollAccumulator;
    int yScrollAccumulator;

    QTimer scrollingEndTimer;

    InputMode inputMode;
    QPixmap screenBuffer;
    ImagePickerDrawer *imagePickerDrawer;
};

