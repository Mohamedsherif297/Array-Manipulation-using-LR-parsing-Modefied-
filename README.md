# Array Manipulation Compiler

A complete compiler implementation with **LR(1) parsing**, **semantic analysis**, and **code generation** for array manipulation programs. Includes a modern **React-based GUI** with syntax highlighting and visual output.

## 🎯 Features

- ✅ Full LR(1) parser with canonical collection
- ✅ Lexical analysis with token generation
- ✅ Semantic analysis with type checking
- ✅ Symbol table management
- ✅ Three-Address Code (TAC) generation
- ✅ Support for 1D and 2D arrays
- ✅ **Modern Web GUI** with Monaco Editor
- ✅ **Visual AST representation**
- ✅ **Interactive Symbol Table**
- ✅ **Formatted TAC output**

## 🚀 Quick Start

### Using the GUI (Recommended)

```bash
cd GUI
./setup.sh
npm start
```

Then open **http://localhost:3000** in your browser.

See [GUI/QUICKSTART.md](GUI/QUICKSTART.md) for detailed instructions.

### Using Command Line

```bash
cd Array-Manipulation-using-LR-parsing--Karim-Ahmed
g++ -std=c++17 Main.cpp -o Main
./Main
```

See [Array-Manipulation-using-LR-parsing--Karim-Ahmed/README.md](Array-Manipulation-using-LR-parsing--Karim-Ahmed/README.md) for full CLI documentation.

## 📁 Project Structure

```
.
├── GUI/                                    # React-based Web Interface
│   ├── src/
│   │   ├── components/                    # React components
│   │   │   ├── CodeEditor.tsx            # Monaco editor
│   │   │   ├── ASTVisualizer.tsx         # AST tree view
│   │   │   ├── SymbolTableView.tsx       # Symbol table display
│   │   │   └── TACView.tsx               # TAC output
│   │   └── App.tsx                        # Main app
│   ├── server.js                          # Express API server
│   └── README.md                          # GUI documentation
│
└── Array-Manipulation-using-LR-parsing--Karim-Ahmed/
    ├── lexer/                             # Phase 1: Lexical Analysis
    ├── parser/                            # Phase 2: Syntax Analysis
    ├── semantic/                          # Phase 3: Semantic Analysis
    ├── codegen/                           # Phase 4: Code Generation
    └── Main.cpp                           # CLI entry point
```

## 🎨 GUI Screenshots

The GUI provides:
- **Code Editor**: VS Code-like editor with syntax highlighting
- **AST Visualization**: Interactive tree view of parsed code
- **Symbol Table**: Tabular display of variables and types
- **TAC Output**: Formatted intermediate code representation

## 💻 Supported Syntax

```cpp
// Variable declarations
int x;
float y;

// Array declarations
int arr[5];
int matrix[2][3];

// Assignments
x = 5;
arr[0] = 10;

// Declaration with initialization
int x = 5;
int arr[2] = {1, 2};
int m[2][2] = {{1,2},{3,4}};

// Expressions
x = a + b * c;
y = arr[i] + 5;
```

## 📚 Documentation

- **[GUI/README.md](GUI/README.md)** - Web interface documentation
- **[GUI/QUICKSTART.md](GUI/QUICKSTART.md)** - Get started in 3 minutes
- **[GUI/FEATURES.md](GUI/FEATURES.md)** - Complete feature list
- **[Array-Manipulation-using-LR-parsing--Karim-Ahmed/README.md](Array-Manipulation-using-LR-parsing--Karim-Ahmed/README.md)** - Compiler documentation
- **[Array-Manipulation-using-LR-parsing--Karim-Ahmed/CFG.md](Array-Manipulation-using-LR-parsing--Karim-Ahmed/CFG.md)** - Grammar specification

## 🛠️ Technology Stack

### Compiler (C++)
- C++17
- LR(1) parsing algorithm
- JSON output format

### GUI (Web)
- React 18 + TypeScript
- Monaco Editor (VS Code's editor)
- Vite (build tool)
- Express (API server)

## 📖 Learning Resources

Each compiler phase has detailed documentation:
1. **lexer/** - Tokenization and lexical analysis
2. **parser/** - LR(1) parsing and AST generation
3. **semantic/** - Type checking and semantic analysis
4. **codegen/** - Intermediate code generation

## 🤝 Contributing

This is an educational project demonstrating compiler design principles.

## 📄 License

Educational project for compiler design course.
