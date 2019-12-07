#pragma once

#include "Config.h"

struct CLIParams {
    std::vector<std::string> imageFiles;
    bool startInVimMode = false;
    bool printFilePath = false;

    std::optional<std::string> resizeToSize;
    std::optional<std::string> maxImageSize;
    std::optional<std::string> rowsAndCols;
    std::optional<std::string> margin;
    std::optional<std::string> padding;
    std::optional<std::string> size;
};

class ConfigManager {
public:
    static Config getOrLoadConfig();

    ConfigManager();

    static void setCLIParams(CLIParams params);

private:
    inline static Config *config;
    inline static CLIParams cliParams;
    inline static bool configLoaded;

    inline static void loadConfig();
};