
#include <vector>
#include <cstring>
#include <algorithm>
#include <linux/input-event-codes.h>
#include <QtCore/Qt>
#include "TextFilteringInputHandler.h"
#include "../../../lib/keycode/keycode.h"
#include "../instruction/FilterInstruction.h"

std::string TextFilteringInputHandler::getFilterText() {
    return this->bufferToString();
}

InputMode TextFilteringInputHandler::getNextMode() {
    return InputMode::VIM;
}

std::function<bool(Image *)> TextFilteringInputHandler::getFilter() {
    auto bufferString = bufferToString();

    auto upperCase = [](std::string input) {
        for (char &it : input)
            it = static_cast<char>(toupper((unsigned char) it));
        return input;
    };

    /** mom, look at me im using lambdas
     AKA refactor me please */
    return std::function<bool(Image *hotkey)>(
            [bufferString, upperCase](Image *image) {
                auto str = upperCase(bufferString);
                auto fileName = image->getFilename();
                auto lastSlashPos = fileName.find_last_of('.');

                if(lastSlashPos != std::string::npos) {
                    fileName = fileName.substr(0, lastSlashPos);
                }

                return upperCase(fileName).find(str) != std::string::npos;
            }
    );
}

InputInstruction *TextFilteringInputHandler::handleKeyPress(unsigned keyPress) {
    switch (keyPress) {
        case Qt::Key_Backspace: {
            if (!this->buffer.empty()) {
                this->buffer.pop_back();
            }

            return new FilterInstruction(this->getFilter(), this->getFilterText());
        }

        case Qt::Key_Backslash: {
            this->buffer.clear();

            return new FilterInstruction(this->getFilter(), this->getFilterText());
        }

        default:
            return FilteringInputHandler::handleKeyPress(keyPress);
    }

}

bool TextFilteringInputHandler::shouldAddToBuffer(unsigned keyPress) {
    return isTextualKey(keyPress);
}

bool TextFilteringInputHandler::isTextualKey(unsigned keyPress) {
    return keyPress >= Qt::Key_Space && keyPress <= Qt::Key_AsciiTilde;
}

std::string TextFilteringInputHandler::bufferToString() {
    std::string bufferString;

    for (auto &&keyPress: buffer) {
        bufferString.push_back(*keycode_linux_rawname(keyPress));
    }

    return bufferString;
}
