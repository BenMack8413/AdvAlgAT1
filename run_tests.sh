#!/usr/bin/env bash

# Terminal Colors
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
RESET="\033[0m"

EXECUTABLE="./bin/regex_tool"
PASSED=0
FAILED=0

# Category tracking
CURRENT_CATEGORY=""
declare -A CAT_PASSED
declare -A CAT_FAILED
FAILED_SUMMARY=()

# Ensure binary exists
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: Executable $EXECUTABLE not found. Run 'make' first.${RESET}"
    exit 1
fi

# =============================================================================
# SECTION 1: DYNAMIC TEST FILE CREATION
# =============================================================================
./make_test_files.sh

# =============================================================================
# SECTION 2: TEST RUNNER LOGIC & GROUPING HELPERS
# =============================================================================
start_category() {
    CURRENT_CATEGORY="$1"
    CAT_PASSED["$CURRENT_CATEGORY"]=0
    CAT_FAILED["$CURRENT_CATEGORY"]=0
    echo -e "\n${BLUE}=====================================================${RESET}"
    echo -e "${BLUE} CATEGORY: ${CURRENT_CATEGORY}${RESET}"
    echo -e "${BLUE}=====================================================${RESET}"
}

run_test() {
    local test_name="$1"
    local pattern="$2"
    local input="$3"
    local expected_keyword="$4"

    echo -n "Running Test: [$test_name] ... "

    # Execute tool and capture stdout/stderr
    output=$($EXECUTABLE "$pattern" "$input" 2>&1)

    # Evaluate test result
    local status="FAIL"
    if [ -z "$expected_keyword" ]; then
        if [ -z "$output" ] || echo "$output" | grep -q "No match found"; then
            status="PASS"
        fi
    else
        if echo "$output" | grep -q "$expected_keyword"; then
            status="PASS"
        fi
    fi

    if [ "$status" == "PASS" ]; then
        echo -e "${GREEN}PASS${RESET}"
        ((PASSED++))
        CAT_PASSED["$CURRENT_CATEGORY"]=$(( ${CAT_PASSED["$CURRENT_CATEGORY"]} + 1 ))
    else
        echo -e "${RED}FAIL${RESET}"
        echo -e "   ${YELLOW}Pattern:${RESET}  '$pattern'"
        echo -e "   ${YELLOW}Target:${RESET}   '$input'"
        echo -e "   ${YELLOW}Expected:${RESET} '$expected_keyword'"
        echo -e "   ${YELLOW}Got:${RESET}      '$output'"
        
        # Flatten output newlines for the single-line summary
        local flat_output="${output//$'\n'/\\n}"
        FAILED_SUMMARY+=("[$CURRENT_CATEGORY] $test_name | CMD: $EXECUTABLE '$pattern' '$input' | EXP: '$expected_keyword' | OUT: '$flat_output'")
        
        ((FAILED++))
        CAT_FAILED["$CURRENT_CATEGORY"]=$(( ${CAT_FAILED["$CURRENT_CATEGORY"]} + 1 ))
    fi
}

run_cli_test() {
    local test_name="$1"
    local expected_keyword="$2"
    shift 2
    local args=("$@")

    echo -n "Running Test: [$test_name] ... "

    output=$($EXECUTABLE "${args[@]}" 2>&1)

    local status="FAIL"
    if [ -z "$expected_keyword" ]; then
        if [ -z "$output" ]; then
            status="PASS"
        fi
    else
        if echo "$output" | grep -q -e "$expected_keyword"; then
            status="PASS"
        fi
    fi

    if [ "$status" == "PASS" ]; then
        echo -e "${GREEN}PASS${RESET}"
        ((PASSED++))
        CAT_PASSED["$CURRENT_CATEGORY"]=$(( ${CAT_PASSED["$CURRENT_CATEGORY"]} + 1 ))
    else
        echo -e "${RED}FAIL${RESET}"
        echo -e "   ${YELLOW}Command:${RESET}  $EXECUTABLE ${args[*]}"
        echo -e "   ${YELLOW}Expected:${RESET} '$expected_keyword'"
        echo -e "   ${YELLOW}Got:${RESET}      '$output'"
        
        # Flatten output newlines for the single-line summary
        local flat_output="${output//$'\n'/\\n}"
        FAILED_SUMMARY+=("[$CURRENT_CATEGORY] $test_name | CMD: $EXECUTABLE ${args[*]} | EXP: '$expected_keyword' | OUT: '$flat_output'")
        
        ((FAILED++))
        CAT_FAILED["$CURRENT_CATEGORY"]=$(( ${CAT_FAILED["$CURRENT_CATEGORY"]} + 1 ))
    fi
}

