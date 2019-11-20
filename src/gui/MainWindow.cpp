#include <QtGui/QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QClipboard>
#include <QtWidgets/QApplication>
#include <iostream>
#include "MainWindow.h"
#include "../util/config/ConfigManager.h"
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
    if (focused) {
        painter.drawPixmap(0, 0, screenBuffer);
    } else {
        QImage im = screenBuffer.toImage().convertToFormat(QImage::Format_ARGB32);
        for (int y = 0; y < im.height(); ++y) {
            QRgb *scanLine = (QRgb *) im.scanLine(y);
            for (int x = 0; x < im.width(); ++x) {
                QRgb pixel = *scanLine;
                uint ci = uint(qGray(pixel));
                *scanLine = qRgba(ci, ci, ci, qAlpha(pixel) / 3);
                ++scanLine;
            }
        }

        painter.drawImage(0, 0, im);
    }

    if (imagePickerDrawer->getFilterString().length() > 0) {
        QFont queryFont;
        queryFont.setFamily(queryFont.defaultFamily());
        queryFont.setPixelSize(28);
        QFontMetrics fm(queryFont);

        QString displayName = QString::fromUtf8(imagePickerDrawer->getFilterString().c_str());
        unsigned int textWidth = 0;
        auto maxTextWidth = config.getScreenGeometry().width() - 20;

        do {
            textWidth = fm.horizontalAdvance(displayName);

            if (textWidth >= maxTextWidth) {
                displayName = displayName.left(displayName.length() - 1);
            }
        } while (textWidth >= maxTextWidth);

        painter.setFont(queryFont);
        painter.setPen(Qt::white);

        painter.drawText((config.getScreenGeometry().width() - textWidth) / 2, 36, displayName);
    }

    QFont inputModeFont;
    inputModeFont.setFamily(inputModeFont.defaultFamily());
    inputModeFont.setPixelSize(12);

    painter.setFont(inputModeFont);
    painter.setPen(Qt::white);

    auto modeText = inputMode == InputMode::VIM ? "VIM" : "DEFAULT";
    painter.drawText(5, config.getScreenGeometry().height() - 10, modeText);

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

    bool shouldExit = this->imagePickerDrawer->getFilterString().empty() &&
                      instruction->getType() == InputInstructionType::CANCEL;

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
        inputHandler.reset(InputHandlerFactory::getInputHandler(inputMode));
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

        if (selectedImage == nullptr) {
            return;
        }

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

    this->imagePickerDrawer = new ImagePickerDrawer(screenBuffer);

    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    auto geo = screen->geometry();

    screenBuffer = QPixmap(geo.width(), geo.height());

    this->imagePickerDrawer->drawFrame(this->imagePickerDrawer->getSelectedImage(), true);
}

void MainWindow::focusOutEvent(QFocusEvent *event) {
    QWidget::focusOutEvent(event);
    focused = false;
    this->repaint();
}

void MainWindow::focusInEvent(QFocusEvent *event) {
    QWidget::focusInEvent(event);
    focused = true;
    this->repaint();
}

