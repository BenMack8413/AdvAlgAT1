# (c) What I Learned

**AI makes far more mistakes when it is used carelessly.** This was the first major project I used AI for, and I ran into many errors I didn't anticipate. Some were small (a test with the wrong expected value, spelling differences), but one was serious: I asked an AI to add more features and then to debug the problems its own changes had introduced, and the code got worse each round until it didn't work at all. I ended up resetting everything to my last commit. That taught me that AI debugging its own output can compound errors instead of fixing them, and that committing working code before asking for new features is what made recovery possible.

**Tests can be wrong too.** One of my tests used the pattern `go+l` against `goooal`, which can never match because of the `a`. The regex engine was fine; the test was wrong. Before this I assumed a failing test meant broken code. Now I check whether the test itself is correct.

**Reading AI output matters.** I found a real bug only because I read what the AI wrote: in `RegexApp.cpp` it used `target.type` and `target.value`, which were never defined anywhere. I fixed it by using `target` directly for the value and adding my own logic to work out the type (file or string).

**Regex.** I had a basic understanding of regular expressions before, but I now know much more syntax: character classes and ranges, shorthands, anchors, word boundaries, counted repetition, and the difference between greedy and lazy quantifiers.

**Testing.** I learned how a test suite can be structured, with a shell script that generates its own fixture files, groups tests into categories, and prints a pass/fail summary.

**C++ projects.** This is the first substantial project I have finished, and my first in C++. I learned how C++ source files are compiled into a binary, how a Makefile automates that, and how to run the resulting program from the command line.

# (d) AI Use

## Tools and how much

I used **Gemini** and **Claude**. Gemini wrote most of the code. Claude wrote a small part of the code (the lazy quantifier support) and helped with the final README and other non-coding sections such as the report write-ups.

## What I used them for

* **Gemini:** writing most of the implementation, debugging, writing the test suite, and writing the first versions of the documentation.
* **Claude:** the lazy quantifier code, final touches to the README, and non-coding sections of the report.

## Where the AI was wrong or unhelpful

1. **A wrong test.** Gemini generated the test `run_test "Direct String Plus Quantifier" "go+l" "goooal" "goooal"`. The pattern needs an `l` but the input has an `a`, so it could never match. When I questioned it, the pattern was corrected to `go+al`. The lesson was to sanity-check the expected result by hand rather than trusting generated tests.
2. **Debugging spiral.** When I asked the AI to add more features, it introduced bugs, and when I asked it to debug those, it made further mistakes. After numerous rounds the program stopped working entirely, so I reverted all changes to my last commit and restarted the feature from scratch. As far as I recall, this was my first attempt at implementing lazy quantifiers.
3. **Undefined members in `RegexApp.cpp`.** While updating the file, the AI used `target.type` and `target.value` to get the type and value of an argument, but no such fields existed. I found this by reading the output, changed `target.value` to `target`, and wrote logic to determine the type.
4. **Spelling mismatch.** The AI repeatedly mixed American and Australian spellings across function and variable names, which caused inconsistencies that I had to find and correct by hand.
5. **A test pointing at the wrong file.** Gemini also produced the test `run_cli_test "Count Matches (File)" "2" "-c" "line" "tests/test_multi_line.txt"`, which originally referenced the wrong fixture file and so produced the wrong result. The tool itself was fine; the test was checking the wrong input. I corrected the file path.

## What I understood versus what I took on trust

I understand the **input handler** well: how flags, the `--` delimiter and positional arguments are parsed. I understand the **tokeniser**'s overall structure and purpose, although not all of its more complex logic (for example the counted-repetition `{n,m}` expansion).

I largely took the **Parser**, **NfaBuilder** and **RegexMatcher** on trust. I know at a high level what they are for (convert to postfix, build an NFA, simulate it), but I can't yet explain the details of their logic with confidence. My main evidence that they work is that the test suite passes, and since much of that suite was also written with AI help, that evidence is weaker than it looks. I also don't remember every small typo or bug I fixed along the way.