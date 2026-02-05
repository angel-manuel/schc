#!/bin/bash
# SCHC Test Runner
# Finds .hs files with "-- expect: <output>" comments, compiles and verifies output

SCHC="./schc"
TEST_DIR="inputs"
PASS=0
FAIL=0
SKIP=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

for file in "$TEST_DIR"/*.hs; do
    # Extract expected output from "-- expect: <output>" comment
    expected=$(grep -m1 '^-- expect:' "$file" 2>/dev/null | sed 's/^-- expect: *//' || true)

    if [ -z "$expected" ]; then
        echo -e "${YELLOW}SKIP${NC} $file (no expect comment)"
        ((SKIP++))
        continue
    fi

    # Compile to temp binary
    tmpbin=$(mktemp)
    if ! $SCHC -b "$tmpbin" "$file" > /dev/null 2>&1; then
        echo -e "${RED}FAIL${NC} $file (compilation failed)"
        ((FAIL++))
        rm -f "$tmpbin"
        continue
    fi

    # Run and capture output
    actual=$("$tmpbin" 2>&1 || true)
    rm -f "$tmpbin"

    # Compare
    if [ "$actual" = "$expected" ]; then
        echo -e "${GREEN}PASS${NC} $file"
        ((PASS++))
    else
        echo -e "${RED}FAIL${NC} $file"
        echo "  Expected: $expected"
        echo "  Actual:   $actual"
        ((FAIL++))
    fi
done

echo ""
echo "Results: $PASS passed, $FAIL failed, $SKIP skipped"

if [ $FAIL -gt 0 ]; then
    exit 1
fi
