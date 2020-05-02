#include "DragDropLayout.h"
#include "../util/config/Config.h"

DragDropLayout::DragDropLayout(QWidget* widget) : QVBoxLayout(widget) {
    textLabel = std::make_unique<QLabel>();
    textLabel->setMaximumWidth(widget->width());
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    this->addWidget(&*textLabel, Qt::AlignCenter);

    expireTimer.setSingleShot(true);
    connect(&expireTimer, &QTimer::timeout, this, [&](){ emit expired(); reset(); });

    reset();
}

void DragDropLayout::reset() {
    textLabel->setText("Drop images and archives here to import into library");

    expireTimer.stop();
    expireTimer.setInterval(2000);
}

void DragDropLayout::importStarted() {
    textLabel->setText("Importing emojis...");
    expireTimer.start();
}

void DragDropLayout::importFinished() {
    textLabel->setText("Emojis imported!");
    expireTimer.start();
}

void DragDropLayout::importFailed(QString errorMessage) {
    textLabel->setText("Import failed!<br><br>" + errorMessage);
    expireTimer.setInterval(4000);
    expireTimer.start();
}

void DragDropLayout::noSuitableFilesDropped() {
    auto supportedExtensions = Config::getImageExtensions();
    supportedExtensions.emplace_back("zip");

    std::string extensions = "";

    for (auto ext : supportedExtensions) {
        if (!extensions.empty()) {
            extensions += ", ";
        }

        extensions += ext;
    }

    textLabel->setText(QString("Unsupported file!<br>emojigun supports these file types:<br>") + QString::fromStdString(extensions));
    expireTimer.setInterval(4000);
    expireTimer.start();
}
