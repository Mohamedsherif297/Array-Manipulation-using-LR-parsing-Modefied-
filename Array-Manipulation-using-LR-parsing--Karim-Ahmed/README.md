# Array-Manipulation-using-LR-parsing-
This project involves developing a C++ application designed to parse and execute array manipulation commands. It serves as a practical implementation of the Compiler Design Phases discussed in the course, specifically focusing on syntax and semantic analysis.

## Project Structure

```
Array-Manipulation-using-LR-parsing--Karim-Ahmed/
├── lexer/              # Phase 1: Lexical Analysis
│   ├── Token_Class.h
│   ├── Symbol_Table.h
│   ├── Lexer.h
│   └── README.md
│
├── parser/             # Phase 2: Syntax Analysis (LR(1) Parser)
│   ├── Grammar.h
│   ├── Item.h
│   ├── Item.cpp
│   ├── First_Set.h
│   ├── Parser_States.cpp
│   ├── Parsing_Table.cpp
│   ├── Parser.h
│   └── README.md
│
├── semantic/           # Phase 3: Semantic Analysis
│   ├── ASTNode.h
│   ├── ast_builder.h/cpp
│   ├── symbol_table.h/cpp
│   ├── semantic_analyzer.h/cpp
│   ├── semantic_output.h/cpp
│   ├── semantic_main.cpp
│   └── README.md
│
├── codegen/            # Phase 4: Code Generation (TAC)
│   ├── codegen_types.h
│   ├── ast_loader.h/cpp
│   ├── symbol_table_loader.h/cpp
│   ├── code_generator.h/cpp
│   ├── codegen_main.cpp
│   ├── Makefile
│   └── README.md
│
├── Main.cpp            # Main entry point (integrates all phases)
├── CFG.md              # Context-Free Grammar documentation
└── To_do_list.md       # Project progress tracker
```

## Compilation Pipeline

```
Source Code
    ↓
[Lexer] → Tokens
    ↓
[Parser] → AST (ast.json)
    ↓
[Semantic Analyzer] → Annotated AST + Symbol Table
    ↓
[Code Generator] → Three-Address Code (IR)
```

## Quick Start

### Compile and Run Main Program
```bash
g++ -std=c++17 Main.cpp -o Main
./Main
```

### Run Individual Phases

**Semantic Analysis:**
```bash
cd semantic
g++ -std=c++17 semantic_main.cpp ast_builder.cpp semantic_analyzer.cpp semantic_output.cpp symbol_table.cpp -o semantic_main
./semantic_main ../ast.json .
```

**Code Generation:**
```bash
cd codegen
make
./codegen ../semantic/annotated_ast.json ../semantic/symbol_table.json ir.txt
```

## Features

- ✅ Full LR(1) parser with canonical collection
- ✅ FIRST set computation
- ✅ Closure and GOTO operations
- ✅ Parsing table construction with conflict detection
- ✅ AST generation
- ✅ Semantic analysis with type checking
- ✅ Symbol table management
- ✅ Intermediate code generation (TAC)
- ✅ Support for 1D and 2D arrays
- ✅ Expression evaluation with operator precedence

## Supported Language Features

- Variable declarations: `int x;`, `float arr[5];`
- Array declarations: `int matrix[2][3];`
- Assignments: `x = 5;`, `arr[0] = 10;`
- Declaration with initialization: `int x = 5;`
- Array initialization: `int arr[2] = {1, 2};`
- 2D array initialization: `int m[2][2] = {{1,2},{3,4}};`
- Arithmetic expressions: `x = a + b * c;`
- Array element access: `arr[i]`, `matrix[i][j]`

