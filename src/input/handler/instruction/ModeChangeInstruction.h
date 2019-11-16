#pragma once


#include "../InputMode.h"
#include "InputInstruction.h"

class ModeChangeInstruction : public InputInstruction {
public:
    const InputMode newMode;
    const bool shouldClearFilters;
    const unsigned bufferedKeyPress;
    const bool sendBuffer;

    explicit ModeChangeInstruction(InputMode newMode, unsigned bufferedKeypress = 0, bool sendBuffer = false,
                                   bool shouldClearFilters = false)
            : InputInstruction(InputInstructionType::CHANGE_MODE), newMode(newMode), bufferedKeyPress(bufferedKeypress),
              sendBuffer(sendBuffer), shouldClearFilters(shouldClearFilters) {};
};



