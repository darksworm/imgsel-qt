#include <QtGui/QPainter>
#include "MainWindow.h"
#include "../config/ConfigManager.h"
#include "../input/handler/InputHandlerFactory.h"
#include "../input/handler/instruction/MoveInstruction.h"
#include "../input/handler/instruction/ModeChangeInstruction.h"
#include "../input/handler/instruction/FilterInstruction.h"
#include "../input/handler/instruction/CopyInstruction.h"

void MainWindow::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);
    QPainter painter(this);

    QPixmap pixmap("/home/ilmars/.temoji/AngelThump.png");
    std::cout << "paint" << "\n";
    painter.drawPixmap(500, 500, pixmap);
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

    bool shouldExit = filterString.empty() && instruction->getType() == InputInstructionType::CANCEL;

    if (shouldExit || instruction->getType() == InputInstructionType::EXIT) {
        QCoreApplication::exit(1);
    }

    if (instruction->getType() == InputInstructionType::CANCEL) {
        clearFilter();
        this->repaint();
    } else if (dynamic_cast<MoveInstruction *>(instruction)) {
        auto moveInstruction = ((MoveInstruction *) instruction);
        auto move = moveInstruction->getMoveDirection();

        bool moved = false;

        if (move != ImagePickerMove::NONE) {
            moved = this->move(moveInstruction->getMoveDirection(), moveInstruction->getMoveSteps());
        }

        if (move == ImagePickerMove::NONE || !moved) {
            this->repaint();
        }
    } else if (dynamic_cast<ModeChangeInstruction *>(instruction)) {
        auto modeChangeInstruction = (ModeChangeInstruction *) instruction;
        inputMode = modeChangeInstruction->newMode;

        if (modeChangeInstruction->shouldClearFilters) {
            clearFilter();
        }

        selectedImage = nullptr;
        this->repaint();
    } else if (dynamic_cast<FilterInstruction *>(instruction)) {
        auto filterInstruction = ((FilterInstruction *) instruction);

        if (!filterInstruction->getFilterString().empty()) {
            setFilter(filterInstruction->getFilter(), filterInstruction->getFilterString());
        } else {
            clearFilter();
        }

        selectedImage = nullptr;
        this->repaint();
    } else if (dynamic_cast<CopyInstruction *>(instruction)) {
        auto path = selectedImage->getPath();

        if (config.shouldPrintFilePath()) {
            std::cout << path;
        } else {
            auto ext = selectedImage->getExtension();
            ext = ext == "jpg" ? "jpeg" : ext;

            // TODO: handle 's in filenames
            std::string command = "cat '" + path + "' | xclip -selection clipboard -target image/" + ext + " -i";
            system(command.c_str());
        }

        QCoreApplication::exit(0);
    }
}

MainWindow::MainWindow() : QWidget() {
    auto config = ConfigManager::getOrLoadConfig();
    inputHandler.reset();
    inputMode = config.getDefaultInputMode();
    selectedImage = nullptr;

    if (!InputHandlerFactory::isCorrectHandler(inputHandler.get(), inputMode)) {
        inputHandler.reset(InputHandlerFactory::getInputHandler(inputMode));
    }

}

void MainWindow::setFilter(std::function<bool(Image *)> filter, std::string filterString) {

}

bool MainWindow::move(ImagePickerMove move, unsigned int steps) {
    return false;
}

void MainWindow::clearFilter() {

}
