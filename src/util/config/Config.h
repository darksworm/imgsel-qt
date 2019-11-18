#pragma once

#include <vector>
#include "../../input/InputMode.h"
#include "../image.h"

class Config {
    friend class ConfigBuilder;

private:
    bool isDebug;
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

    std::optional<unsigned int> width;
    std::optional<unsigned int> height;

    QRect screenGeometry;

    std::vector<Image> images;

    Config() = default;

public:
    bool isIsDebug() const {
        return isDebug;
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

    QRect getScreenGeometry() const {
        return screenGeometry;
    };


    std::optional<unsigned int> getWidth() const {
        return width;
    }

    std::optional<unsigned int> getHeight() const {
        return height;
    }
};