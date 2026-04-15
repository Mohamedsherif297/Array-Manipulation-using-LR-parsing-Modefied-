# Code Generation Phase

## Overview
The code generation phase transforms the annotated AST into Three-Address Code (TAC) intermediate representation.

## Files
- **codegen_types.h** - TAC quadruple structure definitions
- **ast_loader.h/cpp** - Loads annotated AST from JSON
- **symbol_table_loader.h/cpp** - Loads symbol table from JSON
- **code_generator.h/cpp** - Main code generation engine
- **codegen_main.cpp** - Entry point for code generation
- **Makefile** - Build configuration
- **build.sh** - Build script

## Three-Address Code (TAC)
Generates intermediate representation in quadruple form: `(op, arg1, arg2, result)`

### Supported Operations
- **ASSIGN** - Simple assignment: `x = y`
- **LOAD** - Array read: `t1 = arr[offset]`
- **STORE** - Array write: `arr[offset] = value`
- **Arithmetic** - `+`, `-`, `*`, `/`
- **DECL** - Declaration annotation

## Features
- Temporary variable generation
- Expression evaluation with operator precedence
- Array offset calculation (1D and 2D)
- Array initialization from literals
- Element size calculation for different types

## Example Output
For `int x[2][2] = {{1,2},{3,4}};`:
```
// DECL x  type=int
t1 = 0 * 4
x[t1] = 1
t2 = 1 * 4
x[t2] = 2
t3 = 2 * 4
x[t3] = 3
t4 = 3 * 4
x[t4] = 4
```

## Input/Output
- **Input**: 
  - `annotated_ast.json` (from semantic phase)
  - `symbol_table.json` (from semantic phase)
- **Output**: 
  - `ir.txt` - Three-Address Code

## Usage
```bash
./codegen [annotated_ast] [symbol_table] [output_ir]
```

## Build
```bash
make
# or
./build.sh
```