print_summary() {
    echo -e "\n========================================="
    echo -e "         CATEGORY BREAKDOWN SUMMARY      "
    echo -e "========================================="

    for cat in "${!CAT_PASSED[@]}"; do
        p=${CAT_PASSED[$cat]}
        f=${CAT_FAILED[$cat]}
        if [ $f -eq 0 ]; then
            echo -e " - ${cat}: ${GREEN}${p} Passed${RESET}, ${f} Failed"
        else
            echo -e " - ${cat}: ${GREEN}${p} Passed${RESET}, ${RED}${f} Failed${RESET}"
        fi
    done

    echo -e "========================================="
    echo -e "TOTAL RESULTS: ${GREEN}${PASSED} Passed${RESET}, ${RED}${FAILED} Failed${RESET}"
    echo -e "========================================="

    if [ $FAILED -ne 0 ]; then
        echo -e "\n${RED}FAILED TESTS LIST:${RESET}"
        for item in "${FAILED_SUMMARY[@]}"; do
            echo -e "${RED}x${RESET} $item"
        done
        exit 1
    fi
}

echo "========================================="
echo "Running Regex Tool Comprehensive Test Suite"
echo "========================================="

# =============================================================================
# SECTION 3: DIRECT STRING TESTS
# =============================================================================

start_category "String - Exact Literals & Concatenation"
run_test "Literal Exact Match" "hello" "say hello world" "hello"
run_test "Literal Match At Start" "start" "start of line" "start"
run_test "Literal Match At End" "end" "at the end" "end"
run_test "Literal Case Sensitivity Fail" "Hello" "hello world" ""
run_test "Literal Non-Match" "missing" "hello world" ""

start_category "String - Wildcard Dot (.)"
run_test "Dot Wildcard Single" "h.llo" "hello" "hello"
run_test "Dot Wildcard Symbol Match" "a.c" "a!c" "a!c"
run_test "Dot Wildcard Space Match" "a.c" "a c" "a c"
run_test "Dot Wildcard Multiple" "c...t" "count" "count"
run_test "Dot Wildcard Insufficient Length" "c...t" "cat" ""

start_category "String - Character Classes ([...]) & Ranges"
run_test "Char Class Explicit Set" "[aeiou]" "apple" "a"
run_test "Char Class Set Non-Match" "[aeiou]" "rhythm" ""
run_test "Char Class Lowercase Range" "[a-z]+" "123abc456" "abc"
run_test "Char Class Uppercase Range" "[A-Z]+" "helloWORLD" "WORLD"
run_test "Char Class Digit Range" "[0-9]+" "room 404" "404"
run_test "Char Class Multi-Range Alphanumeric" "[a-zA-Z0-9]+" "===Code123===" "Code123"
run_test "Char Class Range with Underscore" "[a-z_]+" "user_name_1" "user_name_"

start_category "String - Negated Character Classes ([^...])"
run_test "Negated Set Match First Consonant" "[^aeiou]+" "apple" "ppl"
run_test "Negated Digit Range" "[^0-9]+" "123abc456" "abc"
run_test "Negated Set Complete Fail" "[^a-z]" "abc" ""
run_test "Negated Set Matching Whitespace/Symbols" "[^a-zA-Z0-9]+" "hello! world" "!"

start_category "String - Shorthand Character Classes (\\d, \\w, \\s)"
run_test "Shorthand \\d (Digit)" "\\d+" "order number 9942" "9942"
run_test "Shorthand \\d Non-Match" "\\d+" "no digits here" ""
run_test "Shorthand \\D (Non-Digit)" "\\D+" "123abc456" "abc"
run_test "Shorthand \\w (Word Char)" "\\w+" "!!var_123!!" "var_123"
run_test "Shorthand \\W (Non-Word Char)" "\\W+" "abc!!!def" "!!!"
run_test "Shorthand \\s (Whitespace)" "a\\sb" "a b" "a b"
run_test "Shorthand \\S (Non-Whitespace)" "\\S+" "   hello   " "hello"

