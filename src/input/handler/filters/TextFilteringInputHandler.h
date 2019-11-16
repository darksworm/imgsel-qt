#pragma once


#include "FilteringInputHandler.h"

class TextFilteringInputHandler : public FilteringInputHandler {
public:
    std::string getFilterText() override;

    std::function<bool(Image *)> getFilter() override;

    InputMode getNextMode() override;

    InputInstruction *handleKeyPress(unsigned keyPress) override;

protected:
    bool shouldAddToBuffer(unsigned keyPress) override;

    std::string bufferToString();

private:
    bool isTextualKey(unsigned keyPress);
};



