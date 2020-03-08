#pragma once

enum class PreprocessorFlags {
    None = 0,
    WhatsAppWhitespace = 1
};

PreprocessorFlags operator|(PreprocessorFlags lhs, PreprocessorFlags rhs);
bool operator&(PreprocessorFlags lhs, PreprocessorFlags rhs);