start_category "String - Escaped Metacharacters"
run_test "Escaped Star" "a\\*b" "a*b" "a*b"
run_test "Escaped Plus" "a\\+b" "a+b" "a+b"
run_test "Escaped Question Mark" "a\\?b" "a?b" "a?b"
run_test "Escaped Dot" "a\\.b" "a.b" "a.b"
run_test "Escaped Backslash" "a\\\\b" "a\\b" "a\\b"
run_test "Escaped Parentheses" "\\(abc\\)" "(abc)" "(abc)"
run_test "Escaped Brackets" "\\[abc\\]" "[abc]" "[abc]"

start_category "String - Quantifiers (*, +, ?)"
run_test "Star 0 Occurrences" "ab*c" "ac" "ac"
run_test "Star 1 Occurrence" "ab*c" "abc" "abc"
run_test "Star Multiple Occurrences" "ab*c" "abbbc" "abbbc"
run_test "Plus 0 Occurrences Fail" "ab+c" "ac" ""
run_test "Plus 1 Occurrence" "ab+c" "abc" "abc"
run_test "Plus Multiple Occurrences" "ab+c" "abbbc" "abbbc"
run_test "Question 0 Occurrences" "colou?r" "color" "color"
run_test "Question 1 Occurrence" "colou?r" "colour" "colour"
run_test "Question Excess Fail" "colou?r" "colouur" ""

start_category "String - Alternation / Union (|)"
run_test "Alternation First Branch" "cat|dog" "I have a cat" "cat"
run_test "Alternation Second Branch" "cat|dog" "I have a dog" "dog"
run_test "Alternation Neither Match" "cat|dog" "I have a fish" ""
run_test "Multiple Alternations" "red|green|blue" "sky is blue" "blue"

start_category "String - Grouping & Precedence ()"
run_test "Grouped Plus Repeated Sequence" "(ha)+" "hahaha" "hahaha"
run_test "Grouped Plus Non-Match" "(ha)+" "hohoho" ""
run_test "Grouped Alternation Choice 1" "gr(a|e)y" "gray color" "gray"
run_test "Grouped Alternation Choice 2" "gr(a|e)y" "grey color" "grey"
run_test "Nested Grouping with Quantifier" "((ab)|(cd))+" "ababcdab" "ababcdab"
run_test "Precedence Check: ab* vs (ab)*" "(ab)*" "ababa" "abab"

start_category "String - Composite Patterns"
run_test "Email Address Structure" "\\w+@\\w+\\.\\w+" "contact info@test.com today" "info@test.com"
run_test "IPv4 Address Format" "\\d+\\.\\d+\\.\\d+\\.\\d+" "ip is 192.168.1.1 online" "192.168.1.1"
run_test "C Identifier Syntax" "[a-zA-Z_]\\w*" "int _myVar1 = 5;" "_myVar1"
run_test "Floating Point Number" "\\d+\\.\\d+" "price is 19.99 dollars" "19.99"
run_test "HTML Tag Format" "<[a-z]+>" "content <div> inside" "<div>"

start_category "CLI Parser & Flags"

# 1. Basic Flag Tests
run_cli_test "Case Insensitive Literal" "HeLlO" "-i" "hello" "HeLlO WoRlD"
run_cli_test "Case Insensitive Class Range" "ABC" "-i" "[a-z]+" "123ABC456"

# 2. Delimiter (--) Tests
# Without --, "-pattern" would throw an "Unknown flag: -p" error
run_cli_test "Double Dash Protects Pattern" "-pattern" "--" "-pattern" "this-is-a-pattern-test"

# 3. Target Starting with Hyphen
# Because flags come first, the parser knows "-target" is positional
run_cli_test "Target Starts With Hyphen" "Code123" "[a-zA-Z0-9]+" "---Code123---"

# 4. Combined Delimiter and Flags
run_cli_test "Flag and Double Dash" "-HeLlO" "-i" "--" "-hello" "test--HeLlO--test"

