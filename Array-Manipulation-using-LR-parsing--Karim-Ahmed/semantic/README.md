# Semantic Analysis Phase

## Overview
The semantic analysis phase performs type checking, validates semantic rules, and annotates the AST with type information.

## Files
- **ASTNode.h** - Enhanced AST node structure with type annotations
- **ast_builder.h/cpp** - Loads AST from JSON
- **symbol_table.h/cpp** - Enhanced symbol table with type and dimension tracking
- **semantic_analyzer.h/cpp** - Main semantic analysis engine
- **semantic_output.h/cpp** - Outputs annotated AST and symbol table
- **semantic_main.cpp** - Entry point for semantic analysis

## Functionality
- Type checking for all expressions and statements
- Variable declaration validation (duplicate detection)
- Undeclared variable detection
- Type compatibility checking in assignments
- Array dimension validation
- Array index type validation (must be integer)
- Type widening: `int` → `float` → `double`

## Type System
- Supported types: `int`, `float`, `double`, `char`, `string`
- Tracks array dimensions (1D and 2D)
- Validates array literal dimensions match declarations

## Semantic Errors Detected
- Undeclared variables
- Duplicate declarations
- Type mismatches in assignments
- Type mismatches in expressions
- Invalid array access (using arrays without indices)
- Non-integer array indices
- Array dimension mismatches

## Input/Output
- **Input**: `ast.json` (from parser phase)
- **Output**: 
  - `annotated_ast.json` - AST with type annotations
  - `symbol_table.json` - Complete symbol table

## Usage
```bash
./semantic_main [ast_file] [output_dir]
```
