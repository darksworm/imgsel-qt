#pragma once

#include "../../InputMode.h"
#include "InputInstruction.h"

class CopyInstruction : public InputInstruction {
private:
public:
    explicit CopyInstruction()
            : InputInstruction(InputInstructionType::COPY) {};
};
