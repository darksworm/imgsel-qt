#include <thread>
#include "lib/CLI11/include/CLI/CLI.hpp"

#include <X11/extensions/XTest.h>
#include <Imlib2.h>

#include "gui/WindowManager.h"
#include "gui/drawer/ShapeDrawerFactory.h"

#include "config/ConfigManager.h"
#include "util/ThreadSafeQueue.h"
#include "input/XEventWrapper.h"

#include <vector>
#include <string>
#include <sys/file.h>
#include "input/InstructionRouter.h"
#include "input/XInputHandler.h"
#include "util/helpers.h"

int masin(int argc, char *argv[]) {
    int pid_file = open("/tmp/imgsel.pid", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if (rc && EWOULDBLOCK == errno) {
        std::cout << "another proccess is already running!";
        exit(1);
    }

    auto homeDir = getHomeDir();

    struct stat info;
    if (stat((homeDir + "/.imgsel/logs").c_str(), &info) != 0) {
        mkdir((homeDir + "/.imgsel").c_str(), S_IRWXU | S_IRGRP | S_IROTH);
        mkdir((homeDir + "/.imgsel/logs").c_str(), S_IRWXU | S_IRGRP | S_IROTH);
    }

    if (!XInitThreads()) {
        exit(1);
    }

    CLI::App app{"IMGSEL - Image selection tool."};

    CLIParams params = CLIParams();

    app.add_option("--files", params.imageFiles, "List of images to display.")
            ->required()
            ->check(CLI::ExistingFile);

    app.add_option("--cache-size", params.cacheSize, "How many (max) bytes of memory to use for caching loaded images.",
                   1024 * 1024 * 100);

    app.add_flag("--vim", params.startInVimMode, "Set the initial mode to VIM mode.");

    auto rowCountOption = app.add_option("--rows", params.rows, "How many rows to display");
    auto colCountOption = app.add_option("--cols", params.cols, "How many cols to display");

    rowCountOption->needs(colCountOption)->required();
    colCountOption->needs(rowCountOption);

    auto widthOption = app.add_option("--max-width", params.maxImageWidth,
                                      "The max image width. Any images larger than this will be scaled to this width");
    auto heightOption = app.add_option("--max-height", params.maxImageHeight,
                                       "The max image height. Any images larger than this will be scaled to this height");

    auto xPadding = app.add_option("--x-padding", params.imageXPadding, "Padding between image and selection box in pixels on the x axis");
    auto yPadding = app.add_option("--y-padding", params.imageYPadding, "Padding between image and selection box in pixels on the y axis");

    xPadding->needs(yPadding);
    yPadding->needs(xPadding);

    auto xMargin = app.add_option("--x-margin", params.imageXMargin, "Margin between images in pixels on the x axis");
    auto yMargin = app.add_option("--y-margin", params.imageYMargin, "Margin between images in pixels on the y axis");

    xMargin->needs(yMargin);
    yMargin->needs(xMargin);

    app.add_flag("--print-path", params.printFilePath,
                 "Write file path to stdout instead of copying it's contents to the clipboard.");

    heightOption->needs(widthOption)->required();
    widthOption->needs(heightOption);

    CLI11_PARSE(app, argc, argv);

    ConfigManager::setCLIParams(params);
    auto config = ConfigManager::getOrLoadConfig();

    std::vector<std::string> imageExtensions = {
            "jpg",
            "jpeg",
            "png",
            "gif"
    };

    std::vector<Image> images;
    for (const auto &img:params.imageFiles) {
        for (const auto &ext:imageExtensions) {
            if (img.length() >= ext.length() && 0 == img.compare(img.length() - ext.length(), ext.length(), ext)) {
                images.emplace_back(img);
                break;
            }
        }
    }

    // set up cache for images
    imlib_set_cache_size(config.getImageCacheSizeBytes());

    auto windowManager = new WindowManager();

    Display *display = windowManager->getDisplay();
    windowManager->getWindow();

    XKeyboardState initKBState;
    XGetKeyboardControl(display, &initKBState);

    ThreadSafeQueue<XEventWrapper> eventQueue;
    bool shouldQuit = false;

    std::thread xEventListener(handleXInput, windowManager, &eventQueue, &shouldQuit);
    std::thread xEventHandler(handleEvents, windowManager, &eventQueue, images, &shouldQuit);

    xEventListener.join();
    xEventHandler.join();

    XKeyboardState exitKBState;
    XGetKeyboardControl(display, &exitKBState);

    if (exitKBState.led_mask & 1 != initKBState.led_mask & 1) {
        auto capsKeycode = XKeysymToKeycode(display, XK_Caps_Lock);

        // Simulate Press
        XTestFakeKeyEvent(display, capsKeycode, True, CurrentTime);
        XFlush(display);

        // Simulate Release
        XTestFakeKeyEvent(display, capsKeycode, False, CurrentTime);
        XFlush(display);
    }

    windowManager->destroyWindow();
    delete windowManager;
    return 0;
}
