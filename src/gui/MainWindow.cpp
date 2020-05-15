#include <QtGui/QPainter>
#include <QtConcurrent/QtConcurrent>
#include <QClipboard>
#include <QtWidgets/QApplication>
#include <iostream>
#include "MainWindow.h"
#include "../app/Application.h"
#include "../input/handler/InputHandlerFactory.h"
#include "../input/handler/instruction/MoveInstruction.h"
#include "../input/handler/instruction/ModeChangeInstruction.h"
#include "../input/handler/instruction/FilterInstruction.h"
#include "../input/handler/instruction/CopyInstruction.h"
#include "../assets/assets.h"

#ifdef WIN32
QT_BEGIN_NAMESPACE
Q_GUI_EXPORT HBITMAP qt_pixmapToWinHBITMAP(const QPixmap &p, int hbitmapFormat = 0);
#endif

void MainWindow::paintEvent(QPaintEvent *event) {
    auto config = ConfigManager::getOrLoadConfig();

    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
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

    int oneColumnHeight = imagePickerDrawer->getShapeProperties().getOneColumnHeight();
    int emptyHeight = (config.getScreenGeometry().height() - oneColumnHeight) / 2;

    painter.drawText((config.getScreenGeometry().width() - textWidth) / 2, emptyHeight / 2  + 6, displayName);

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
    xScrollAccumulator = 0;
    yScrollAccumulator = 0;

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
        copyImage(this->imagePickerDrawer->getSelectedImage());
    }

    this->repaint();
}

