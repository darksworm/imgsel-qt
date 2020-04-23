#include <QScreen>
#include <QHotkey>
#include "gui/SettingsWindow.h"
#include "gui/MainWindow.h"
#include "util/lib/CLI11.hpp"
#include "util/config/ConfigManager.h"
#include "Application.h"
#include "util/validators/IntXIntValidator.h"
#include "util/validators/DirectoriesContainImages.h"
#include <project_config.h>
#include <iostream>
#include "util/FileDownloader.h"

#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif

#ifdef WIN32
#include <QtPlugin>
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

#define emojigunApp ((Application*) qApp)

int main(int argc, char *argv[]) {
    bool oneShotMode = argc > 1;

    if (oneShotMode) {
        CLIParams params;
        CLI::App cli_app{"EMOJIGUN - emoji sharing tool."};

        cli_app.get_option("--help")
                ->group("Meta");
        cli_app.add_flag("-v,--version", [](auto in) {
                    std::cout << "EMOJIGUN v" << PROJECT_VER << "\n";
                    exit(0);
                }, "Show application version")
                ->group("Meta");

        const static IntXIntValidator intXIntValidator;
        const static DirectoriesContainImages directoriesContainImages;

        cli_app.add_option("--files", params.imageFiles, "List of image files or image directories to display.")
                ->required()
                ->check(CLI::ExistingPath)
                ->check(directoriesContainImages);

        cli_app.add_flag("--vim", params.startInVimMode, "Set the initial mode to VIM mode.");

        cli_app.add_option("--grid-size", params.rowsAndCols,
                           "How many images to display on screen {columns}x{rows}")
                ->check(intXIntValidator)
                ->group("Visual");

        cli_app.add_option("--window-size", params.size,
                           "EMOJIGUN window size {width}x{height}")
                ->check(intXIntValidator)
                ->group("Visual");

        cli_app.add_option("--padding", params.padding,
                           "Padding between image and selection box in pixels. {horizontal}x{vertical}")
                ->check(intXIntValidator)
                ->group("Visual");

        cli_app.add_option("--margin", params.margin,
                           "Margin between image selection boxes in pixels. {horizontal}x{vertical}")
                ->check(intXIntValidator)
                ->group("Visual");

        cli_app.add_option("--image-size", params.maxImageSize,
                           "The max image width. Any images larger than this will be scaled to this size. {width}x{height}")
                ->check(intXIntValidator)
                ->group("Visual");

        auto resizeOpt = cli_app.add_option("--resize-output-image", params.resizeToSize,
                                            "Resize image to specific dimensions before copying {width}x{height}")
                ->check(intXIntValidator)
                ->group("Output");

        auto printPathOpt = cli_app.add_flag("--print-path", params.printFilePath,
                                             "Write file path to stdout instead of copying it's contents to the clipboard.")
                ->group("Output");

        resizeOpt->excludes(printPathOpt);

        CLI11_PARSE(cli_app, argc, argv)

        ConfigManager::setCLIParams(params);
    }

    Application app(argc, argv, oneShotMode);
    if (!app.lock()) {
        std::cout << "Another instance of the app is already running!";
        return -42;
    }

    auto window = new MainWindow();
    app.setMainWindow(window);

#ifdef WIN32
    app.setPathToExecutable(argv[0]);
#endif

    if (oneShotMode) {
        window->display();
    } else {
        Q_INIT_RESOURCE(qtres);

        auto settingsWindow = new SettingsWindow(window);

        bool startMinimized = app.getSettings().value("start_minimized", false).toInt();
        if (!startMinimized) {
            settingsWindow->show();
        }

        bool checkForUpdates = app.getSettings().value("check_for_updates_on_launch", true).toInt();
        if (checkForUpdates) {
            app.checkForUpdates();
        }

        app.setSettingsWindow(settingsWindow);

#ifdef WIN32
        bool launchOnStartup = app.getSettings().value("launch_on_startup", false).toBool();
        if (launchOnStartup) {
            app.checkSavedExeVersion();
        }
#endif

        auto hkSequence = app.getSettings().value("hotkey_sequence", "ctrl+shift+x").toString();
        app.hotkeyBindingChange(hkSequence);
    }

    return app.exec();
}
