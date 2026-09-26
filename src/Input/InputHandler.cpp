#include "InputHandler.hpp"
#include <stdexcept>
#include <filesystem>

InputHandler::InputHandler(int argc, char* argv[]) {
    parse_arguments(argc, argv);
}

std::string InputHandler::get_pattern() const { return pattern_; }
std::string InputHandler::get_target() const { return target_input_; }
InputType InputHandler::get_input_type() const { return type_; }

void InputHandler::parse_arguments(int argc, char* argv[]) {
    if (argc < 3) {
        throw std::invalid_argument("Usage: ./regex_tool <pattern> <string|filepath>");
    }

    pattern_ = argv[1];
    target_input_ = argv[2];

    if (std::filesystem::exists(target_input_) && 
        std::filesystem::is_regular_file(target_input_)) {
        type_ = InputType::FilePath;
    } else {
        type_ = InputType::DirectString;
    }
}