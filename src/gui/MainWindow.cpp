#include <QtGui/QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QClipboard>
#include <QtWidgets/QApplication>
#include <iostream>
#include "MainWindow.h"
#include "../Application.h"
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

    QString displayName;

    if (imagePickerDrawer->getFilterString().length() > 0) {
        displayName = QString::fromUtf8(imagePickerDrawer->getFilterString().c_str());
    } else {
        displayName = "Type to search";
    }

    QFont queryFont;
    queryFont.setFamily(queryFont.defaultFamily());
    queryFont.setPixelSize(28);
    QFontMetrics fm(queryFont);

    unsigned int textWidth = 0;
    auto maxTextWidth = config.getScreenGeometry().width() - 20;

    do {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        textWidth = fm.horizontalAdvance(displayName);
#else
        textWidth = fm.width(displayName);
#endif

        if (textWidth >= maxTextWidth) {
            displayName = displayName.left(displayName.length() - 1);
        }
    } while (textWidth >= maxTextWidth);

    painter.setFont(queryFont);
    painter.setPen(Qt::white);

    painter.drawText((config.getScreenGeometry().width() - textWidth) / 2, 36, displayName);

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
        this->imagePickerDrawer->clearFilter();
        this->imagePickerDrawer->move(ImagePickerMove::HOME);

        emit exitInstructionReceived();
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
            auto preprocessFlags = dynamic_cast<CopyInstruction *>(instruction)->getPreprocessFlags();

            bool whatsappResize = preprocessFlags & PreprocessorFlags::WhatsAppWhitespace ||
                                  config.getPreprocessorFlags() & PreprocessorFlags::WhatsAppWhitespace;

            if (whatsappResize) {
                QPixmap result(320, 320);
                result.fill(Qt::transparent);
                QPixmap imgPixmap(selectedImage->getPath().c_str());
                QPainter painter(&result);

                painter.drawPixmap(
                        result.width() / 2 - imgPixmap.width() / 2,
                        result.height() / 2 - imgPixmap.height() / 2,
                        imgPixmap
                );

                auto tempImage = new QTemporaryFile;
                tempImage->open();
                result.toImage().save(tempImage, QString::fromStdString(
                        selectedImage->getExtension()).toUpper().toStdString().c_str());
                tempImage->close();

                path = tempImage->fileName().toStdString();
            }

            if (config.shouldResizeOutputImage()) {
                auto targetSize = config.getResizeOutputToSize().value();
                QImage image(selectedImage->getPath().c_str());

                auto width = image.width();
                auto height = image.height();

                if (!whatsappResize || width > 320 || height > 320) {
                    if (whatsappResize) {
                        targetSize.width = 300;
                        targetSize.height = 300;
                    }

                    if (targetSize.width > 0 && width > targetSize.width) {
                        auto scale = (double) targetSize.width / width;
                        int new_height = height * scale;
                        image = image.scaledToHeight(new_height, Qt::SmoothTransformation);

                        height = new_height;
                        width = width * scale;
                    }

                    if (targetSize.height > 0 && height > targetSize.height) {
                        auto scale = (double) targetSize.height / height;
                        int new_width = width * scale;
                        image = image.scaledToWidth(new_width, Qt::SmoothTransformation);                    
                    }

                    auto tempImage = new QTemporaryFile;
                    tempImage->open();
                    image.save(tempImage,
                               QString::fromStdString(selectedImage->getExtension()).toUpper().toStdString().c_str());
                    tempImage->close();

                    path = tempImage->fileName().toStdString();
                }
            }

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

        if (config.shouldResizeOutputImage()) {
            QFile file(QString::fromStdString(path));
            file.remove();
        }

        this->imagePickerDrawer->clearFilter();
        this->imagePickerDrawer->move(ImagePickerMove::HOME);

        emit exitInstructionReceived();
    }

    this->repaint();
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

void MainWindow::display(bool invalidateConfig) {
    if (invalidateConfig) {
        bool imageListChanged = ConfigManager::invalidateConfigIfImageListChanged();
        imagePickerDrawer->reset(imageListChanged);
    }

    auto config = ConfigManager::getOrLoadConfig();

    if (config.getImages().empty()) {
        QSettings settings("EMOJIGUN", "EMOJIGUN");
        auto defaultDir = Application::defaultLibraryDirectory();
        auto imageDirFromSettings = settings.value("library_path", defaultDir).toString();

        QMessageBox noImagesMsgBox;

        QPushButton *getEmojisBtn = noImagesMsgBox.addButton("Get emojis", QMessageBox::ActionRole);
        noImagesMsgBox.addButton("Ok", QMessageBox::ActionRole);

        noImagesMsgBox.setParent(nullptr);
        noImagesMsgBox.setIcon(QMessageBox::Icon::Critical);
        noImagesMsgBox.setWindowTitle("No images found!");
        noImagesMsgBox.setText(
            "To use EMOJIGUN, you must first add emojis to your library. Please copy your emojis to your library folder at " + imageDirFromSettings + "."
        );

        noImagesMsgBox.exec();

        if (noImagesMsgBox.clickedButton() == getEmojisBtn) {
            QString link = "https://emojigun.com/#/loader";
            QDesktopServices::openUrl(link);
        }

        return;
    }

    auto geo = config.getScreenGeometry();

    setGeometry(geo);

    inputHandler.reset();
    inputMode = config.getDefaultInputMode();

    if (!InputHandlerFactory::isCorrectHandler(inputHandler.get(), inputMode)) {
        inputHandler.reset(InputHandlerFactory::getInputHandler(inputMode));
    }

    imagePickerDrawer->drawFrame(nullptr, false);

    show();
    raise();
    setFocus();

    emit displayed(WId());
}

MainWindow::MainWindow() : QWidget() {
    setWindowTitle(QApplication::translate("APPLICATION", "EMOJIGUN"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint);

    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);

    auto config = ConfigManager::getOrLoadConfig();
    auto geo = config.getScreenGeometry();
    screenBuffer = QPixmap(geo.width(), geo.height());

    this->imagePickerDrawer = new ImagePickerDrawer(screenBuffer);
    this->imagePickerDrawer->drawFrame(this->imagePickerDrawer->getSelectedImage(), true);
}
