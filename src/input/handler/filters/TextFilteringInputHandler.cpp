

#include "TextFilteringInputHandler.h"

std::string TextFilteringInputHandler::getFilterText() {
    return std::__cxx11::string();
}

bool TextFilteringInputHandler::isHotkeyVisible(Hotkey hotkey) {
    return false;
}

InputMode TextFilteringInputHandler::getNextMode() {
    return InputMode::KEY_FILTER;
}
