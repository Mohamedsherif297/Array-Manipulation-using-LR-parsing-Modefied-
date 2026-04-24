# Array Manipulation Compiler with LR Parsing

A complete compiler implementation for array manipulation using LR parsing, featuring a modern web-based GUI interface.

## 🚀 Quick Start

### Prerequisites
- **Node.js** (v16 or higher)
- **g++** compiler with C++17 support
- **npm** package manager

### 1. Setup Dependencies
```bash
cd GUI
npm install
```

### 2. Compile the Compiler
```bash
cd Array-Manipulation-using-LR-parsing--Karim-Ahmed
compile_main_only.bat
```

### 3. Start the System

#### Option A: Manual Start (Recommended)
**Terminal 1 - Backend Server:**
```bash
cd GUI
node server.js
```

**Terminal 2 - Frontend GUI:**
```bash
cd GUI
npm run dev
```

#### Option B: Quick Start Script
```bash
# Double-click this file or run:
start_compiler.bat
```

### 4. Access the GUI
Open your browser and navigate to: **http://localhost:3000**

## 📋 Usage Guide

### Supported Syntax
The compiler supports array manipulation with the following syntax:

```c
// Array declarations with initialization
int x[2] = {1, 2};
int y[3] = {10, 20, 30};

// Scalar variables
int a = x[0];
int b = y[1];

// Complex expressions
int z[5] = {2 + x[0], 4 + x[1], 3, 4, 5};
```

### GUI Features
1. **Code Editor** - Enter your source code
2. **AST Viewer** - View the Abstract Syntax Tree
3. **Symbol Table** - See all declared variables with types and array information
4. **Three-Address Code** - View generated intermediate code

### Compilation Pipeline
The compiler processes code through 5 phases:
1. **Lexical Analysis** - Tokenization
2. **Syntax Analysis** - LR(1) parsing and AST generation
3. **Semantic Analysis** - Type checking and symbol table generation
4. **Code Generation** - Three-address code generation
5. **Code Optimization** - Constant folding, dead code elimination

## 🏗️ Project Structure

```
├── Array-Manipulation-using-LR-parsing--Karim-Ahmed/
│   ├── Main.cpp                    # Main compiler executable
│   ├── Main.exe                    # Compiled main program
│   ├── compile_main_only.bat       # Compilation script
│   ├── lexer/                      # Lexical analyzer
│   ├── parser/                     # LR(1) parser implementation
│   ├── semantic/                   # Semantic analyzer
│   ├── codegen/                    # Code generator
│   └── optimizer/                  # Code optimizer
├── GUI/
│   ├── server.js                   # Express backend server
│   ├── package.json                # Node.js dependencies
│   ├── vite.config.ts              # Frontend configuration
│   ├── src/                        # React frontend source
│   └── temp/                       # Temporary compilation files
└── start_compiler.bat              # Quick start script
```

## 🔧 Configuration

### Server Ports
- **Backend API**: http://localhost:3003
- **Frontend GUI**: http://localhost:3000

### Changing Ports
If you need to change ports due to conflicts:

1. **Backend Port** - Edit `GUI/server.js`:
   ```javascript
   const PORT = 3003  // Change this
   ```

2. **Frontend Proxy** - Edit `GUI/vite.config.ts`:
   ```javascript
   proxy: {
     '/api': {
       target: 'http://localhost:3003',  // Match backend port
       changeOrigin: true
     }
   }
   ```

## 🛠️ Development

### Recompiling Components
If you modify the compiler source code:

```bash
# Recompile main compiler
cd Array-Manipulation-using-LR-parsing--Karim-Ahmed
compile_main_only.bat

# Recompile individual components
cd semantic && g++ -std=c++17 -o semantic_main.exe *.cpp
cd codegen && g++ -std=c++17 -o codegen.exe *.cpp
cd optimizer && g++ -std=c++17 -o optimizer.exe *.cpp
```

### Frontend Development
```bash
cd GUI
npm run dev    # Development server with hot reload
npm run build  # Production build
```

## 📝 Example Usage

1. Start the system using the quick start guide above
2. Open http://localhost:3000 in your browser
3. Enter this example code:
   ```c
   int x[2] = {1, 2};
   int y = x[0];
   int z[3] = {x[0] + x[1], 5, 10};
   ```
4. Click "Compile" to see the results in all tabs

## 🐛 Troubleshooting

### Common Issues

**Port Already in Use:**
- Kill existing Node.js processes: `taskkill /F /IM node.exe`
- Or change ports in configuration

**Compilation Errors:**
- Ensure g++ is installed and in PATH
- Check that all source files are present
- Run `compile_main_only.bat` to recompile

**GUI Not Loading:**
- Check that both servers are running
- Verify ports are not blocked by firewall
- Check browser console for errors

**Empty Results:**
- Ensure Main.exe exists in the compiler directory
- Check server logs for compilation errors
- Verify input syntax is correct

### Getting Help
If you encounter issues:
1. Check the browser console for frontend errors
2. Check the terminal running the backend for server errors
3. Ensure all prerequisites are installed
4. Try recompiling the compiler components

## 📄 License

This project is for educational purposes. See individual component licenses for details.