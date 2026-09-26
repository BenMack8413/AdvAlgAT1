#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <string>
#include "Types.hpp"

class InputHandler {
public:
    InputHandler(int argc, char* argv[]);

    std::string get_pattern() const;
    std::string get_target() const;
    InputType get_input_type() const;

private:
    std::string pattern_;
    std::string target_input_;
    InputType type_ = InputType::DirectString;

    void parse_arguments(int argc, char* argv[]);
};

#endif // INPUT_HANDLER_HPP