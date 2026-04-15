#!/bin/bash

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  COMPILER TEST RUNNER${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if Main executable exists
if [ ! -f "./Main" ]; then
    echo -e "${YELLOW}⚠️  Main executable not found. Compiling...${NC}"
    g++ -std=c++17 Main.cpp -o Main
    if [ $? -ne 0 ]; then
        echo -e "${RED}❌ Compilation failed!${NC}"
        exit 1
    fi
    echo -e "${GREEN}✅ Compilation successful!${NC}"
    echo ""
fi

# Run the test
echo -e "${BLUE}Running compiler pipeline...${NC}"
echo ""

./Main

exit_code=$?

echo ""
if [ $exit_code -eq 0 ]; then
    echo -e "${GREEN}✅ All phases completed successfully!${NC}"
else
    echo -e "${RED}❌ Test failed with exit code: $exit_code${NC}"
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Generated Files:${NC}"
echo -e "${BLUE}========================================${NC}"

# Check for output files
files=("ast.json" "semantic/annotated_ast.json" "semantic/symbol_table.json" "codegen/ir.txt")

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        size=$(wc -c < "$file" | tr -d ' ')
        echo -e "${GREEN}✅${NC} $file (${size} bytes)"
    else
        echo -e "${RED}❌${NC} $file (not found)"
    fi
done

echo ""
echo -e "${BLUE}To modify test case, edit Main.cpp line 24${NC}"
