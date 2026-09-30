# What I Built

## The algorithm

I implemented a regular expression engine based on **Thompson's Construction**, and built a grep-style command-line tool around it. A pattern is compiled into a Non-deterministic Finite Automaton (NFA), and text is matched by **simulating the NFA directly**: at each input character the matcher holds the set of states the automaton could currently be in and advances all of them together. Because it never backtracks, matching time cannot blow up exponentially on patterns like `(a*)*b`, which is the main advantage over the backtracking engines used by many standard libraries.

The implementation is in C++17 with no external dependencies.

## Pipeline

Compilation is four stages, each in its own class under `src/Logic/`:

1. **Tokeniser**: turns the pattern string into tokens. Literals, classes, shorthands and `.` become a single `Literal` token carrying a character predicate. Operators, parentheses, anchors and lazy quantifiers get their own token types. Counted repetition (`{n,m}`) is expanded here into copies of the preceding atom.
2. **Parser**: inserts explicit concatenation operators, then converts infix to postfix with the shunting-yard algorithm (precedence: quantifiers > concatenation > alternation).
3. **NfaBuilder**: evaluates the postfix stream with a stack of NFA fragments, each with one start and one accept state, exactly as in Thompson's Construction.
4. **RegexMatcher**: runs the simulation. `find_match` tries each start position from left to right and returns the first that yields a match.

## Key design decisions

**Predicate transitions instead of symbol transitions.** Each NFA edge holds a `std::function<bool(char)>` instead of a single character. This keeps the automaton small (`[a-zA-Z0-9]` is one edge, not 62), makes classes, negation, shorthands and `.` trivial to support, and lets case-insensitivity be handled entirely in the tokeniser by wrapping the predicate. The cost is that edges cannot be compared or merged, which rules out an easy NFA-to-DFA conversion.

**Anchors as assertion states.** `^`, `$`, `\b` and `\B` are represented as states with an `anchor_assertion` flag and an epsilon edge. During epsilon-closure the matcher checks the assertion against the current position and simply refuses to enter the state if it fails. This means anchors need no special-casing in the main matching loop.

**Match priority via ordered state lists.** Each state's epsilon edges are stored in priority order, and the epsilon-closure explores them depth-first in that order, preserving it in the output list. For greedy quantifiers the "loop again" edge comes first; for lazy quantifiers the "exit" edge comes first (see `NfaBuilder.cpp`). The matcher treats a state's index in the list as its priority and prefers accepting states with a lower index. This is how greedy versus lazy matching works without any backtracking, and it gives Perl-style leftmost-first semantics rather than POSIX leftmost-longest.

**Ownership.** `NfaGraph` owns every state through `std::vector<std::unique_ptr<State>>`; edges between states are raw non-owning pointers. This avoids leaks and cycles-of-shared_ptr problems, since NFAs for `*` and `+` contain loops.

## Extensions beyond the textbook version

The textbook algorithm covers literals, concatenation, alternation and `*`. I added `+`, `?`, `.`, character classes with ranges and negation, `\d \w \s` and their negations, escapes (`\t`, `\n`, `\xHH`), anchors and word boundaries, counted repetition, lazy quantifiers, and a case-insensitive mode.

## Simplifications and known limits

* No capture groups, backreferences or lookaround (backreferences cannot be done by an NFA at all).
* Input is treated as bytes; there is no Unicode support.
* No DFA construction or caching, so each start position re-simulates from scratch. Worst case for a line of length n and pattern size m is O(n² · m).
* `\r`, and escapes inside `[...]`, are not supported. Empty alternation branches and empty groups are not handled.

## Navigating the code

| If you want to see... | Open |
| :--- | :--- |
| Program flow (parse args, loop over targets, count vs print) | `src/RegexApp.cpp` |
| Command-line flags and the `--` delimiter | `src/Input/InputHandler.cpp` |
| How pattern text becomes tokens; `{n,m}` expansion | `src/Logic/Tokeniser.cpp` |
| Concatenation insertion and shunting-yard | `src/Logic/Parser.cpp` |
| Thompson fragments for each operator, greedy vs lazy edge order | `src/Logic/NfaBuilder.cpp` |
| Epsilon-closure, anchor checks, and the match loop | `src/Logic/RegexMatcher.cpp` |
| Highlighting | `src/Output/OutputFormatter.cpp` |
| Tests | `run_tests.sh`, `make_test_files.sh` |

The tool's interface and a worked example are covered in the Track B write-up; build and usage instructions are in the README.