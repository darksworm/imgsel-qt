#include <QtGui/QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QClipboard>
#include <iostream>
#include "MainWindow.h"
#include "../config/ConfigManager.h"
#include "../input/handler/InputHandlerFactory.h"
#include "../input/handler/instruction/MoveInstruction.h"
#include "../input/handler/instruction/ModeChangeInstruction.h"
#include "../input/handler/instruction/FilterInstruction.h"
#include "../input/handler/instruction/CopyInstruction.h"

void MainWindow::paintEvent(QPaintEvent *event) {
    auto config = ConfigManager::getOrLoadConfig();
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(0, 0, config.getScreenGeometry().width(), config.getScreenGeometry().height(),
                     QColor(QRgba64::fromRgba(0, 0, 0, 200)));

    painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter.drawPixmap(0, 0, screenBuffer);
    painter.end();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    auto instruction = std::unique_ptr<InputInstruction>(inputHandler->handleKeyPress(event->key()));
    handleInstruction(instruction.get());
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    auto instruction = std::unique_ptr<InputInstruction>(inputHandler->handleKeyRelease(event->key()));
    handleInstruction(instruction.get());
}

void MainWindow::handleInstruction(InputInstruction *instruction) {
    auto config = ConfigManager::getOrLoadConfig();

    bool shouldExit = this->imagePickerDrawer->getFilterString().empty() && instruction->getType() == InputInstructionType::CANCEL;

    if (shouldExit || instruction->getType() == InputInstructionType::EXIT) {
        QCoreApplication::exit(1);
    }

    if (instruction->getType() == InputInstructionType::CANCEL) {
        this->imagePickerDrawer->clearFilter();
    } else if (dynamic_cast<MoveInstruction *>(instruction)) {
        auto moveInstruction = ((MoveInstruction *) instruction);
        auto move = moveInstruction->getMoveDirection();

        bool moved = false;

        if (move != ImagePickerMove::NONE) {
            moved = this->imagePickerDrawer->move(moveInstruction->getMoveDirection(), moveInstruction->getMoveSteps());
        }

        if (move == ImagePickerMove::NONE || !moved) {
            this->imagePickerDrawer->drawFrame(this->imagePickerDrawer->getSelectedImage(), false);
        }
    } else if (dynamic_cast<ModeChangeInstruction *>(instruction)) {
        auto modeChangeInstruction = (ModeChangeInstruction *) instruction;
        inputMode = modeChangeInstruction->newMode;
        this->imagePickerDrawer->drawFrame(nullptr, false);

        if (modeChangeInstruction->shouldClearFilters) {
            this->imagePickerDrawer->clearFilter();
        }
    } else if (dynamic_cast<FilterInstruction *>(instruction)) {
        auto filterInstruction = ((FilterInstruction *) instruction);

        if (!filterInstruction->getFilterString().empty()) {
            this->imagePickerDrawer->setFilter(filterInstruction->getFilter(), filterInstruction->getFilterString());
        } else {
            this->imagePickerDrawer->clearFilter();
        }

        this->imagePickerDrawer->drawFrame(nullptr);
    } else if (dynamic_cast<CopyInstruction *>(instruction)) {
        auto selectedImage = this->imagePickerDrawer->getSelectedImage();
        auto path = selectedImage->getPath();

        if (config.shouldPrintFilePath()) {
            std::cout << path;
        } else {
#if defined(Q_OS_LINUX)
            auto ext = selectedImage->getExtension();
            ext = ext == "jpg" ? "jpeg" : ext;

            // TODO: handle 's in filenames
            std::string command = "cat '" + path + "' | xclip -selection clipboard -target image/" + ext + " -i";
            system(command.c_str());
#else
            auto img = QPixmap(QString(path.c_str()));
            QApplication::clipboard()->setPixmap(img, QClipboard::Clipboard);
            QApplication::clipboard()->setPixmap(img, QClipboard::Selection);
            QApplication::processEvents();
#endif
        }

        QCoreApplication::exit(0);
    }

    this->repaint();
}

MainWindow::MainWindow() : QWidget() {
    auto config = ConfigManager::getOrLoadConfig();
    inputHandler.reset();
    inputMode = config.getDefaultInputMode();

    if (!InputHandlerFactory::isCorrectHandler(inputHandler.get(), inputMode)) {
        inputHandler.reset(InputHandlerFactory::getInputHandler(inputMode));
    }
    this->threadPool = new QThreadPool(this);
    this->threadPool->setMaxThreadCount(10);

    this->imagePickerDrawer = new ImagePickerDrawer(screenBuffer);

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    auto geo = screen->geometry();

    screenBuffer = QPixmap(geo.width(), geo.height());

    this->imagePickerDrawer->drawFrame(this->imagePickerDrawer->getSelectedImage(), true);
}

