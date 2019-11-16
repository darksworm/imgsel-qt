#pragma once

#include "InputInstruction.h"
#include "../../../gui/ImagePickerDrawer.h"

class MoveInstruction : public InputInstruction {
private:
    ImagePickerMove moveDirection;
    unsigned int moveSteps = 1;
public:
    ImagePickerMove getMoveDirection() {
        return this->moveDirection;
    }

    explicit MoveInstruction(ImagePickerMove moveDirection, unsigned int moveSteps) : InputInstruction(
            InputInstructionType::MOVE),
                                                                                      moveDirection(moveDirection),
                                                                                      moveSteps(moveSteps) {}

    unsigned int getMoveSteps() const {
        return moveSteps;
    }
};