# 5. Multiple Targets
# Passing two direct strings to ensure the loop processes both
run_cli_test "Multiple Targets Processing" "match2" "match\d" "match1" "match2"

# =============================================================================
# SECTION 4: FILE READING TESTS
# =============================================================================

start_category "File - Single Line Inputs"
run_test "Single Line File - Match" "fox" "tests/test_single_line.txt" "Line 1"
run_test "Single Line File - Digit Match" "\\d+" "tests/test_single_line.txt" "123"
run_test "Single Line File - No Match" "cat" "tests/test_single_line.txt" ""

start_category "File - Multi-Line Inputs"
run_test "Multi-Line File - Single Line Match" "port \\d+" "tests/test_multi_line.txt" "Line 2"
run_test "Multi-Line File - First Match Occurrence" "ERROR" "tests/test_multi_line.txt" "Line 2"
run_test "Multi-Line File - Second Match Occurrence" "ERROR" "tests/test_multi_line.txt" "Line 4"
run_test "Multi-Line File - Union Match" "ERROR|INFO" "tests/test_multi_line.txt" "Line 3"
run_test "Multi-Line File - No Match" "CRITICAL" "tests/test_multi_line.txt" ""

start_category "File - Structured Data Logs"
run_test "Log File - IP Search (First)" "\\d+\\.\\d+\\.\\d+\\.\\d+" "tests/test_logs.txt" "192.168.1.50"
run_test "Log File - IP Search (Second)" "\\d+\\.\\d+\\.\\d+\\.\\d+" "tests/test_logs.txt" "10.0.0.1"
run_test "Log File - Log Level Match" "\\[(INFO|WARN)\\]" "tests/test_logs.txt" "Line 1"

start_category "File - Stress & Buffer Limits"
run_test "Long Line Target Extraction" "STRESS_TARGET_MATCH" "tests/test_long_line.txt" "STRESS_TARGET_MATCH"
run_test "Long Line Regex Pattern Matching" "a+STRESS_TARGET_MATCHb+" "tests/test_long_line.txt" "Line 1"

start_category "File - Whitespace & Empty Lines"
run_test "Leading Tab Matching" "\\tline" "tests/test_whitespace.txt" "Line 4"
run_test "Trailing Whitespace Matching" "space\\s+" "tests/test_whitespace.txt" "Line 5"
run_test "Blank Line Non-Match" "text" "tests/test_whitespace.txt" ""
run_test "Empty File Search" ".*" "tests/test_empty.txt" ""

start_category "File - Symbol-Heavy Configurations"
run_test "Ini Section Bracket Match" "\\[database\\]" "tests/test_config.ini" "Line 1"
run_test "Escaped Password Symbols" "\"P@ssw0rd!#\\d+\"" "tests/test_config.ini" "Line 3"
run_test "Inline Comment Match" "#.*" "tests/test_config.ini" "Line 4"

start_category "File - Multi-Occurrence & Overlaps"
run_test "First Occurrence Matching" "cat" "tests/test_multiple_matches.txt" "Line 1"
run_test "Middle Occurrence Matching" "dog" "tests/test_multiple_matches.txt" "dog"

start_category "File - EOF & Formatting Edge Cases"
run_test "Standard Line in EOF File" "first line" "tests/test_no_eof_newline.txt" "Line 1"
run_test "Missing Newline at EOF Match" "second line" "tests/test_no_eof_newline.txt" "Line 2"

# =============================================================================
# SECTION 5: ADVANCED REGEX SYNTAX
# =============================================================================

start_category "String - Anchors (^, $)"
run_test "Start of String Match" "^hello" "hello world" "hello"
run_test "Start of String Non-Match" "^world" "hello world" ""
run_test "End of String Match" "world$" "hello world" "world"
run_test "End of String Non-Match" "hello$" "hello world" ""
run_test "Exact String Match (Both Anchors)" "^hello$" "hello" "hello"
run_test "Exact String Non-Match" "^hello$" "hello world" ""

