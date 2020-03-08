#include "PreprocessorFlags.h"

PreprocessorFlags operator|(PreprocessorFlags lhs, PreprocessorFlags rhs) {
    return static_cast<PreprocessorFlags>(static_cast<char>(lhs) | static_cast<char>(rhs));
}

bool operator&(PreprocessorFlags lhs, PreprocessorFlags rhs) {
    return static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs);
}
