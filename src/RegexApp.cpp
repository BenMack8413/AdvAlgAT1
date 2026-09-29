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
        RegexConfig config = InputHandler::parse(argc, argv);
        RegexMatcher matcher(config.pattern, config.case_insensitive);
        OutputFormatter formatter;

        size_t total_match_count = 0;

        for (const std::string& target : config.targets) {
            bool is_file = std::filesystem::exists(target) && std::filesystem::is_regular_file(target);

            if (!is_file) {
                MatchResult match = matcher.find_match(target);

                if (match.matched) {
                    total_match_count++;
                    if (!config.count_only) {
                        formatter.display_string_match(target, match);
                    }
                }
            }
            else {
                FileReader reader(target);
                if (!reader.open()) {
                    std::cerr << "Error: Failed to open file path " << target << "\n";
                    continue;
                }

                std::string line;
                size_t line_num = 0;
                while (reader.get_next_line(line, line_num)) {
                    MatchResult match = matcher.find_match(line);

                    if (match.matched) {
                        total_match_count++;
                        if (!config.count_only) {
                            formatter.display_file_match(line_num, line, match);
                        }
                    }
                }
                reader.close();
            }
        }

        if (config.count_only) {
            std::cout << total_match_count << "\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Application Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}