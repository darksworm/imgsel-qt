#include "DragDropLayout.h"
#include "../util/config/Config.h"

DragDropLayout::DragDropLayout(QWidget* widget) : QVBoxLayout(widget) {
    textLabel = new QLabel();
    textLabel->setMaximumWidth(widget->width());
    textLabel->setAlignment(Qt::AlignCenter);

    this->addWidget(textLabel, Qt::AlignCenter);

    expireTimer.setSingleShot(true);
    connect(&expireTimer, &QTimer::timeout, this, [&](){ emit expired(); reset(); });

    reset();
}

void DragDropLayout::reset() {
    textLabel->setText("Drop images and archives to import into library");

    expireTimer.stop();
    expireTimer.setInterval(2000);
}

void DragDropLayout::importStarted() {
    textLabel->setText("Importing images...");
    expireTimer.start();
}

void DragDropLayout::importFinished() {
    textLabel->setText("Images imported!");
    expireTimer.start();
}

void DragDropLayout::importFailed() {
    textLabel->setText("Import failed!");
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

    // TODO this text enlarges the window
    // TODO fix wording
    // TODO this text should dissapear when mouse leaves window instead of on
    // timer
    textLabel->setText(QString("emojigun supports only these file types: ") + QString::fromStdString(extensions));
    expireTimer.start();
}
