# Project Architecture

## Directory Structure

```
Array-Manipulation-using-LR-parsing--Karim-Ahmed/
│
├── 📁 lexer/                    # PHASE 1: Lexical Analysis
│   ├── Token_Class.h            # Token type definitions
│   ├── Symbol_Table.h           # Basic symbol table
│   └── Lexer.h                  # Tokenizer implementation
│
├── 📁 parser/                   # PHASE 2: Syntax Analysis
│   ├── Grammar.h                # CFG with 40 productions
│   ├── Item.h                   # LR(1) item structure
│   ├── Item.cpp                 # Closure & GOTO operations
│   ├── First_Set.h              # FIRST set computation
│   ├── Parser_States.cpp        # State machine builder
│   ├── Parsing_Table.cpp        # ACTION/GOTO tables
│   └── Parser.h                 # Main parser + AST builder
│
├── 📁 semantic/                 # PHASE 3: Semantic Analysis
│   ├── ASTNode.h                # AST node with type info
│   ├── ast_builder.h/cpp        # AST loader from JSON
│   ├── symbol_table.h/cpp       # Enhanced symbol table
│   ├── semantic_analyzer.h/cpp  # Type checker
│   ├── semantic_output.h/cpp    # Output writer
│   ├── semantic_main.cpp        # Standalone entry point
│   ├── annotated_ast.json       # Output: typed AST
│   └── symbol_table.json        # Output: symbol table
│
├── 📁 codegen/                  # PHASE 4: Code Generation
│   ├── codegen_types.h          # TAC quadruple definitions
│   ├── ast_loader.h/cpp         # Load annotated AST
│   ├── symbol_table_loader.h/cpp # Load symbol table
│   ├── code_generator.h/cpp     # TAC generator
│   ├── codegen_main.cpp         # Standalone entry point
│   ├── Makefile                 # Build configuration
│   ├── build.sh                 # Build script
│   └── ir.txt                   # Output: Three-Address Code
│
├── 📁 optimizer/                # PHASE 5: Code Optimization
│   ├── optimizer_types.h        # Optimization data structures
│   ├── ir_loader.h/cpp          # Load/write IR
│   ├── optimizer.h/cpp          # 5 optimization techniques
│   ├── optimizer_main.cpp       # Standalone entry point
│   ├── Makefile                 # Build configuration
│   └── optimized_ir.txt         # Output: Optimized TAC
│
├── Main.cpp                     # 🚀 Integrated pipeline (All 5 phases)
├── ast.json                     # Output: Raw AST from parser
├── CFG.md                       # Grammar documentation
├── To_do_list.md                # Progress tracker
├── README.md                    # Main documentation
└── ARCHITECTURE.md              # This file
```

## Data Flow

```
┌─────────────────────────────────────────────────────────────────┐
│                         SOURCE CODE                              │
│                   "int x[2][2] = {{1,2},{3,4}};"                │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 1: LEXER (lexer/)                                        │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ • Tokenization                                            │  │
│  │ • Comment removal                                         │  │
│  │ • Basic symbol table                                      │  │
│  └──────────────────────────────────────────────────────────┘  │
└────────────────────────────┬────────────────────────────────────┘
                             │ Tokens
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 2: PARSER (parser/)                                      │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ • FIRST set computation                                   │  │
│  │ • LR(1) state machine construction                        │  │
│  │ • ACTION/GOTO table generation                            │  │
│  │ • Shift-reduce parsing                                    │  │
│  │ • AST construction                                        │  │
│  └──────────────────────────────────────────────────────────┘  │
└────────────────────────────┬────────────────────────────────────┘
                             │ ast.json
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 3: SEMANTIC ANALYZER (semantic/)                         │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ • Type checking                                           │  │
│  │ • Declaration validation                                  │  │
│  │ • Symbol table enhancement                                │  │
│  │ • AST annotation with types                               │  │
│  │ • Error detection & reporting                             │  │
│  └──────────────────────────────────────────────────────────┘  │
└────────────────────────────┬────────────────────────────────────┘
                             │ annotated_ast.json + symbol_table.json
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 4: CODE GENERATOR (codegen/)                             │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ • Three-Address Code generation                           │  │
│  │ • Temporary variable management                           │  │
│  │ • Array offset calculation                                │  │
│  │ • Expression evaluation                                   │  │
│  └──────────────────────────────────────────────────────────┘  │
└────────────────────────────┬────────────────────────────────────┘
                             │ ir.txt
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    THREE-ADDRESS CODE (TAC)                      │
│                                                                  │
│  // DECL x  type=int                                            │
│  t1 = 0 * 4                                                     │
│  x[t1] = 1                                                      │
│  t2 = 1 * 4                                                     │
│  x[t2] = 2                                                      │
│  ...                                                            │
└─────────────────────────────────────────────────────────────────┘
```

## Module Dependencies

```
Main.cpp
  ├─→ lexer/Lexer.h
  │     ├─→ lexer/Token_Class.h
  │     └─→ lexer/Symbol_Table.h
  │
  └─→ parser/Parser.h
        ├─→ parser/Parsing_Table.cpp
        │     ├─→ parser/Item.h
        │     ├─→ parser/Parser_States.cpp
        │     │     └─→ parser/Item.cpp
        │     │           ├─→ parser/First_Set.h
        │     │           │     └─→ parser/Grammar.h
        │     │           └─→ parser/Grammar.h
        │     └─→ parser/Grammar.h
        └─→ parser/Grammar.h

semantic/semantic_main.cpp
  ├─→ semantic/ast_builder.h
  ├─→ semantic/symbol_table.h
  ├─→ semantic/semantic_analyzer.h
  │     ├─→ semantic/ASTNode.h
  │     └─→ semantic/symbol_table.h
  └─→ semantic/semantic_output.h

codegen/codegen_main.cpp
  ├─→ codegen/ast_loader.h
  │     └─→ semantic/ASTNode.h
  ├─→ codegen/symbol_table_loader.h
  └─→ codegen/code_generator.h
        ├─→ codegen/codegen_types.h
        ├─→ codegen/symbol_table_loader.h
        └─→ semantic/ASTNode.h
```

## Execution Modes

### Mode 1: Integrated Pipeline (Phases 1-2)
```bash
./Main
```
- Runs lexer and parser
- Outputs: `ast.json`

### Mode 2: Full Pipeline (All Phases)
```bash
# Phase 1-2: Lexing + Parsing
./Main

# Phase 3: Semantic Analysis
cd semantic
./semantic_main ../ast.json .

# Phase 4: Code Generation
cd ../codegen
./codegen ../semantic/annotated_ast.json ../semantic/symbol_table.json ir.txt
```

### Mode 3: Individual Phase Testing
Each phase can be compiled and tested independently using its own entry point.

## Key Design Decisions

1. **Modular Architecture**: Each compiler phase is isolated in its own folder
2. **JSON Intermediate Format**: Phases communicate via JSON files for debugging
3. **Standalone Executables**: Each phase can run independently
4. **Shared Data Structures**: ASTNode is shared between semantic and codegen phases
5. **Header-Only Implementation**: Most code is in headers for simplicity
