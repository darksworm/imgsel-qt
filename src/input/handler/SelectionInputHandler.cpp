#include <QtCore/Qt>
#include "instruction/MoveInstruction.h"
#include "instruction/ModeChangeInstruction.h"
#include "SelectionInputHandler.h"

InputInstruction *SelectionInputHandler::handleKeyPress(unsigned keyCode) {
    auto instruction = InputHandler::handleKeyPress(keyCode);

    if (keyCode == Qt::Key_Q) {
        instruction = new InputInstruction(InputInstructionType::EXIT);
        return instruction;
    }

    if (instruction->getType() == InputInstructionType::NONE) {
        ImagePickerMove move = ImagePickerMove::NONE;

        switch (keyCode) {
            case Qt::Key_Slash:
                delete instruction;
                instruction = new ModeChangeInstruction(InputMode::DEFAULT);
                break;
            case Qt::Key_H:
                move = ImagePickerMove::PREVIOUS;
                break;
            case Qt::Key_L:
                move = ImagePickerMove::NEXT;
                break;
            case Qt::Key_J:
                move = ImagePickerMove::DOWN;
                break;
            case Qt::Key_K:
                move = ImagePickerMove::UP;
                break;
            default:
                move = ImagePickerMove::NONE;
                break;
        }

        if (move != ImagePickerMove::NONE) {
            delete instruction;
            instruction = new MoveInstruction(move, repeatNextCommandTimes);
        } else if (keyCode == Qt::Key_C) {
            // change to the same mode, but clear filters.
            instruction = new ModeChangeInstruction(InputMode::VIM, 0, false, true);
        }
    }

    if (keyCode == Qt::Key_C && repeatNextCommand) {
        repeatNextCommandTimes = repeatNextCommandTimes * 10;
    } else if (keyCode >= Qt::Key_1 && keyCode <= Qt::Key_9) {
        if (repeatNextCommand) {
            repeatNextCommandTimes = repeatNextCommandTimes * 10 + (keyCode - 1);
        } else {
            repeatNextCommandTimes = keyCode - 1;
        }

        repeatNextCommand = true;
    } else if (!isModifier(keyCode) && keyCode != Qt::Key_G) {
        repeatNextCommandTimes = 1;
        repeatNextCommand = false;
    }

    if (keyCode == Qt::Key_G) {
        delete instruction;

        if (isModifierActive(Qt::Key_Shift)) {
            if (repeatNextCommand) {
                unsigned targetLine = repeatNextCommandTimes;
                instruction = new MoveInstruction(ImagePickerMove::LINE, targetLine);
            } else {
                instruction = new MoveInstruction(ImagePickerMove::END, 1);
            }

        } else {
            instruction = new MoveInstruction(ImagePickerMove::HOME, 1);
        }

        repeatNextCommand = false;
        repeatNextCommandTimes = 1;
    }

    return instruction;
}

InputMode SelectionInputHandler::getNextMode() {
    return InputMode::DEFAULT;
}
