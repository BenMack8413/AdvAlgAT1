#!/usr/bin/env bash

# Terminal Colors
GREEN="\033[0;32m"
RED="\033[0;31m"
RESET="\033[0m"

EXECUTABLE="./bin/regex_tool"
TEST_FILE="tests/sample.txt"
PASSED=0
FAILED=0

# Ensure binary exists
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: Executable $EXECUTABLE not found. Run 'make' first.${RESET}"
    exit 1
fi

run_test() {
    local test_name="$1"
    local pattern="$2"
    local input="$3"
    local expected_keyword="$4"

    echo -n "Running Test: $test_name ... "

    # Execute tool and capture stdout
    output=$($EXECUTABLE "$pattern" "$input" 2>&1)

    # Check if expected keyword exists in output
    if echo "$output" | grep -q "$expected_keyword"; then
        echo -e "${GREEN}PASS${RESET}"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${RESET}"
        echo "   Pattern:  '$pattern'"
        echo "   Target:   '$input'"
        echo "   Expected: '$expected_keyword'"
        echo "   Got:      '$output'"
        ((FAILED++))
    fi
}

echo "========================================="
echo "Running Regex Tool Test Suite"
echo "========================================="

# -----------------------------------------------------------------------------
# TEST CATEGORY 1: Direct String Input
# -----------------------------------------------------------------------------
# Test 1.1: Standard String Match
run_test "Direct String Match" "(a|b)*c" "test string with abac inside" "abac"

# Test 1.2: Direct String No Match
run_test "Direct String Non-Match" "xyz" "hello world" "No match found"

# Test 1.3: Quantifier String Match
run_test "Direct String Plus Quantifier" "go+al" "goooal" "goooal"


# -----------------------------------------------------------------------------
# TEST CATEGORY 2: File Reading Input
# -----------------------------------------------------------------------------
# Test 2.1: File Line Matching
run_test "File Line Search (ca+t)" "ca+t" "$TEST_FILE" "Line 2"

# Test 2.2: Multiple File Line Matches
run_test "File Line Search Multiple Matches" "ca+t" "$TEST_FILE" "Line 3"

# Summary
echo "========================================="
echo -e "Results: ${GREEN}${PASSED} Passed${RESET}, ${RED}${FAILED} Failed${RESET}"
echo "========================================="

if [ $FAILED -ne 0 ]; then
    exit 1
fi