# Parser Phase (Syntax Analysis)

## Overview
The parser phase performs syntax analysis using a canonical LR(1) parser to build an Abstract Syntax Tree (AST).

## Files
- **Grammar.h** - Context-Free Grammar (CFG) definition with 40 productions
- **Item.h** - LR(1) item structure definition
- **Item.cpp** - Closure and GOTO operations implementation
- **First_Set.h** - FIRST set computation for predictive parsing
- **Parser_States.cpp** - LR(1) state machine construction (canonical collection)
- **Parsing_Table.cpp** - ACTION and GOTO table generation with conflict detection
- **Parser.h** - Main parser implementation with AST generation

## Key Algorithms
1. **FIRST Set Computation** - Determines which terminals can start a production
2. **Closure Operation** - Expands LR(1) items with lookaheads
3. **GOTO Operation** - Computes state transitions
4. **Canonical Collection** - Builds all LR(1) states
5. **Parsing Table Construction** - Generates ACTION/GOTO tables
6. **Shift-Reduce Parser** - Executes parsing with AST construction

## Grammar Support
- Variable declarations: `int x;`, `float arr[5];`
- Array declarations: `int matrix[2][3];` (1D and 2D)
- Assignments: `x = 5;`, `arr[0] = 10;`
- Declaration with initialization: `int x = 5;`, `int arr[2] = {1, 2};`
- Arithmetic expressions: `x = a + b * c;`
- Array literals: `int m[2][2] = {{1,2},{3,4}};`

## Output
- **ast.json** - Abstract Syntax Tree in JSON format

## Usage
```cpp
#include "parser/Parser.h"

computeFIRST();
buildStates();
buildParsingTable();
Node* root = parse(input);
```
