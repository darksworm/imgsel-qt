#define DEBUG false

#include <QtGui/QScreen>
#include <QtGui/QGuiApplication>
#include <QCursor>
#include <QtCore/QDir>
#include <QtGui/QImage>
#include "ConfigManager.h"
#include "ConfigBuilder.h"
#include "../StringTools.h"
#include "../../input/handler/instruction/PreprocessorFlags.h"
#include <set>
#include <cmath>
#include <QtCore/QProcess>
#include "../../Application.h"

void ConfigManager::loadConfig() {
    QSettings settings("EMOJIGUN", "EMOJIGUN");

    std::vector<std::string> imageExtensions = Config::getImageExtensions();
    std::vector<Image> images;

    bool autoMode = !cliParams.rowsAndCols.has_value();
    bool oneShotMode = ((Application *) qApp)->isOneShotMode();

    QStringList allowedExtensions;
    for (const auto &ext : Config::getImageExtensions()) {
        allowedExtensions << "*." + QString::fromStdString(ext);
    }

    auto imageFilePaths = cliParams.imageFiles;

    if (!oneShotMode) {
        auto defaultDir = Application::defaultLibraryDirectory();
        auto imageDirFromSettings = settings.value("library_path", defaultDir).toString();

        if (imageDirFromSettings == defaultDir) {
            QDir().mkdir(imageDirFromSettings);
        }

        imageFilePaths.emplace_back(imageDirFromSettings.toStdString());
    }

    for (auto &path : imageFilePaths) {
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

    if (cliParams.size.has_value()) {
        auto sizes = StringTools::splitIntoInts(cliParams.size.value(), "x");

        builder = builder.setWidth(sizes.at(0))
                .setHeight(sizes.at(1));

        geo = QRect(0, 0, sizes.at(0), sizes.at(1));
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

        if (images.empty()) {
            widths.insert(50);
            heights.insert(50);
        }

        // get middle elements
        auto widthsIterator = widths.begin();
        std::advance(widthsIterator, widths.size() / 2);

        auto heightsIterator = heights.begin();
        std::advance(heightsIterator, heights.size() / 2);

        // because we want medians
        cliParams.maxImageSize = std::to_string((unsigned)*widthsIterator) + "x" + std::to_string((unsigned)*heightsIterator);

        auto imageSizes = StringTools::splitIntoInts(cliParams.maxImageSize.value(), "x");

        unsigned xEmptySpace = imageSizes.at(0) / 2.5;
        unsigned yEmptySpace = imageSizes.at(1) / 2.5;

        cliParams.padding = std::to_string(xEmptySpace) + "x" + std::to_string(yEmptySpace);
        cliParams.margin = *cliParams.padding;

        // we don't want to take up the whole screen
        double screenUsageModifier = 0.9;

        unsigned maxImagesInHeight = geo.height() /
                                     (imageSizes.at(1) + yEmptySpace * 3) * screenUsageModifier;

        unsigned maxImagesInWidth = geo.width() /
                                    (imageSizes.at(0) + xEmptySpace * 3) * screenUsageModifier;

        cliParams.rowsAndCols = std::to_string(maxImagesInWidth) + "x" + std::to_string(maxImagesInHeight);
    }

    auto rowsAndCols = StringTools::splitIntoInts(cliParams.rowsAndCols.value(), "x");
    auto maxImageSize = StringTools::splitIntoInts(cliParams.maxImageSize.value(), "x");
    auto padding = StringTools::splitIntoInts(cliParams.padding.value(), "x");
    auto margin = StringTools::splitIntoInts(cliParams.margin.value(), "x");

    builder.setIsDebug(DEBUG)
            .setDefaultInputMode(cliParams.startInVimMode ? InputMode::VIM : InputMode::DEFAULT)

            .setCols(rowsAndCols.at(0))
            .setRows(rowsAndCols.at(1))

            .setMaxImageWidth(maxImageSize.at(0))
            .setMaxImageHeight(maxImageSize.at(1))

            .setPrintFilePath(cliParams.printFilePath)

            .setXPadding(padding.at(0))
            .setYPadding(padding.at(1))

            .setXMargin(margin.at(0))
            .setYMargin(margin.at(1))

            .setScreenGeometry(geo)

            .setImages(images);

    if (cliParams.resizeToSize.has_value()) {
        auto sizes = StringTools::splitIntoInts(cliParams.resizeToSize.value(), "x");

        builder.setResizeOutputToSize(Size{
                .width = (unsigned) sizes.at(0),
                .height = (unsigned) sizes.at(1)
        });
    }

    if (!oneShotMode) {
        auto resizeSettingEnabled = settings.value("resize_output_image", false).toBool();
        if (resizeSettingEnabled) {
            auto resizeWidth = settings.value("resize_output_image_width", 32).toInt();
            auto resizeHeight = settings.value("resize_output_image_height", 32).toInt();

            builder.setResizeOutputToSize(Size{
                    .width = (unsigned) resizeWidth,
                    .height = (unsigned) resizeHeight
            });
        }
    }

#ifdef WITH_X11
    QProcess process;
    process.setProgram("xdotool");
    process.setArguments(QStringList() << "getwindowfocus" << "getwindowname");
    process.start();
    process.waitForFinished(-1);
    QString title = process.readAll();

    if (title.toLower().contains("whatsapp")) {
        builder.addPreprocessorFlag(PreprocessorFlags::WhatsAppWhitespace);
    }
#endif

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

void ConfigManager::invalidateConfig() {
    ConfigManager::configLoaded = false;
}
