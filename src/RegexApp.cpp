#include "RegexApp.hpp"
#include "Types.hpp"
#include "Input/InputHandler.hpp"
#include "Input/FileReader.hpp"
#include "Logic/RegexMatcher.hpp"
#include "Output/OutputFormatter.hpp"
#include <iostream>
#include <exception>

int RegexApp::run(int argc, char* argv[]) {
    try {
        InputHandler input(argc, argv);
        RegexMatcher matcher(input.get_pattern());
        OutputFormatter formatter;

        if (input.get_input_type() == InputType::DirectString) {
            std::string target = input.get_target();
            MatchResult match = matcher.find_match(target);
            formatter.display_string_match(target, match);
        } 
        else if (input.get_input_type() == InputType::FilePath) {
            FileReader reader(input.get_target());
            if (!reader.open()) {
                std::cerr << "Error: Failed to open file path " << input.get_target() << "\n";
                return 1;
            }

            std::string line;
            size_t line_num = 0;
            while (reader.get_next_line(line, line_num)) {
                MatchResult match = matcher.find_match(line);
                if (match.matched) {
                    formatter.display_file_match(line_num, line, match);
                }
            }
            reader.close();
        }
    } catch (const std::exception& ex) {
        std::cerr << "Application Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}