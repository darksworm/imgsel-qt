#pragma once

#include <QtWidgets/QWidget>
#include <QKeyEvent>
#include "../util/ThreadSafeQueue.h"
#include "../input/handler/InputHandler.h"
#include "ImagePickerMove.h"
#include "ImagePickerDrawer.h"

class MainWindow: public QWidget {
    void paintEvent(QPaintEvent *event) override;

//    ThreadSafeQueue<> *eventQueue;
public:
    MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void handleInstruction(InputInstruction* instruction);

    std::unique_ptr<InputHandler> inputHandler;

private:
    QThreadPool* threadPool;
    std::vector<QFuture<std::optional<QImage>>> imageFutures;
    InputMode inputMode;
    QPixmap screenBuffer;
    ImagePickerDrawer* imagePickerDrawer;
};

