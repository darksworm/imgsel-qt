#include <QScreen>
#include "util/lib/CLI11.hpp"
#include "gui/MainWindow.cpp"
#include "util/config/ConfigManager.h"
#include "Application.h"
#include "util/validators/IntXIntValidator.h"
#include "util/validators/DirectoriesContainImages.h"
#include "gui/SettingsWindow.h"
#include <project_config.h>

#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif

#include <iostream>
#include <libnet.h>

#ifdef WIN32
#include <QtPlugin>
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif

void moveWindow(WId id) {
    // this is outside because the app won't compile if xlib is
    // explicitly included in any other file.
#ifdef WITH_X11
    XMoveWindow(QX11Info::display(), id, 0, 0);
#endif
}

int main(int argc, char *argv[]) {
    CLIParams params;
    CLI::App cli_app{"IMGSEL - Image selection tool."};

    cli_app.get_option("--help")
            ->group("Meta");
    cli_app.add_flag("-v,--version", [](auto in) {
                std::cout << "IMGSEL v" << PROJECT_VER << "\n";
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
                       "IMGSEL window size {width}x{height}")
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

    Application app(argc, argv);

    if (!app.lock()) {
        std::cout << "Another instance of the app is already running!";
        return -42;
    }

    ConfigManager::setCLIParams(params);

    auto window = new MainWindow(moveWindow);

    window->setWindowTitle(QApplication::translate("APPLICATION", "IMGSEL-QT"));
    window->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint);

    window->setAttribute(Qt::WA_NoSystemBackground, true);
    window->setAttribute(Qt::WA_TranslucentBackground, true);
    window->setGeometry(ConfigManager::getOrLoadConfig().getScreenGeometry());

    SettingsWindow settingsWindow(window);
    settingsWindow.show();

    return app.exec();
}
