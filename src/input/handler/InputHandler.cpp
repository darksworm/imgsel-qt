#include <QtCore/Qt>
#include "InputHandler.h"
#include "instruction/ModeChangeInstruction.h"
#include "instruction/CopyInstruction.h"
#include "instruction/MoveInstruction.h"

InputInstruction *InputHandler::handleKeyPress(unsigned keyCode) {
    if (keyCode == Qt::Key_Escape) {
        return new InputInstruction(InputInstructionType::CANCEL);
    }

    if (keyCode == Qt::Key_CapsLock) {
        InputMode mode = getNextMode();

        return new ModeChangeInstruction(mode);
    }

    if (keyCode == Qt::Key_Return || keyCode == Qt::Key_Enter) {
        auto preprocessorFlags = PreprocessorFlags::None;

        if(isModifierActive(Qt::Key_Alt) || isModifierActive(Qt::Key_AltGr)) {
            preprocessorFlags = preprocessorFlags | PreprocessorFlags::WhatsAppWhitespace;
        }

        return new CopyInstruction(preprocessorFlags);
    }

    if (isModifier(keyCode)) {
        addModifier(keyCode);
    }

    if (keyCode == Qt::Key_C && isModifierActive(Qt::Key_Control)) {
        return new InputInstruction(InputInstructionType::EXIT);
    }

    if (keyCode == Qt::Key_Delete) {
        return new InputInstruction(InputInstructionType::DELETE);
    }

    ImagePickerMove move = ImagePickerMove::NONE;

    switch (keyCode) {
        case Qt::Key_Left:
            move = ImagePickerMove::PREVIOUS;
            break;
        case Qt::Key_Right:
            move = ImagePickerMove::NEXT;
            break;
        case Qt::Key_Down:
            move = ImagePickerMove::DOWN;
            break;
        case Qt::Key_Up:
            move = ImagePickerMove::UP;
            break;
        case Qt::Key_PageDown:
            move = ImagePickerMove::PG_DOWN;
            break;
        case Qt::Key_PageUp:
            move = ImagePickerMove::PG_UP;
            break;
        case Qt::Key_Home:
            move = ImagePickerMove::HOME;
            break;
        case Qt::Key_End:
            move = ImagePickerMove::END;
            break;
    }

    if (move != ImagePickerMove::NONE) {
        return new MoveInstruction(move, 1);
    }

    return new InputInstruction(InputInstructionType::NONE);
}

InputInstruction *InputHandler::handleKeyRelease(unsigned keyCode) {
    if (isModifier(keyCode)) {
        removeModifier(keyCode);
    }

    return new InputInstruction(InputInstructionType::NONE);
}

void InputHandler::removeModifier(unsigned keyCode) {
    activeModifiers.erase(keyCode);
}

bool InputHandler::isModifier(unsigned keyCode) {
    switch (keyCode) {
        case Qt::Key_Shift:
        case Qt::Key_Control:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
            return true;
    }

    return false;
}

void InputHandler::addModifier(unsigned keyCode) {
    activeModifiers.insert(keyCode);
}

bool InputHandler::isModifierActive(unsigned int key) {
    return activeModifiers.find(key) != activeModifiers.end();
}
