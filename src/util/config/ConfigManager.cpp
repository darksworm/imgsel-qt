#define DEBUG false

#include <QtGui/QScreen>
#include <QtGui/QGuiApplication>
#include <QCursor>
#include <QtCore/QDir>
#include <QtGui/QImage>
#include "ConfigManager.h"
#include "ConfigBuilder.h"
#include "../StringTools.h"
#include <set>
#include <cmath>

void ConfigManager::loadConfig() {
    std::vector<std::string> imageExtensions = Config::getImageExtensions();
    std::vector<Image> images;

    bool autoMode = !cliParams.rows.has_value();

    QStringList allowedExtensions;
    for (const auto &ext : Config::getImageExtensions()) {
        allowedExtensions << "*." + QString::fromStdString(ext);
    }

    for (auto &path : cliParams.imageFiles) {
        QDir dir(QString::fromStdString(path));

        if (dir.exists()) {
            if (dir.isEmpty()) {
                continue;
            }

            QStringList dirImages = dir.entryList(allowedExtensions);

            if (dirImages.isEmpty()) {
                continue;
            }

            for (const auto& img: dirImages) {
                images.emplace_back(dir.absoluteFilePath(img).toStdString());
            }
        } else {
            for (const auto &ext:imageExtensions) {
                if (path.length() >= ext.length() && 0 == path.compare(path.length() - ext.length(), ext.length(), ext)) {
                    images.emplace_back(path);
                    break;
                }
            }
        }
    }

    QScreen *screen = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
    screen = QGuiApplication::screenAt(QCursor::pos());
#else
    auto screens = QGuiApplication::screens();
    auto cursorPos = QCursor::pos();
    for (const auto &scn:screens) {
        if (scn->geometry().contains(cursorPos)) {
            screen = scn;
            break;
        }
    }
#endif

    auto builder = ConfigBuilder();

    QRect geo;

    if (cliParams.width.has_value()) {
        builder = builder.setWidth(cliParams.width.value())
                .setHeight(cliParams.height.value());

        geo = QRect(0, 0, cliParams.width.value(), cliParams.height.value());
    } else {
        geo = screen->geometry();
    }

    if (autoMode) {
        std::multiset<unsigned> widths;
        std::multiset<unsigned> heights;

        QImage image;

        for (auto &img:images) {
            image.load(QString::fromStdString(img.getPath()));

            widths.insert(image.width());
            heights.insert(image.height());

            image.detach();
        }

        // get middle elements
        auto widthsIterator = widths.begin();
        std::advance(widthsIterator, widths.size() / 2);

        auto heightsIterator = heights.begin();
        std::advance(heightsIterator, heights.size() / 2);

        // because we want medians
        cliParams.maxImageWidth = *widthsIterator;
        cliParams.maxImageHeight = *heightsIterator;

        cliParams.imageXPadding = cliParams.maxImageWidth.value() / 2.5;
        cliParams.imageYPadding = cliParams.maxImageWidth.value() / 2.5;

        cliParams.imageXMargin = cliParams.maxImageWidth.value() / 2.5;
        cliParams.imageYMargin = cliParams.maxImageWidth.value() / 2.5;

        // we don't want to take up the whole screen
        double screenUsageModifier = 0.9;

        unsigned maxImagesInHeight = geo.height() /
                                     (cliParams.maxImageHeight.value() + cliParams.imageYPadding.value() * 2 +
                                      cliParams.imageYMargin.value()) * screenUsageModifier;

        unsigned maxImagesInWidth = geo.width() /
                                    (cliParams.maxImageWidth.value() + cliParams.imageXPadding.value() * 2 +
                                     cliParams.imageXPadding.value()) * screenUsageModifier;

        cliParams.rows = maxImagesInHeight;
        cliParams.cols = maxImagesInWidth;
    }

    builder.setIsDebug(DEBUG)
            .setDefaultInputMode(cliParams.startInVimMode ? InputMode::VIM : InputMode::DEFAULT)

            .setCols(cliParams.cols.has_value() ? cliParams.cols.value() : 0)
            .setRows(cliParams.rows.has_value() ? cliParams.rows.value() : 0)

            .setMaxImageHeight(cliParams.maxImageHeight.has_value() ? cliParams.maxImageHeight.value() : 0)
            .setMaxImageWidth(cliParams.maxImageWidth.has_value() ? cliParams.maxImageWidth.value() : 0)

            .setPrintFilePath(cliParams.printFilePath)

            .setYPadding(cliParams.imageYPadding.has_value() ? cliParams.imageYPadding.value() : 30)
            .setXPadding(cliParams.imageXPadding.has_value() ? cliParams.imageXPadding.value() : 30)

            .setYMargin(cliParams.imageYMargin.has_value() ? cliParams.imageYMargin.value() : 30)
            .setXMargin(cliParams.imageXMargin.has_value() ? cliParams.imageXMargin.value() : 30)

            .setScreenGeometry(geo)

            .setImages(images);

    if (cliParams.resizeToSize.has_value()) {
        auto sizes = StringTools::splitIntoInts(cliParams.resizeToSize.value(), "x");

        builder.setResizeOutputToSize(Size{
                .width = (unsigned) sizes.at(0),
                .height = (unsigned) sizes.at(1)
        });
    }

    ConfigManager::config = builder.build();
    ConfigManager::configLoaded = true;
}

Config ConfigManager::getOrLoadConfig() {
    if (!ConfigManager::configLoaded) {
        loadConfig();
    }

    return *ConfigManager::config;
}

ConfigManager::ConfigManager() {
    ConfigManager::configLoaded = false;
}

void ConfigManager::setCLIParams(CLIParams params) {
    ConfigManager::cliParams = std::move(params);
}
