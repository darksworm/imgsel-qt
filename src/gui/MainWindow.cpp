#include <QtGui/QPainter>
#include <QtConcurrent/QtConcurrent>
#include "MainWindow.h"
#include "../config/ConfigManager.h"
#include "../input/handler/InputHandlerFactory.h"
#include "../input/handler/instruction/MoveInstruction.h"
#include "../input/handler/instruction/ModeChangeInstruction.h"
#include "../input/handler/instruction/FilterInstruction.h"
#include "../input/handler/instruction/CopyInstruction.h"

std::optional<QImage> loadImage(std::string path) {
    std::optional<QImage> result;
    result.reset();

    auto config = ConfigManager::getOrLoadConfig();

    QImage image(path.c_str());

    if(image.isNull()) {
        return result;
    }

    auto width = image.width();
    auto height = image.height();

    if (config.getMaxImageHeight() + config.getMaxImageWidth() > 0) {
        if (config.getMaxImageWidth() > 0 && width > config.getMaxImageWidth()) {
            auto scale = (double)config.getMaxImageWidth() / width;
            int new_height = height * scale;
            image = image.scaledToHeight(new_height);
        }

        if (config.getMaxImageHeight() > 0 && height > config.getMaxImageHeight()) {
            auto scale = (double)config.getMaxImageHeight() / height;
            int new_width = width * scale;
            image = image.scaledToWidth(new_width);
        }
    }

    result = image;
    return result;
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QElapsedTimer timer;
    timer.start();
    auto config = ConfigManager::getOrLoadConfig();
    QWidget::paintEvent(event);
    QPainter painter(this);
    QFutureSynchronizer<void> qFutureSynchronizer;

    auto images = config.getImages();

    int i = 0;
    for(auto &img : config.getImages()) {
        QFuture<std::optional<QImage>> imgFuture = QtConcurrent::run(threadPool, &loadImage, img.getPath());
        qFutureSynchronizer.addFuture(imgFuture);
        imageFutures.emplace_back(imgFuture);

        if(i++ > 88) {
            break;
        }
    }

    auto x = 70, y = 50;

    qFutureSynchronizer.waitForFinished();
    for(auto &d: imageFutures) {
        auto res = d.result();
        if(res.has_value()) {
            painter.drawImage( x, y, res.value() );
            x += 100;

            if(x > 1800) {
                x = 50;
                y += 100;
            }
        }
    }

    qDebug() << "The slow operation took" << timer.elapsed() << "milliseconds";
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
    this->threadPool = new QThreadPool(this);
    this->threadPool->setMaxThreadCount(10);
}

void MainWindow::setFilter(std::function<bool(Image *)> filter, std::string filterString) {

}

bool MainWindow::move(ImagePickerMove move, unsigned int steps) {
    return false;
}

void MainWindow::clearFilter() {

}
