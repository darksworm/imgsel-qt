#include <QApplication>
#include <QScreen>
#include "lib/CLI11.hpp"
#include "gui/MainWindow.cpp"
#include "config/ConfigManager.h"

int initConfig(int argc, char **argv) {
    CLI::App cli_app{"IMGSEL - Image selection tool."};

    CLIParams params = CLIParams();

    cli_app.add_option("--files", params.imageFiles, "List of images to display.")
            ->required()
            ->check(CLI::ExistingFile);

    cli_app.add_option("--cache-size", params.cacheSize,
                       "How many (max) bytes of memory to use for caching loaded images.",
                       1024 * 1024 * 100);

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

    auto width = cli_app.add_option("--width", params.width,
                                    "Screen width");
    auto height = cli_app.add_option("--height", params.height,
                                     "Screen height");

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

    return 0;
}

int main(int argc, char *argv[]) {
    int configInitExitCode = initConfig(argc, argv);

    if (configInitExitCode) {
        return configInitExitCode;
    }

    QApplication app(argc, argv);
    auto config = ConfigManager::getOrLoadConfig();

    MainWindow window;

    window.setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::WindowStaysOnTopHint | Qt::Dialog);
    window.setParent(nullptr);
    window.setAttribute(Qt::WA_NoSystemBackground, true);
    window.setAttribute(Qt::WA_TranslucentBackground, true);

    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());

    if (config.getHeight().has_value()) {
        window.setGeometry(0, 0, config.getWidth().value(), config.getHeight().value());
    } else {
        window.setGeometry(screen->geometry());
    }
    window.show();
    window.raise();

    window.setWindowTitle(QApplication::translate("APPLICATION", "IMGSEL-QT"));
    window.activateWindow();
    window.move(screen->geometry().x(), screen->geometry().y());
    window.setFocus();

    return app.exec();
}
