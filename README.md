# Regex CLI Tool

A custom, lightweight regular expression engine built from scratch in C++. It parses regex patterns into Non-Deterministic Finite Automata (NFAs) using Thompson's Construction and evaluates text via NFA simulation. It supports both direct string evaluation and line-by-line file scanning, mirroring the standard Unix philosophy of silent failures and highlighted terminal output.

## Core Features

* **NFA-Based Evaluation:** Compiles patterns into NFAs for efficient, linear-time matching without catastrophic backtracking.
* **Greedy Matching:** Implements the left-most, longest match rule to correctly handle unbounded quantifiers (`+`, `*`).
* **Dual Execution Modes:** Scan a direct string argument or parse multiline text files.
* **Unix-Style Output:** Returns colored ANSI highlights for valid matches and fails silently (empty output) for non-matches.
* **Comprehensive Test Suite:** Includes stress tests for buffer limits, complex file format parsing, and detailed string edge cases.

## Supported Regex Syntax

| Feature | Syntax | Example | Description |
| :--- | :--- | :--- | :--- |
| **Literals** | `a`, `b`, `1` | `cat` | Matches exact characters. |
| **Wildcard** | `.` | `c.t` | Matches any single character. |
| **Character Classes** | `[...]` | `[aeiou]` | Matches any character inside the brackets. |
| **Class Ranges** | `[x-y]` | `[a-zA-Z0-9]` | Matches characters within the specified ASCII ranges. |
| **Negated Classes** | `[^...]` | `[^0-9]` | Matches any character *not* inside the brackets. |
| **Shorthands** | `\d`, `\w`, `\s` | `\d+` | Matches digits (`\d`), word characters (`\w`), or whitespace (`\s`). |
| **Negated Shorthands** | `\D`, `\W`, `\S` | `\S+` | Matches non-digits (`\D`), non-words (`\W`), or non-whitespace (`\S`). |
| **Whitespace Escapes** | `\t`, `\n` | `\tline` | Matches horizontal tabs or newline characters. |
| **Zero or More** | `*` | `ab*c` | Matches the preceding element 0 or more times (Greedy). |
| **One or More** | `+` | `ab+c` | Matches the preceding element 1 or more times (Greedy). |
| **Zero or One** | `?` | `colou?r` | Matches the preceding element 0 or 1 time. |
| **Alternation** | `\|` | `cat\|dog` | Matches either the pattern on the left or the right. |
| **Grouping** | `(...)` | `(ab)+` | Groups multiple tokens together to apply quantifiers or alternation. |
| **Escaping** | `\` | `a\+b` | Escapes metacharacters to match them literally. |

## Build & Installation

Ensure you have a modern C++ compiler (`g++` or `clang++`) and `make` installed.

```bash
# Compile the executable
make

# Clean build artifacts
make clean
```
## Usage
The tool accepts two arguments: the regex pattern and the target. The target can be a direct string or a file path.
### String Matching 
```bash
./bin/regex_tool "\d+" "user id 404 found"
```
### File Matching
```Bash
./bin/regex_tool "ERROR\|WARN" /var/log/syslog
```
(Note: When no match is found, the tool exits silently without printing anything to standard output.)
## Testing
The project includes an automated Bash test suite that validates AST construction, quantifier greediness, file-reading edge cases, and stress limits.
```Bash
# Make the script executable
chmod +x tests/run_tests.sh

# Run the test suite
./tests/run_tests.sh
```