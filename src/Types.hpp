#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstddef>

struct MatchResult {
    bool matched = false;
    size_t start_idx = 0;
    size_t end_idx = 0;
};

enum class InputType {
    DirectString,
    FilePath
};

#endif // TYPES_HPP