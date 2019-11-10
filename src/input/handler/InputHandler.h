#pragma once

#include <string>
#include <set>
#include "../../image/image.h"
#include "instruction/InputInstruction.h"
#include "InputMode.h"

class InputHandler {
public:
    virtual InputInstruction *handleKeyPress(unsigned keyCode);
    virtual InputInstruction *handleKeyRelease(unsigned keyCode);
    virtual InputMode getNextMode() = 0;
protected:
    void addModifier(unsigned keyCode);
    void removeModifier(unsigned keyCode);
    bool isModifier(unsigned keyCode);
    bool isModifierActive(unsigned key);

private:
    std::set<unsigned int> activeModifiers;
};