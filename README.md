# Regex CLI Tool

A regular expression engine built from scratch in C++17, wrapped in a small grep-style command-line tool. Patterns are compiled into a Non-Deterministic Finite Automaton (NFA) using Thompson's Construction and matched by simulating the NFA directly, so there is no backtracking and no exponential blow-up on pathological patterns.

The tool scans direct string arguments or files line by line, highlights matches in the terminal, and stays silent when nothing matches (Unix style).

## Features

* **Thompson's Construction NFA** built from a postfix token stream (shunting-yard parser).
* **Backtracking-free simulation**: the matcher advances a set of active NFA states one character at a time.
* **Greedy and lazy quantifiers**: `*`, `+`, `?`, `{n}`, `{n,}`, `{n,m}`, and lazy variants (`*?`, `+?`, `??`, `{n,m}?`).
* **Anchors and assertions**: `^`, `$`, `\b`, `\B`.
* **Character classes** with ranges and negation, plus `\d \w \s` shorthands (and negations).
* **Case-insensitive mode** (`-i`) and **count mode** (`-c`).
* **Multiple targets**: mix any number of strings and files in one call.
* **Automated test suite** covering syntax, quantifier semantics, CLI parsing, file edge cases and stress inputs.

## Supported Regex Syntax

| Feature | Syntax | Example | Description |
| :--- | :--- | :--- | :--- |
| Literals | `a`, `1` | `cat` | Matches the exact characters. |
| Wildcard | `.` | `c.t` | Any single character except newline. |
| Character class | `[abc]` | `[aeiou]` | Any one character in the set. |
| Class ranges | `[x-y]` | `[a-zA-Z0-9_]` | ASCII ranges, combinable with single characters. |
| Negated class | `[^...]` | `[^0-9]` | Any character not in the set. |
| Shorthands | `\d \w \s` | `\d+` | Digit, word character (`[A-Za-z0-9_]`), whitespace. |
| Negated shorthands | `\D \W \S` | `\S+` | Complements of the above. |
| Whitespace escapes | `\t`, `\n` | `\tindent` | Tab and newline. |
| Hex escape | `\xHH` | `\x41` | Character with the given two-digit hex code. |
| Escaping | `\` + metachar | `a\+b`, `\(x\)` | Matches the metacharacter literally. |
| Zero or more | `*` | `ab*c` | Greedy. |
| One or more | `+` | `ab+c` | Greedy. |
| Zero or one | `?` | `colou?r` | Greedy. |
| Counted repetition | `{n}` `{n,}` `{n,m}` | `a{2,4}` | Exactly n, at least n, or n to m repetitions. |
| Lazy quantifiers | `*?` `+?` `??` `{n,m}?` | `<.*?>` | Prefer the shortest match. |
| Alternation | `\|` | `cat\|dog` | Either side; earlier branches take priority. |
| Grouping | `(...)` | `(ab)+` | Applies quantifiers or alternation to a sub-pattern. |
| Start / end anchors | `^`, `$` | `^Error`, `done$` | Start / end of the line or string. |
| Word boundaries | `\b`, `\B` | `\bcat\b` | Word boundary / not a word boundary. |

**Not supported:** capture groups and backreferences, lookahead/lookbehind, `\r`, escapes or shorthands *inside* `[...]`, Unicode-aware matching (input is treated as bytes), and empty alternation branches or empty groups such as `a|` or `()`.

### Match semantics

The engine reports the **leftmost** match. Among matches starting at that position, quantifier and alternation priority decides the result (Perl/PCRE-style "leftmost-first"): greedy quantifiers prefer more input, lazy quantifiers prefer less, and in `a|b` the left branch is preferred. This is *not* POSIX leftmost-longest, so `cat|category` matches `cat` in the text `category`.

## Build

Requires `g++` (or any C++17 compiler; edit `CXX` in the Makefile) and `make`.

```bash
make          # builds ./bin/regex_tool
make clean    # removes build/ and bin/
```

## Usage

```
regex_tool [FLAGS] [--] <pattern> [target1 target2 ...]
```

A target that is an existing regular file is scanned line by line. Anything else is treated as a literal string to search.

| Flag | Meaning |
| :--- | :--- |
| `-i` | Case-insensitive matching. |
| `-c` | Print only the number of matches instead of the matches themselves. |
| `--` | End of flags; everything after is positional. Lets a pattern start with `-`. |

Short flags can be combined (`-ic`). Flags must come before the pattern.

### Examples

```bash
# Direct string: the first match is highlighted in bold red
./bin/regex_tool "\d+" "user id 404 found"

# File: matching lines are printed with their line numbers
./bin/regex_tool "ERROR|WARN" /var/log/syslog
#   Line 12: ... ERROR ...

# Case-insensitive
./bin/regex_tool -i "hello" "HeLlO WoRlD"

# Count matches (strings: non-overlapping matches; files: matching lines)
./bin/regex_tool -c "cat" "cat dog cat"      # prints 2
./bin/regex_tool -c "line" notes.txt

# Pattern that starts with a dash
./bin/regex_tool -- "-pattern" "this-is-a-pattern-test"

# Multiple targets (strings and files can be mixed)
./bin/regex_tool "^[af]" fruit_a.txt fruit_b.txt
```

Note the shell: quote the pattern so `|`, `*`, `(`, `\` and `$` reach the tool unchanged.

### Output behaviour

* String target: the whole string is printed with the matched part highlighted.
* File target: `Line <n>: <line>` with the matched part highlighted.
* No match: nothing is printed. With `-c`, `0` is printed.
* Invalid pattern or unknown flag: an error message on stderr and exit status 1. Unlike `grep`, a non-match still exits 0.

## Project Structure

```
src/
  main.cpp, RegexApp.{hpp,cpp}   entry point and top-level control flow
  Types.hpp                      MatchResult
  Input/    InputHandler, FileReader        argument parsing, line reading
  Logic/    Tokeniser, Parser, NfaBuilder,  pattern -> tokens -> postfix -> NFA -> match
            RegexMatcher, Token.hpp
  Output/   OutputFormatter                 ANSI highlighting
run_tests.sh, make_test_files.sh            test suite and test-data generator
Makefile
```

## Testing

```bash
make test
```

This builds the tool if needed, runs `make_test_files.sh` to generate the fixture files in `tests/`, and then runs `run_tests.sh`. Tests are grouped into categories (literals, classes, quantifiers, lazy matching, anchors, word boundaries, CLI flags, file edge cases, long-line stress tests, multi-input handling) and a per-category pass/fail summary is printed at the end. The script exits non-zero if any test fails.

## Complexity

Simulation keeps a set of active NFA states, so each input character costs O(m) for a pattern of size m, and there is no backtracking. The tool tries each start position in turn, so scanning a line of length n is O(n² · m) in the worst case, and typically much faster because a start position is abandoned as soon as its state set empties.
