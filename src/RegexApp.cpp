#include "RegexApp.hpp"
#include "Types.hpp"
#include "Input/InputHandler.hpp"
#include "Input/FileReader.hpp"
#include "Logic/RegexMatcher.hpp"
#include "Output/OutputFormatter.hpp"
#include <iostream>
#include <exception>
#include <filesystem>

int RegexApp::run(int argc, char* argv[]) {
    try {
        // Parse all CLI arguments into the unified configuration struct
        RegexConfig config = InputHandler::parse(argc, argv);
        
        // Pass the pattern (and eventually config flags like -i) to the matcher
        RegexMatcher matcher(config.pattern, config.case_insensitive);
        OutputFormatter formatter;

        // Process every target sequentially
        for (const std::string& target : config.targets) {
            
            bool is_file = std::filesystem::exists(target) && std::filesystem::is_regular_file(target);

            if (!is_file) {
                // Treat as a direct string
                MatchResult match = matcher.find_match(target);
                formatter.display_string_match(target, match);
            }
            else {
                // Treat as a file path
                FileReader reader(target);
                if (!reader.open()) {
                    std::cerr << "Error: Failed to open file path " << target << "\n";
                    continue; // Skip to the next target instead of crashing
                }

                std::string line;
                size_t line_num = 0;
                while (reader.get_next_line(line, line_num)) {
                    MatchResult match = matcher.find_match(line);
                    bool is_match = match.matched;
                    if (config.invert_match) {
                        is_match = !is_match;
                    }
                    if (is_match) {
                        formatter.display_file_match(line_num, line, match);
                    }
                }
                reader.close();
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Application Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}