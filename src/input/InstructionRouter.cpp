#include <memory>
#include <thread>
#include <iostream>
#include "../gui/WindowManager.h"
#include "../util/ThreadSafeQueue.h"
#include "XEventWrapper.h"
#include "../image/image.h"
#include "../gui/ImagePickerDrawer.h"
#include "handler/InputHandler.h"
#include "x11_keycodes.h"
#include "handler/InputHandlerFactory.h"
#include "handler/instruction/ModeChangeInstruction.h"
#include "handler/instruction/MoveInstruction.h"
#include "handler/instruction/FilterInstruction.h"
#include "handler/instruction/CopyInstruction.h"
#include "../util/helpers.h"
#include "../config/ConfigManager.h"

void handleEvents(WindowManager *windowManager, ThreadSafeQueue<XEventWrapper> *eventQueue,
                  std::vector<Image> images, bool *shouldQuit) {
    std::unique_ptr<ImagePickerDrawer> itemPickerDrawer(
            new ImagePickerDrawer(windowManager, &images)
    );

    unsigned int width, height;

    auto config = ConfigManager::getOrLoadConfig();
    std::unique_ptr<InputHandler> inputHandler(nullptr);

    bool keep_running = true, readEvent;
    XEventWrapper event;
    InputMode state = config.getDefaultInputMode();

    bool initialFrameDrawn = false;

    while (keep_running) {
        unsigned keyCode = 0;

        readEvent = eventQueue->next(event);

        /*
        * TODO: this probably shouldn't be a static value
        *   we should probably calculate how much time has passed between cycles
        *   and sleep for (16.6ms - passed time)
        */
        std::this_thread::sleep_for(std::chrono::nanoseconds(16600000));

        if (!readEvent) {
            continue;
        }

        if (event.eventType == -1) {
            keep_running = false;
            continue;
        }

        if (event.eventType == ConfigureNotify || event.eventType == Expose) {
            if (initialFrameDrawn && event.eventType == ConfigureNotify) {
                continue;
            }

            windowManager->getWindowDimensions(&width, &height);
            auto hk = itemPickerDrawer->getSelectedImage() ? itemPickerDrawer->getSelectedImage()
                                                           : &*images.begin();
            itemPickerDrawer->drawFrame(hk, true);
            drawText(windowManager, state == InputMode::DEFAULT ? "DEFAULT" : "VIM", Dimensions(10, height - 10));

            initialFrameDrawn = true;
            continue;
        }

        if (!InputHandlerFactory::isCorrectHandler(inputHandler.get(), state)) {
            inputHandler.reset(InputHandlerFactory::getInputHandler(state));
        }

        if (event.eventType == KeyPress || event.eventType == KeyRelease) {
            keyCode = x11_keycode_to_libinput_code(event.keySym);
        }

        //spdlog::debug("RAW: {} FORMATTED: {} {}", keycode_linux_rawname(keyCode),
                      //keycode_linux_name(keycode_linux_to_hid(keyCode)), keyCode);

        std::unique_ptr<InputInstruction> instruction;

        if (event.eventType == KeyPress) {
            instruction = std::unique_ptr<InputInstruction>(inputHandler->handleKeyPress(keyCode));
        } else if (event.eventType == KeyRelease) {
            instruction = std::unique_ptr<InputInstruction>(inputHandler->handleKeyRelease(keyCode));
        } else continue;

        if (instruction->getType() == InputInstructionType::NONE) {
            continue;
        }

        bool shouldExit =
                itemPickerDrawer->getFilterString().empty() && instruction->getType() == InputInstructionType::CANCEL;

        if (shouldExit || instruction->getType() == InputInstructionType::EXIT) {
            keep_running = false;
            continue;
        }

        if (instruction->getType() == InputInstructionType::CANCEL) {
            itemPickerDrawer->clearFilter();
            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<MoveInstruction *>(instruction.get())) {
            auto moveInstruction = ((MoveInstruction *) instruction.get());
            auto move = moveInstruction->getMoveDirection();

            bool moved = false;

            if (move != ImagePickerMove::NONE) {
                moved = itemPickerDrawer->move(moveInstruction->getMoveDirection(), moveInstruction->getMoveSteps());
            }

            if (move == ImagePickerMove::NONE || !moved) {
                itemPickerDrawer->drawFrame(itemPickerDrawer->getSelectedImage());
            }
        } else if (dynamic_cast<ModeChangeInstruction *>(instruction.get())) {
            auto modeChangeInstruction = (ModeChangeInstruction *) instruction.get();
            state = modeChangeInstruction->newMode;

            if (modeChangeInstruction->shouldClearFilters) {
                itemPickerDrawer->clearFilter();
            }

            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<FilterInstruction *>(instruction.get())) {
            auto filterInstruction = ((FilterInstruction *) instruction.get());

            if (!filterInstruction->getFilterString().empty()) {
                itemPickerDrawer->setFilter(filterInstruction->getFilter(), filterInstruction->getFilterString());
            } else {
                itemPickerDrawer->clearFilter();
            }
            itemPickerDrawer->drawFrame(nullptr);
        } else if (dynamic_cast<CopyInstruction *>(instruction.get())) {
            auto selectedImage = itemPickerDrawer->getSelectedImage();
            auto path = selectedImage->getPath();

            if (config.shouldPrintFilePath()) {
                std::cout << path;
            } else {
                auto ext = selectedImage->getExtension();
                ext = ext == "jpg" ? "jpeg" : ext;

                // TODO: handle 's in filenames
                std::string command = "cat '" + path + "' | xclip -selection clipboard -target image/" + ext + " -i";
                system(command.c_str());
            }

            keep_running = false;
        }

        drawText(windowManager, state == InputMode::DEFAULT ? "DEFAULT" : "VIM", Dimensions(10, height - 10));

        XClearArea(
            windowManager->getDisplay(),
            windowManager->getWindow(),
            0,
            0,
            width,
            60,
            false
        );

        if (!itemPickerDrawer->getFilterString().empty()) {
            drawCenteredText(windowManager, itemPickerDrawer->getFilterString(), Dimensions(width / 2, 30));
        }
    }

    *shouldQuit = true;
}


