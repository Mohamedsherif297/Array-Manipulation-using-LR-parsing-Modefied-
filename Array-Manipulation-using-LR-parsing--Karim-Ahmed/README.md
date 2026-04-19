# Array Manipulation Compiler - Core Implementation

This directory contains the core compiler implementation with LR parsing for array manipulation.

## Components

- **Main.cpp** - Main compiler pipeline orchestrator
- **lexer/** - Lexical analyzer (tokenization)
- **parser/** - LR(1) parser implementation  
- **semantic/** - Semantic analyzer and type checker
- **codegen/** - Three-address code generator
- **optimizer/** - Code optimization engine

## Compilation

To compile the main executable:
```bash
compile_main_only.bat
```

This creates `Main.exe` which runs the complete compilation pipeline.

## Usage

The compiler can be used standalone:
```bash
Main.exe input_file.txt
```

Or through the web GUI (see main README.md in parent directory).

## Output Files

- `ast.json` - Abstract Syntax Tree
- `semantic/annotated_ast.json` - Semantically analyzed AST  
- `semantic/symbol_table.json` - Symbol table with type information
- `codegen/ir.txt` - Generated three-address code
- `optimizer/optimized_ir.txt` - Optimized intermediate code

## Supported Language Features

- Variable declarations: `int x;`, `float arr[5];`
- Array declarations: `int matrix[2][3];`
- Assignments: `x = 5;`, `arr[0] = 10;`
- Declaration with initialization: `int x = 5;`
- Array initialization: `int arr[2] = {1, 2};`
- 2D array initialization: `int m[2][2] = {{1,2},{3,4}};`
- Arithmetic expressions: `x = a + b * c;`
- Array element access: `arr[i]`, `matrix[i][j]`

