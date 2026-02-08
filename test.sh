#!/bin/bash

# Test script for program-arguments library
# This script tests all functionality and can be run locally or in CI

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/cmake-build-debug"
EXAMPLE_BIN="${BUILD_DIR}/example"

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

print_error() {
    echo -e "${RED}[FAIL]${NC} $1"
}

# Function to run a test
run_test() {
    local test_name="$1"
    local test_command="$2"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    print_info "Running test: $test_name"

    if eval "$test_command" > /tmp/test_output.log 2>&1; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        print_success "$test_name"
        return 0
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        print_error "$test_name"
        cat /tmp/test_output.log
        return 1
    fi
}

# Function to run a test expecting specific output
run_test_with_output() {
    local test_name="$1"
    local test_command="$2"
    local expected_pattern="$3"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    print_info "Running test: $test_name"

    if eval "$test_command" 2>&1 | grep -q "$expected_pattern"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
        print_success "$test_name"
        return 0
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
        print_error "$test_name (expected pattern not found: '$expected_pattern')"
        return 1
    fi
}

# Print header
echo "========================================"
echo "Program Arguments Library - Test Suite"
echo "========================================"
echo ""

# Check if example binary exists
if [ ! -f "$EXAMPLE_BIN" ]; then
    print_error "Example binary not found at $EXAMPLE_BIN"
    print_info "Please build the project first: cmake --build cmake-build-debug"
    exit 1
fi

print_success "Example binary found"
echo ""

# Run tests
echo "=== Basic Functionality Tests ==="
run_test_with_output "Display help" "$EXAMPLE_BIN --help" "Usage:"
run_test "Valid arguments" "$EXAMPLE_BIN -i input.txt -n 50" 0
run_test_with_output "Missing required" "$EXAMPLE_BIN" "Required argument missing"

echo ""
echo "=== Validation Tests ==="
run_test_with_output "Invalid count" "$EXAMPLE_BIN -i input.txt -n 150" "Count must be between"
run_test_with_output "Invalid threshold" "$EXAMPLE_BIN -i input.txt -t 2.0" "Threshold must be between"
run_test_with_output "Invalid file ext" "$EXAMPLE_BIN -i input.txt -o file.pdf" "must have .txt extension"

echo ""
echo "========================================"
echo "Test Summary"
echo "========================================"
echo "Total tests:  $TOTAL_TESTS"
echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ Some tests failed${NC}"
    exit 1
fi

