#include <QScreen>
#include <QHotkey>
#include "gui/SettingsWindow.h"
#include "gui/MainWindow.h"
#include "util/lib/CLI11.hpp"
#include "util/config/ConfigManager.h"
#include "app/Application.h"
#include "util/validators/IntXIntValidator.h"
#include "util/validators/DirectoriesContainImages.h"
#include <project_config.h>
#include <iostream>
#include "util/FileDownloader.h"
#include "assets/assets.h"

#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif

#ifdef WIN32
#include <QtPlugin>
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

#ifdef __linux__
#include "util/linuxutils.h"
#endif

int main(int argc, char *argv[]) {
    bool oneShotMode = argc > 1;

    Application app(argc, argv, oneShotMode);
    if (!app.lock()) {
        std::cout << "Another instance of the app is already running!";
        return -42;
    }

    bool startMinimized = app.getSettings().value("start_minimized", false).toInt();

    QSplashScreen* splashScreen = nullptr;

    if (!oneShotMode && !startMinimized) {
        QPixmap splashScreenImage(ASSET_SPLASH_IMAGE);
        splashScreen = new QSplashScreen(splashScreenImage);
        splashScreen->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint);
        splashScreen->show();
        app.processEvents();
    }

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
cli_app.add_option("--margin", params.margin, "Margin between image selection boxes in pixels. {horizontal}x{vertical}")
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

#ifdef __linux__
    catchUnixExitSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});
#endif

    auto window = new MainWindow();
    app.setMainWindow(window);

#ifdef WIN32
    emojigunUpdater.setPathToExecutable(argv[0]);
#endif

    if (oneShotMode) {
        window->display();
    } else {
        Q_INIT_RESOURCE(qtres);

        auto settingsWindow = new SettingsWindow(window);

        if (!startMinimized) {
            if (splashScreen != nullptr) {
                splashScreen->finish(settingsWindow);
            }

            settingsWindow->show();
        }

        bool checkForUpdates = app.getSettings().value("check_for_updates_on_launch", true).toInt();
        if (checkForUpdates) {
            emojigunUpdater.checkForUpdates();
        }

        app.setSettingsWindow(settingsWindow);

        auto hkSequence = app.getSettings().value("hotkey_sequence", "ctrl+shift+x").toString();
        app.hotkeyBindingChange(hkSequence);
    }

    return app.exec();
}
