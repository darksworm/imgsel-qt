#include <QApplication>
#include <QScreen>
#include "util/lib/CLI11.hpp"
#include "gui/MainWindow.cpp"
#include "util/config/ConfigManager.h"
#ifdef WITH_X11
#include <QtX11Extras/QX11Info>
#include <tkPort.h>
#endif

int main(int argc, char *argv[]) {
    CLIParams params;
    CLI::App cli_app{"IMGSEL - Image selection tool."};

    cli_app.add_option("--files", params.imageFiles, "List of images to display.")
            ->required()
            ->check(CLI::ExistingFile);

    cli_app.add_flag("--vim", params.startInVimMode, "Set the initial mode to VIM mode.");

    auto rowCountOption = cli_app.add_option("--rows", params.rows, "How many rows to display");
    auto colCountOption = cli_app.add_option("--cols", params.cols, "How many cols to display");

    rowCountOption->needs(colCountOption)->required();
    colCountOption->needs(rowCountOption);

    auto widthOption = cli_app.add_option("--max-width", params.maxImageWidth,
                                          "The max image width. Any images larger than this will be scaled to this width");
    auto heightOption = cli_app.add_option("--max-height", params.maxImageHeight,
                                           "The max image height. Any images larger than this will be scaled to this height");

    auto xPadding = cli_app.add_option("--x-padding", params.imageXPadding,
                                       "Padding between image and selection box in pixels on the x axis");
    auto yPadding = cli_app.add_option("--y-padding", params.imageYPadding,
                                       "Padding between image and selection box in pixels on the y axis");

    xPadding->needs(yPadding);
    yPadding->needs(xPadding);

    auto xMargin = cli_app.add_option("--x-margin", params.imageXMargin,
                                      "Margin between images in pixels on the x axis");
    auto yMargin = cli_app.add_option("--y-margin", params.imageYMargin,
                                      "Margin between images in pixels on the y axis");

    auto width = cli_app.add_option("--window-width", params.width,
                                    "Window width");
    auto height = cli_app.add_option("--window-height", params.height,
                                     "Window height");

    width->needs(height);
    height->needs(width);

    xMargin->needs(yMargin);
    yMargin->needs(xMargin);

    cli_app.add_flag("--print-path", params.printFilePath,
                     "Write file path to stdout instead of copying it's contents to the clipboard.");

    heightOption->needs(widthOption)->required();
    widthOption->needs(heightOption);

    CLI11_PARSE(cli_app, argc, argv);
    ConfigManager::setCLIParams(params);

    QApplication app(argc, argv);
    auto config = ConfigManager::getOrLoadConfig();

    MainWindow window;
    window.setWindowTitle(QApplication::translate("APPLICATION", "IMGSEL-QT"));
    window.setWindowFlags(window.windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint | Qt::Dialog);
    window.setAttribute(Qt::WA_NoSystemBackground, true);
    window.setAttribute(Qt::WA_TranslucentBackground, true);
    window.setGeometry(config.getScreenGeometry());
    window.show();
    window.raise();

#ifdef WITH_X11
    XMoveWindow(QX11Info::display(), window.winId(), 0, 0);
#endif

    window.setFocus();

    return app.exec();
}
