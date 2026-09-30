# (b) Track B: The Tool

## What it does

`regex_tool` is a small grep-style command-line program built on my own regex engine. Given a pattern and one or more targets, it searches each target and prints the matches with the matched text highlighted in bold red. A target can be a literal string or a file, and files are searched line by line with line numbers. If nothing matches it prints nothing, following the Unix convention of silent failure.

```
regex_tool [FLAGS] [--] <pattern> [target1 target2 ...]
```

| Flag | Effect |
| :--- | :--- |
| `-i` | Case-insensitive matching |
| `-c` | Print only a count instead of the matches |
| `--` | End of flags, so a pattern can start with `-` |

The tool is not a wrapper around `std::regex`. Everything from parsing the pattern to matching text is implemented in the project.

## How the algorithm sits inside the tool

The tool has three layers, each in its own folder:

```
Input/    InputHandler, FileReader          command line and file reading
Logic/    Tokeniser, Parser, NfaBuilder,    the regex engine
          RegexMatcher
Output/   OutputFormatter                   highlighted printing
```

`RegexApp::run` connects them:

1. `InputHandler::parse` turns `argv` into a config: the pattern, the targets, and the two flags.
2. `RegexMatcher` is constructed **once** from the pattern. That compiles the pattern to an NFA (tokenise, convert to postfix, build the Thompson NFA).
3. For every target, the app calls `find_match` on either the whole string or each line of the file. `find_match` is the NFA simulation, and it returns a start and end index.
4. `OutputFormatter` uses those indices to print the text before the match, the match in red, and the text after.

The important point is that the engine only exposes one operation, `find_match(text, start_from)`, which returns a `MatchResult`. Everything the tool does, including printing, counting and multiple targets, is built from repeated calls to it. With `-c` on a string target, the app calls `find_match` repeatedly, moving `start_from` past each match (and by one character after an empty match, to avoid looping forever), which is why `start_from` exists.

Because the pattern is compiled once and reused for every target and every line, compile cost is paid once regardless of file size, and files are read one line at a time so they never need to fit in memory.

## Interface decisions

**Follow grep's conventions where they help.** Pattern first, then targets, silent when nothing matches, `-i` and `-c` with the same meaning as grep, and combined short flags (`-ic`).

**Flags before the pattern, with `--` as an escape hatch.** The parser stops reading flags at the first positional argument, so the tool can never mistake part of a pattern or a target for a flag. The cost is that a pattern starting with `-` needs `--`. I chose this because a regex tool's patterns are full of punctuation, and ambiguity there is worse than one extra argument.

**Files and strings share one interface.** If a target is an existing regular file it is read as a file, otherwise it is treated as a string. This makes quick experiments easy (`regex_tool "\d+" "abc 123"`) but has a trade-off: a string that happens to be the name of a file will be read as that file.

**Different output for the two modes.** String targets print the whole string with the match highlighted. File targets print `Line N: ...` so the user can find the line again.

**Errors go to stderr with exit status 1.** An unknown flag, a missing pattern, an unclosed `[` or a trailing backslash is reported as a message rather than a crash.

**`-c` counts what is natural for each mode.** For a string it counts non-overlapping matches. For a file it counts matching lines, as grep does.

**Multiple targets.** Any mix of strings and files can be given in one call and all are processed in order.

## Worked example

These are real runs. `[red]...[/red]` marks the highlighted region, which appears as bold red in a terminal.

**1. Extract a number from a string**
```
$ regex_tool "\d+" "user id 404 found"
user id [red]404[/red] found
```

**2. Search files for errors and warnings.** In a pattern `|` means alternation, so the pattern only needs to be quoted so the shell doesn't treat it as a pipe.
```
$ regex_tool "ERROR|WARN" tests/test_logs.txt tests/test_multi_line.txt
Line 2: 2026-09-29 10:02:15 [[red]WARN[/red]] High memory usage: 88%
Line 2: [red]ERROR[/red]: Database connection failed on port 5432
Line 4: [red]ERROR[/red]: Timeout reached after 3000ms
```
Both files are searched in one call. Line numbers restart for each file.

**3. Find IP addresses in a log**
```
$ regex_tool "\d+\.\d+\.\d+\.\d+" tests/test_logs.txt
Line 1: 2026-09-29 10:00:01 [INFO] User logged in from [red]192.168.1.50[/red]
Line 3: 2026-09-29 10:05:30 [INFO] User logged out from [red]10.0.0.1[/red]
```

**4. Count matching lines, ignoring case**
```
$ regex_tool -ic "error" tests/test_multi_line.txt
2
```

**5. Greedy versus lazy matching.** The engine has both, and the difference is visible on the same input:
```
$ regex_tool "<.*?>" "<a>text</a>"
[red]<a>[/red]text</a>

$ regex_tool "<.*>" "<a>text</a>"
[red]<a>text</a>[/red]
```
Lazy `.*?` stops at the first `>` and greedy `.*` runs to the last one. Inside the engine this is decided by the order of two edges in the NFA (see the "What I Built" document).

**6. No match is silent**
```
$ regex_tool "zzz" "hello"
$ echo $?
0
```
Unlike grep, the exit status is still 0 when nothing matches.

## Testing

The tool is tested with a Bash suite (`make test`) that first generates fixture files and then runs 127 test cases grouped into categories: literals, wildcards, classes and ranges, negated classes, shorthands, escapes, quantifiers, lazy quantifiers, alternation, grouping, anchors, word boundaries, counted repetition, composite patterns (emails, IP addresses), CLI flags, multiple inputs, and file edge cases (empty file, missing final newline, blank lines, a 20,000-character line, symbol-heavy config files). All 127 pass. A test passes when the expected text appears anywhere in the output, which is a loose check, so the tests confirm that a match was found more than they confirm the exact highlighted span.

## Known limitations

* **Crash on an empty alternation branch.** `regex_tool "a|" "abc"` crashes with a segmentation fault (exit code 139), because the NFA builder pops from an empty stack. Empty groups like `()` and a leading quantifier like `*a` are likely to fail the same way.
* **Unbalanced parentheses are not reported.** `regex_tool "(abc" "abc"` prints a match and exits 0 instead of reporting an error.
* **No filename on multi-file output.** When several files are searched, the output shows only `Line N`, so the user cannot tell which file a line came from.
* **Exit status.** A non-match exits 0, unlike grep, which exits 1, so the tool can't be used directly in shell `if` tests.
* **Missing features.** No capture groups, backreferences or lookaround; input is treated as bytes rather than Unicode; `\r` and escapes inside `[...]` are not supported; only the first match on each line is highlighted.
* **Performance.** The matcher restarts the NFA simulation at every start position, so the worst case for a line of length n and a pattern of size m is O(n² · m). There is no backtracking, so it can't blow up exponentially.