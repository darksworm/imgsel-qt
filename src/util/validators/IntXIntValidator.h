#pragma once
#include "../lib/CLI11.hpp"
#include "../StringTools.h"

struct IntXIntValidator : public CLI::Validator {
    IntXIntValidator() {
        this->name("IntXInt");
        this->func_ = [](const std::string &str) {
            auto input = CLI::detail::split(CLI::detail::to_lower(str), 'x');

            if(input.size() != 2) {
                return std::string("Incorrect parameter passed, format should be {INT}x{INT}");
            }

            auto inputWidth = input.at(0);
            auto inputHeight = input.at(1);

            if(!StringTools::isNumber(inputWidth) || !StringTools::isNumber(inputHeight)) {
                return std::string("Incorrect parameter passed, format should be {INT}x{INT}");
            }

            return std::string("");
        };
    }
};
