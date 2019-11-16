#include <iostream>
#include <exception>

struct ImageNotLoadable : public std::exception {
    const char *what() const noexcept override {
        return "Out of bounds";
    }
};