void MainWindow::copyImage(Image* image, PreprocessorFlags preprocessFlags) {
    if (image == nullptr) {
        return;
    }

    auto config = ConfigManager::getOrLoadConfig();
    auto path = image->getPath();

    QImage testImage(QString::fromStdString(path));
    if (testImage.isNull()) {
        return;
    }

    if (config.shouldPrintFilePath()) {
        std::cout << path;
    } else {
        bool whatsappResize = preprocessFlags & PreprocessorFlags::WhatsAppWhitespace ||
                              config.getPreprocessorFlags() & PreprocessorFlags::WhatsAppWhitespace;

        if (whatsappResize) {
            QPixmap result(320, 320);
            result.fill(Qt::transparent);
            QPixmap imgPixmap(image->getPath().c_str());
            QPainter painter(&result);

            painter.drawPixmap(
                    result.width() / 2 - imgPixmap.width() / 2,
                    result.height() / 2 - imgPixmap.height() / 2,
                    imgPixmap
            );

            auto tempImage = new QTemporaryFile;
            tempImage->open();
            result.toImage().save(tempImage, QString::fromStdString(
                    image->getExtension()).toUpper().toStdString().c_str());
            tempImage->close();

            path = tempImage->fileName().toStdString();
        }

        if (config.shouldResizeOutputImage()) {
            auto targetSize = config.getResizeOutputToSize().value();
            QImage outputImg(image->getPath().c_str());

            auto width = outputImg.width();
            auto height = outputImg.height();

            if (!whatsappResize || width > 320 || height > 320) {
                if (whatsappResize) {
                    targetSize.width = 300;
                    targetSize.height = 300;
                }

                if (targetSize.width > 0 && width > targetSize.width) {
                    auto scale = (double) targetSize.width / width;
                    int new_height = height * scale;
                    outputImg = outputImg.scaledToHeight(new_height, Qt::SmoothTransformation);

                    height = new_height;
                    width = width * scale;
                }

                if (targetSize.height > 0 && height > targetSize.height) {
                    auto scale = (double) targetSize.height / height;
                    int new_width = width * scale;
                    outputImg = outputImg.scaledToWidth(new_width, Qt::SmoothTransformation);                    
                }

                auto tempImage = new QTemporaryFile;
                tempImage->open();
                outputImg.save(tempImage,
                           QString::fromStdString(image->getExtension()).toUpper().toStdString().c_str());
                tempImage->close();

                path = tempImage->fileName().toStdString();
            }
        }

#ifdef WIN32
        auto img = QImage(QString(path.c_str()));
        auto mirrored = img.mirrored(false, true);
        
        OpenClipboard(nullptr);
        HBITMAP hBitmap = qt_pixmapToWinHBITMAP(QPixmap::fromImage(mirrored), 1);
        DIBSECTION ds;
        ::GetObject(hBitmap, sizeof(DIBSECTION), &ds);
        //make sure compression is BI_RGB
        ds.dsBmih.biCompression = BI_RGB;
        HDC hdc = ::GetDC(NULL);
        HBITMAP hbitmap_ddb = ::CreateDIBitmap(
            hdc, &ds.dsBmih, CBM_INIT, ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
        ::ReleaseDC(NULL, hdc);

        EmptyClipboard();
        SetClipboardData(CF_BITMAP, hbitmap_ddb);
        CloseClipboard();
        ::DeleteObject(hBitmap);
#else
        auto ext = image->getExtension();
        ext = ext == "jpg" ? "jpeg" : ext;

        // TODO: handle 's in filenames
        std::string command = "cat '" + path + "' | xclip -selection clipboard -target image/" + ext + " -i";
        system(command.c_str());
#endif
    }

    if (config.shouldResizeOutputImage()) {
        QFile file(QString::fromStdString(path));
        file.remove();
    }

    this->imagePickerDrawer->clearFilter();
    this->imagePickerDrawer->move(ImagePickerMove::HOME);

    emit exitInstructionReceived();
    emit imageCopied();
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
        emit noImagesToDisplay();
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

void MainWindow::mousePressEvent (QMouseEvent *event) {
    QWidget::mousePressEvent(event);

    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (scrollingEndTimer.isActive()) {
        return;
    }

    auto mousePos = event->localPos();
    auto imageToCopy = imagePickerDrawer->getImageAtPos(mousePos.x(), mousePos.y());

    if (imageToCopy != nullptr) {
        copyImage(imageToCopy);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);

    if (scrollingEndTimer.isActive()) {
        return;
    }

    auto mousePos = event->localPos();
    auto imageUnderCursor = imagePickerDrawer->getImageAtPos(mousePos.x(), mousePos.y());

    if (imageUnderCursor != nullptr) {
        if (imageUnderCursor != imagePickerDrawer->getSelectedImage()) {
            imagePickerDrawer->drawFrame(imageUnderCursor);
            repaint();
        }

        setCursor(Qt::PointingHandCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void MainWindow::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);
    event->accept();
        
    setCursor(Qt::SizeVerCursor);
    scrollingEndTimer.start((int)scrollEndMilis);

    auto numDegrees = event->angleDelta() / 8;

    xScrollAccumulator += numDegrees.x();
    yScrollAccumulator += numDegrees.y();
    
    bool moved = false;

    if (abs(yScrollAccumulator) >= scrollTreshold) {
        auto direction = yScrollAccumulator < 0 ? ImagePickerMove::DOWN : ImagePickerMove::UP;
        moved = this->imagePickerDrawer->move(direction);
        yScrollAccumulator = 0;
    }

    if (abs(xScrollAccumulator) >= scrollTreshold) {
        auto direction = xScrollAccumulator < 0 ? ImagePickerMove::RIGHT : ImagePickerMove::LEFT;
        if(!moved) {
            moved = this->imagePickerDrawer->move(direction);
        }
        xScrollAccumulator = 0;
    }

    if (moved) {
        this->repaint();
    }
}

void MainWindow::scrollEnd() {
    yScrollAccumulator = 0;
    xScrollAccumulator = 0;

    QPoint mousePos = mapFromGlobal(QCursor::pos());
    QMouseEvent fakeEvent(QEvent::MouseMove, mousePos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    qApp->sendEvent(this, &fakeEvent);
}

MainWindow::MainWindow() : QWidget() {
    setWindowTitle("EMOJIGUN");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint);

    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    auto config = ConfigManager::getOrLoadConfig();
    auto geo = config.getScreenGeometry();
    screenBuffer = QPixmap(geo.width(), geo.height());

    this->imagePickerDrawer = new ImagePickerDrawer(screenBuffer);
    this->imagePickerDrawer->drawFrame(this->imagePickerDrawer->getSelectedImage(), true);

    setMouseTracking(true);

    scrollingEndTimer.setSingleShot(true);
    scrollingEndTimer.setInterval(scrollEndMilis);

    connect(
        &scrollingEndTimer, &QTimer::timeout,
        this, &MainWindow::scrollEnd
    );
}
