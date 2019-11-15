#pragma once

#include "Config.h"

struct CLIParams {
    unsigned int cacheSize;
    std::vector<std::string> imageFiles;
    bool startInVimMode = false;

    std::optional<unsigned int> maxImageWidth;
    std::optional<unsigned int> maxImageHeight;

    std::optional<unsigned int> rows;
    std::optional<unsigned int> cols;

    bool printFilePath = false;

    std::optional<unsigned int> imageXMargin;
    std::optional<unsigned int> imageYMargin;

    std::optional<unsigned int> imageXPadding;
    std::optional<unsigned int> imageYPadding;

    std::optional<unsigned int> width;
    std::optional<unsigned int> height;
};

class ConfigManager {
public:
    static Config getOrLoadConfig();
    ConfigManager();
    static void setCLIParams(CLIParams params);
private:
    inline static Config* config;
    inline static CLIParams cliParams;
    inline static bool configLoaded;
    inline static void loadConfig();
};