Markdown# Regular Expression NFA Matcher & CLI Tool (`regex_tool`)

A high-performance C++17 regular expression matching utility based on **Thompson's Construction Algorithm** and **Dijkstra's Shunting-Yard Algorithm**. This project converts infix regular expressions into Non-deterministic Finite Automata (NFAs) to evaluate text strings and streaming files without catastrophic backtracking.

Built as part of **Programming Assignment 1 (Track B: Implementation and Building Something)**.

---

## Features & Supported Regex Syntax

The core engine compiles input regex patterns into an NFA state machine and executes character evaluations using set-based $\epsilon$-closure processing.

| Feature | Syntax Example | Description |
|---|---|---|
| **Literal Characters** | `a`, `b`, `1` | Matches exact literal characters |
| **Implicit Concatenation** | `ab` | Sequences expressions (`a` followed by `b`) |
| **Alternation** | `a\|b` | Matches either expression `a` OR `b` |
| **Grouping** | `(a\|b)c` | Overrides operator precedence via parenthetical sub-NFAs |
| **Kleene Star** | `a*` | Matches **0 or more** occurrences of `a` |
| **Plus Quantifier** | `a+` | Matches **1 or more** occurrences of `a` |
| **Optional Quantifier** | `a?` | Matches **0 or 1** occurrence of `a` |

---

## Project Structure

```text
AdvAlgAT1/
├── bin/                  # Generated binary outputs (git-ignored)
├── build/                # Compiled object files (.o) (git-ignored)
├── src/                  # Source code directory
│   ├── Input/
│   │   ├── FileReader.hpp / .cpp     # Streaming file line-by-line reader
│   │   └── InputHandler.hpp / .cpp   # CLI argument parsing & path resolution
│   ├── Logic/
│   │   ├── RegexMatcher.hpp / .cpp   # Shunting-yard parser & Thompson NFA engine
│   │   └── Types.hpp                 # Common struct/enum definitions
│   ├── Output/
│   │   └── OutputFormatter.hpp / .cpp # ANSI terminal color rendering
│   ├── RegexApp.hpp / .cpp           # Main application workflow controller
│   └── main.cpp                      # Entry point
├── .gitignore            # Ignores build artifacts and OS junk
├── Makefile              # Project compilation script
└── README.md             # Project documentation
Prerequisites & CompilationRequirementsC++ Compiler: g++ or clang++ with C++17 support.Build System: GNU Make.Building the ProjectTo compile the application, run make from the root directory:Bashmake
This generates:Object files in build/The target executable in bin/regex_toolTo clean all build artifacts and start fresh:Bashmake clean
Usageregex_tool accepts two command-line arguments: a regex pattern and a target (which can be either a raw text string or a path to a file).Bash./bin/regex_tool "<pattern>" "<target_string_or_filepath>"
Example 1: Direct String InputPassing a raw string evaluates the regex against the text and highlights the matching substring in bold red terminal output:Bash./bin/regex_tool "(a|b)*c" "test string with abac inside"
Output:Plaintexttest string with abac inside
(where abac is highlighted in ANSI bold red)Example 2: File Search InputPassing a valid file path streams the file line-by-line, matching lines, and printing them with line numbers and highlighted matches:Bash./bin/regex_tool "cat+" sample.txt
Output:PlaintextLine 4: The cat sat on the mat.
Line 12: A wild caaat appeared.
Architecture OverviewInputHandler: Parses argc/argv and uses std::filesystem to automatically distinguish between raw string inputs and file paths.RegexMatcher:Pre-processes pattern strings to insert explicit concatenation symbols (.).Converts infix regex syntax to postfix (RPN) using the Shunting-Yard algorithm.Constructs the NFA graph using Thompson's Construction.Evaluates text using set-based active state transitions and recursive $\epsilon$-closures.FileReader: Reads target files line-by-line to process large files with minimal memory overhead.OutputFormatter: Wraps matched start and end indices ([start_idx, end_idx)) with ANSI escape codes (\033[1;31m) for visual terminal highlighting.