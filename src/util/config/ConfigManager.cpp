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

    std::vector<Image> images;

    bool autoMode = !cliParams.rowsAndCols.has_value();
    auto imageFilePaths = getImagePaths();

    for (auto &path : imageFilePaths) {
        images.emplace_back(path);
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

    std::vector<int> maxImageSize;
    std::vector<int> padding;
    std::vector<int> margin;
    std::vector<int> rowsAndCols;

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
            widths.insert(128);
            heights.insert(128);
        }

        // get middle elements
        auto widthsIterator = widths.begin();
        std::advance(widthsIterator, widths.size() / 2);

        auto heightsIterator = heights.begin();
        std::advance(heightsIterator, heights.size() / 2);

        auto maxWidth = *widthsIterator;
        auto maxHeight = *heightsIterator;

        if (maxWidth > 64 || maxHeight > 64) { 
            maxWidth = 64;
            maxHeight = 64;
        }

        // because we want medians
        maxImageSize = { (int)maxWidth, (int)maxHeight };

        unsigned xEmptySpace = maxImageSize.at(0) / 2.5;
        unsigned yEmptySpace = maxImageSize.at(1) / 2.5;

        padding = { (int)xEmptySpace, (int) yEmptySpace };
        margin = padding;

        // we don't want to take up the whole screen
        double screenUsageModifier = 0.85;

        unsigned maxImagesInHeight = geo.height() /
                                     (maxImageSize.at(1) + yEmptySpace * 3) * screenUsageModifier;

        unsigned maxImagesInWidth = geo.width() /
                                    (maxImageSize.at(0) + xEmptySpace * 3) * screenUsageModifier;

        rowsAndCols = { (int) maxImagesInWidth, (int) maxImagesInHeight };
    } else {
        rowsAndCols = StringTools::splitIntoInts(cliParams.rowsAndCols.value(), "x");
        maxImageSize = StringTools::splitIntoInts(cliParams.maxImageSize.value(), "x");
        padding = StringTools::splitIntoInts(cliParams.padding.value(), "x");
        margin = StringTools::splitIntoInts(cliParams.margin.value(), "x");
    }

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

std::vector<std::string> ConfigManager::getImagePaths() {
    std::vector<std::string> imageExtensions = Config::getImageExtensions();

    QStringList allowedExtensions;
    for (const auto &ext : imageExtensions) {
        allowedExtensions << "*." + QString::fromStdString(ext);
    }

    bool oneShotMode = ((Application *) qApp)->isOneShotMode();
    auto imageFilePaths = cliParams.imageFiles;

    std::vector<std::string> images;

    if (!oneShotMode) {
        QSettings settings("EMOJIGUN", "EMOJIGUN");

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

    return images;
}

Config ConfigManager::getOrLoadConfig() {
    bool oneShotMode = ((Application *) qApp)->isOneShotMode();

    if (!ConfigManager::configLoaded) {
        loadConfig();
    }

    if (!oneShotMode) {
       applyConfigFromQSettings();
    }

    return *ConfigManager::config;
}

void ConfigManager::applyConfigFromQSettings(){
    QSettings settings("EMOJIGUN", "EMOJIGUN");
    auto resizeSettingEnabled = settings.value("resize_output_image", true).toBool();

    if (resizeSettingEnabled) {
        auto resizeWidth = settings.value("resize_output_image_width", 32).toInt();
        auto resizeHeight = settings.value("resize_output_image_height", 32).toInt();

        config->resizeOutputToSize = {
                .width = (unsigned) resizeWidth,
                .height = (unsigned) resizeHeight
        };
    } else {
        config->resizeOutputToSize.reset();
    }
}

void ConfigManager::setCLIParams(CLIParams params) {
    ConfigManager::cliParams = std::move(params);
}

bool ConfigManager::invalidateConfigIfImageListChanged() {
    // return bool - whether image list has changed
    if (!ConfigManager::configLoaded) {
        return false;
    }

    auto oldImages = ConfigManager::config->getImages();
    std::vector<std::string> oldImagePaths;

    for (auto &img : oldImages) {
        oldImagePaths.emplace_back(img.getPath());
    }

    auto newImagePaths = getImagePaths();

    if (oldImagePaths == newImagePaths) {
        return false;
    }

    delete ConfigManager::config;
    ConfigManager::configLoaded = false;

    return true;
}
