#pragma once

#include "../../InputMode.h"
#include "InputInstruction.h"
#include "PreprocessorFlags.h"

class CopyInstruction : public InputInstruction {
private:
    PreprocessorFlags preprocessorFlags = PreprocessorFlags::None;
public:
    explicit CopyInstruction(PreprocessorFlags preprocessorFlags)
            : InputInstruction(InputInstructionType::COPY), preprocessorFlags(preprocessorFlags) {};
    PreprocessorFlags getPreprocessFlags() {
        return preprocessorFlags;
    }
};
