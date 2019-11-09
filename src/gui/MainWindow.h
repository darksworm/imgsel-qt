#pragma once

#include <QtWidgets/QWidget>
#include <QKeyEvent>
#include "../util/ThreadSafeQueue.h"
#include "../input/handler/InputHandler.h"
#include "ImagePickerMove.h"

class MainWindow: public QWidget {
    void paintEvent(QPaintEvent *event) override;

//    ThreadSafeQueue<> *eventQueue;
public:
    MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void handleInstruction(InputInstruction* instruction);

    void clearFilter();

    bool move(ImagePickerMove move, unsigned int steps);

    void setFilter(std::function<bool(Image *)> filter, std::string filterString);

    std::unique_ptr<InputHandler> inputHandler;
    InputMode inputMode;
    std::string filterString;
    Image *selectedImage;
};

