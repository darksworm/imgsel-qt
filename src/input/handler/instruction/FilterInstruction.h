#pragma once

#include <utility>
#include <functional>

#include "InputInstruction.h"
#include "../../../image/image.h"

class FilterInstruction : public InputInstruction {
private:
    std::function<bool(Image *hotkey)> filterFunc;
    std::string filterString;
public:
    explicit FilterInstruction(std::function<bool(Image *hotkey)> filterFunc, std::string filterString)
            : InputInstruction(
            InputInstructionType::FILTER), filterString(std::move(filterString)) {
        this->filterFunc = std::move(filterFunc);
    }

    std::function<bool(Image *)> getFilter() {
        return filterFunc;
    }

    const std::string &getFilterString() const {
        return filterString;
    }
};