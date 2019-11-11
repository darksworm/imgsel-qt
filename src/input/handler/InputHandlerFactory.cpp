

#include "InputHandlerFactory.h"
#include "SelectionInputHandler.h"
#include "filters/TextFilteringInputHandler.h"

InputHandler *InputHandlerFactory::getInputHandler(InputMode type) {
    switch (type) {
        case InputMode::DEFAULT:
            return new TextFilteringInputHandler();
        case InputMode::VIM:
            return new SelectionInputHandler();
        default:
            throw;
    }
}

bool InputHandlerFactory::isCorrectHandler(InputHandler *handler, InputMode type) {
    if (handler == nullptr) {
        return false;
    }

    if (type == InputMode::VIM && dynamic_cast<SelectionInputHandler *>(handler)) {
        return true;
    }

    if (type == InputMode::DEFAULT && dynamic_cast<TextFilteringInputHandler *>(handler)) {
        return true;
    }

    return false;
}
