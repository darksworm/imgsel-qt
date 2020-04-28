#pragma once

#include <optional>
#include "Config.h"

struct CLIParams {
    std::vector<std::string> imageFiles;
    bool startInVimMode = false;
    bool printFilePath = false;

    std::optional<std::string> resizeToSize = std::nullopt;
    std::optional<std::string> maxImageSize = std::nullopt;
    std::optional<std::string> rowsAndCols = std::nullopt;
    std::optional<std::string> margin = std::nullopt;
    std::optional<std::string> padding = std::nullopt;
    std::optional<std::string> size = std::nullopt;
};

class ConfigManager {
friend class SettingsWindow;
public:
    ConfigManager();
    static Config& getOrLoadConfig();
    static bool invalidateConfigIfImageListChanged();
    static void setCLIParams(CLIParams params);

private:
    inline static Config *config;
    inline static CLIParams cliParams;
    inline static bool configLoaded;

    inline static void loadConfig();
    static std::vector<std::string> getImagePaths();
    static void applyConfigFromQSettings();
};
