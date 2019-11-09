#pragma once

#include <vector>
#include "../input/handler/InputMode.h"
#include "../image/image.h"

class Config {
    friend class ConfigBuilder;

private:
    bool isDebug;
    unsigned int imageCacheSizeBytes;
    InputMode defaultInputMode = InputMode::DEFAULT;

    unsigned int maxImageWidth = 0;
    unsigned int maxImageHeight = 0;

    unsigned int rows = 0;
    unsigned int cols = 0;

    bool printFilePath = false;

    unsigned int yPadding = 40;
    unsigned int xPadding = 40;

    unsigned int xMargin = 40;
    unsigned int yMargin = 20;

    std::vector<Image> images;

    Config() = default;

public:
    bool isIsDebug() const {
        return isDebug;
    }

    unsigned int getImageCacheSizeBytes() {
        return imageCacheSizeBytes;
    }

    InputMode getDefaultInputMode() {
        return defaultInputMode;
    }

    unsigned int getMaxImageWidth() const {
        return maxImageWidth;
    }

    unsigned int getMaxImageHeight() const {
        return maxImageHeight;
    }

    unsigned int getRows() const {
        return rows;
    }

    unsigned int getCols() const {
        return cols;
    }

    bool shouldPrintFilePath() const {
        return printFilePath;
    }

    unsigned int getYPadding() const {
        return yPadding;
    }

    unsigned int getXPadding() const {
        return xPadding;
    }

    unsigned int getYMargin() const {
        return yMargin;
    }

    unsigned int getXMargin() const {
        return xMargin;
    }

    std::vector<Image> getImages() const {
        return images;
    }
};