start_category "String - Word Boundaries (\b, \B)"
run_test "Word Boundary Match (Start)" "\\bcat" "the cat sat" "cat"
run_test "Word Boundary Match (End)" "cat\\b" "tomcat" "cat"
run_test "Word Boundary Exact Word" "\\bcat\\b" "the cat sat" "cat"
run_test "Word Boundary Non-Match" "\\bcat\\b" "the tomcat sat" ""
run_test "Non-Word Boundary Match" "tom\\B" "tomcat" "tom"
run_test "Non-Word Boundary Non-Match" "tom\\B" "tom cat" ""

start_category "String - Specific Quantifiers ({n}, {n,m})"
run_test "Exact Repetition {n}" "a{3}" "baaac" "aaa"
run_test "Exact Repetition Non-Match" "a{3}" "baac" ""
run_test "Min Repetition {n,}" "a{2,}" "baaaac" "aaaa"
run_test "Min Repetition Non-Match" "a{2,}" "bac" ""
run_test "Range Repetition {n,m}" "a{2,3}" "baaaac" "aaa"

start_category "String - Lazy / Ungreedy Quantifiers (*?, +?, ??)"
run_test "Lazy Star *?" "<.*?>" "<a>text</a>" "<a>"
run_test "Greedy Star (Control)" "<.*>" "<a>text</a>" "<a>text</a>"
run_test "Lazy Plus +?" "a.+?c" "abcbcdc" "abc"
run_test "Lazy Question ??" "a??" "aa" "" # Should match empty string if supported

start_category "String - Groups, Lookarounds & Backreferences"
run_test "Non-Capturing Group" "(?:foo)bar" "foobar" "foobar"
run_test "Backreference (Same Word Twice)" "\\b(\\w+)\\s+\\1\\b" "cat cat" "cat cat"
run_test "Backreference Non-Match" "\\b(\\w+)\\s+\\1\\b" "cat dog" ""
run_test "Positive Lookahead" "foo(?=bar)" "foobar" "foo"
run_test "Positive Lookahead Non-Match" "foo(?=bar)" "foobaz" ""
run_test "Negative Lookahead" "foo(?!bar)" "foobaz" "foo"
run_test "Positive Lookbehind" "(?<=foo)bar" "foobar" "bar"
run_test "Negative Lookbehind" "(?<!foo)bar" "bazbar" "bar"

start_category "String - Hex & Control Characters"
run_test "Newline Match" "hello\\nworld" "hello
world" "world"
run_test "Hexadecimal Match" "\\x61\\x62\\x63" "abc" "abc"
run_test "Carriage Return / Tab Match" "\\r\\t" "$(printf '\r\t')" "" # Validates it doesn't crash, grep testing this directly is tricky

# =============================================================================
# SECTION 6: CLI FLAGS (-v, -c, -n) & COMBINATIONS
# =============================================================================

start_category "CLI Flags - Single Flags"

run_cli_test "Count Matches (String)" "2" "-c" "cat" "cat dog cat"
run_cli_test "Count Matches (File)" "2" "-c" "line" "tests/test_lines.txt"

start_category "CLI Flags - Combinations"
run_cli_test "Count + Case Insensitive (-ci)" "2" "-ci" "cat" "tests/test_lines.txt"

# =============================================================================
# SECTION 7: MULTI-INPUT HANDLING (Strings & Files)
# =============================================================================

start_category "Multi-Input - Strings"
# Passing 4 distinct strings to the tool. It should process all of them.
run_cli_test "Multiple String Inputs Match 1" "apple" "a.*" "apple" "banana" "carrot" "date"
run_cli_test "Multiple String Inputs Match 2" "carrot" "c.*" "apple" "banana" "carrot" "date"

start_category "Multi-Input - Files"
# Passing 2 distinct files. Tool should read both. 
run_cli_test "Multi-File Cross-Match A" "banana" "banana" "tests/test_multi_a.txt" "tests/test_multi_b.txt"
run_cli_test "Multi-File Cross-Match B" "fig" "fig" "tests/test_multi_a.txt" "tests/test_multi_b.txt"

start_category "Multi-Input - Files + Flags"
run_cli_test "Multi-File + Count" "2" "-c" "^[af]" "tests/test_multi_a.txt" "tests/test_multi_b.txt" # Matches apple (a) and fig (b)

# Output final results summary
print_summary