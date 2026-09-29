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
mkdir -p tests

# 1. Standard Single Line Test File
cat << 'EOF' > tests/test_single_line.txt
The quick brown fox jumps over 123 lazy dogs.
EOF

# 2. Standard Multi-Line Test File
cat << 'EOF' > tests/test_multi_line.txt
Header line: system initialization
ERROR: Database connection failed on port 5432
INFO: Retrying connection...
ERROR: Timeout reached after 3000ms
Footer line: process terminated
EOF

# 3. Empty Test File
touch tests/test_empty.txt

# 4. Structured Log File
cat << 'EOF' > tests/test_logs.txt
2026-09-29 10:00:01 [INFO] User logged in from 192.168.1.50
2026-09-29 10:02:15 [WARN] High memory usage: 88%
2026-09-29 10:05:30 [INFO] User logged out from 10.0.0.1
EOF

# 5. Long Line Stress Test File (10,000 'a's + target + 10,000 'b's)
LONG_PREFIX=$(printf 'a%.0s' {1..10000})
LONG_SUFFIX=$(printf 'b%.0s' {1..10000})
echo "${LONG_PREFIX}STRESS_TARGET_MATCH${LONG_SUFFIX}" > tests/test_long_line.txt

# 6. Whitespace & Blank Lines File
cat << 'EOF' > tests/test_whitespace.txt


   
	line with leading tab
trailing space   

EOF

# 7. Symbol-Heavy Configuration File
cat << 'EOF' > tests/test_config.ini
[database]
server_host=127.0.0.1:8080
db_pass="P@ssw0rd!#123"
enabled=true # main flag
EOF

# 8. Multiple Matches / Overlaps File
cat << 'EOF' > tests/test_multiple_matches.txt
cat dog cat bird cat
EOF

# 9. Missing Newline at EOF File
printf "first line\nsecond line without newline" > tests/test_no_eof_newline.txt


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
        ((FAILED++))
        CAT_FAILED["$CURRENT_CATEGORY"]=$(( ${CAT_FAILED["$CURRENT_CATEGORY"]} + 1 ))
        FAILED_SUMMARY+=("[$CURRENT_CATEGORY] $test_name (Pattern: '$pattern')")
    fi
}

print_summary() {
    echo -e "\n========================================="
    echo -e "         CATEGORY BREAKDOWN SUMMARY       "
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
            echo -e " ${RED}x${RESET} $item"
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
run_test "Literal Case Sensitivity Fail" "Hello" "hello world" "No match found"
run_test "Literal Non-Match" "missing" "hello world" "No match found"

start_category "String - Wildcard Dot (.)"
run_test "Dot Wildcard Single" "h.llo" "hello" "hello"
run_test "Dot Wildcard Symbol Match" "a.c" "a!c" "a!c"
run_test "Dot Wildcard Space Match" "a.c" "a c" "a c"
run_test "Dot Wildcard Multiple" "c...t" "count" "count"
run_test "Dot Wildcard Insufficient Length" "c...t" "cat" "No match found"

start_category "String - Character Classes ([...]) & Ranges"
run_test "Char Class Explicit Set" "[aeiou]" "apple" "a"
run_test "Char Class Set Non-Match" "[aeiou]" "rhythm" "No match found"
run_test "Char Class Lowercase Range" "[a-z]+" "123abc456" "abc"
run_test "Char Class Uppercase Range" "[A-Z]+" "helloWORLD" "WORLD"
run_test "Char Class Digit Range" "[0-9]+" "room 404" "404"
run_test "Char Class Multi-Range Alphanumeric" "[a-zA-Z0-9]+" "---Code123---" "Code123"
run_test "Char Class Range with Underscore" "[a-z_]+" "user_name_1" "user_name_"

start_category "String - Negated Character Classes ([^...])"
run_test "Negated Set Match First Consonant" "[^aeiou]+" "apple" "ppl"
run_test "Negated Digit Range" "[^0-9]+" "123abc456" "abc"
run_test "Negated Set Complete Fail" "[^a-z]" "abc" "No match found"
run_test "Negated Set Matching Whitespace/Symbols" "[^a-zA-Z0-9]+" "hello! world" "!"

start_category "String - Shorthand Character Classes (\\d, \\w, \\s)"
run_test "Shorthand \\d (Digit)" "\\d+" "order number 9942" "9942"
run_test "Shorthand \\d Non-Match" "\\d+" "no digits here" "No match found"
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
run_test "Plus 0 Occurrences Fail" "ab+c" "ac" "No match found"
run_test "Plus 1 Occurrence" "ab+c" "abc" "abc"
run_test "Plus Multiple Occurrences" "ab+c" "abbbc" "abbbc"
run_test "Question 0 Occurrences" "colou?r" "color" "color"
run_test "Question 1 Occurrence" "colou?r" "colour" "colour"
run_test "Question Excess Fail" "colou?r" "colouur" "No match found"

start_category "String - Alternation / Union (|)"
run_test "Alternation First Branch" "cat|dog" "I have a cat" "cat"
run_test "Alternation Second Branch" "cat|dog" "I have a dog" "dog"
run_test "Alternation Neither Match" "cat|dog" "I have a fish" "No match found"
run_test "Multiple Alternations" "red|green|blue" "sky is blue" "blue"

start_category "String - Grouping & Precedence ()"
run_test "Grouped Plus Repeated Sequence" "(ha)+" "hahaha" "hahaha"
run_test "Grouped Plus Non-Match" "(ha)+" "hohoho" "No match found"
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
run_test "Blank Line Non-Match" "text" "tests/test_whitespace.txt" "No match found"
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

# Output final results summary
print_summary