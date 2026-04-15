# Setup Instructions

## Quick Setup

Run the automated setup script:

```bash
cd GUI
./setup.sh
```

This will:
1. Install all npm dependencies
2. Compile the compiler executables if needed
3. Create necessary directories

## Manual Setup

If you prefer to set up manually:

### 1. Install Dependencies

```bash
cd GUI
npm install
```

### 2. Compile Compiler Executables

#### Main Compiler (Lexer + Parser)
```bash
cd ../Array-Manipulation-using-LR-parsing--Karim-Ahmed
g++ -std=c++17 Main.cpp -o Main
```

#### Semantic Analyzer
```bash
cd semantic
g++ -std=c++17 semantic_main.cpp ast_builder.cpp semantic_analyzer.cpp \
    semantic_output.cpp symbol_table.cpp -o semantic_main
```

#### Code Generator
```bash
cd ../codegen
make
```

### 3. Create Temp Directory

```bash
cd ../../GUI
mkdir -p temp
```

## Running the Application

### Option 1: Run Both Servers Together
```bash
npm start
```

### Option 2: Run Separately

**Terminal 1 - Backend:**
```bash
npm run server
```

**Terminal 2 - Frontend:**
```bash
npm run dev
```

Then open your browser to: `http://localhost:3000`

## Troubleshooting

### "Cannot find module" errors
```bash
rm -rf node_modules package-lock.json
npm install
```

### Compiler executable not found
Make sure all executables are compiled and have execute permissions:
```bash
chmod +x ../Array-Manipulation-using-LR-parsing--Karim-Ahmed/Main
chmod +x ../Array-Manipulation-using-LR-parsing--Karim-Ahmed/semantic/semantic_main
chmod +x ../Array-Manipulation-using-LR-parsing--Karim-Ahmed/codegen/codegen
```

### Port already in use
Change the port in:
- `vite.config.ts` (line 6) for frontend
- `server.js` (line 12) for backend

### Compilation fails in browser
Check the backend logs in the terminal running `npm run server` for detailed error messages.

## System Requirements

- Node.js v18 or higher
- npm v9 or higher
- g++ with C++17 support
- macOS, Linux, or WSL on Windows
