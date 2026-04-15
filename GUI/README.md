# Array Manipulation Compiler - GUI

A modern React-based web interface for the Array Manipulation Compiler with syntax highlighting, AST visualization, symbol table display, and Three-Address Code (TAC) output.

## Features

- 🎨 **Monaco Editor** - VS Code-like code editor with syntax highlighting
- 🌳 **AST Visualization** - Interactive tree view of the Abstract Syntax Tree
- 📊 **Symbol Table** - Tabular display of declared variables and their properties
- ⚙️ **TAC Display** - Formatted view of generated Three-Address Code
- 🎯 **Real-time Compilation** - Compile and see results instantly
- 🌙 **Dark Theme** - Modern dark UI matching VS Code aesthetics

## Prerequisites

- Node.js (v18 or higher)
- npm or yarn
- Compiled compiler executables:
  - `../Array-Manipulation-using-LR-parsing--Karim-Ahmed/Main`
  - `../Array-Manipulation-using-LR-parsing--Karim-Ahmed/semantic/semantic_main`
  - `../Array-Manipulation-using-LR-parsing--Karim-Ahmed/codegen/codegen`

## Installation

```bash
cd GUI
npm install
```

## Running the Application

You need to run both the frontend and backend:

### Terminal 1 - Backend API Server
```bash
npm run server
```
This starts the Express server on `http://localhost:3001`

### Terminal 2 - Frontend Dev Server
```bash
npm run dev
```
This starts the Vite dev server on `http://localhost:3000`

Open your browser to `http://localhost:3000`

## Usage

1. **Write Code**: Type or paste your array manipulation code in the editor
   - Example: `int x[2][2] = {{1,2},{3,4}};`

2. **Compile**: Click the "▶️ Compile & Run" button

3. **View Results**: Switch between tabs to see:
   - **Abstract Syntax Tree**: Hierarchical view of parsed code
   - **Symbol Table**: Variables and their types/dimensions
   - **Three-Address Code**: Generated intermediate representation

## Supported Syntax

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

## Project Structure

```
GUI/
├── src/
│   ├── components/
│   │   ├── CodeEditor.tsx       # Monaco editor wrapper
│   │   ├── ASTVisualizer.tsx    # AST tree visualization
│   │   ├── SymbolTableView.tsx  # Symbol table display
│   │   ├── TACView.tsx          # TAC code display
│   │   └── OutputPanel.tsx      # Tab container
│   ├── App.tsx                  # Main application
│   └── main.tsx                 # Entry point
├── server.js                    # Express API server
├── package.json
└── vite.config.ts
```

## API Endpoints

### POST `/api/compile`
Compiles the provided source code through all compiler phases.

**Request:**
```json
{
  "code": "int x[2][2] = {{1,2},{3,4}};"
}
```

**Response:**
```json
{
  "ast": { ... },
  "symbolTable": { ... },
  "tac": "..."
}
```

## Building for Production

```bash
npm run build
```

The built files will be in the `dist/` directory.

## Troubleshooting

### Compilation Errors
- Ensure all compiler executables are built and have execute permissions
- Check that paths in `server.js` point to the correct compiler directory

### Port Already in Use
- Change the port in `vite.config.ts` (frontend) or `server.js` (backend)

### Monaco Editor Not Loading
- Clear browser cache and reload
- Check browser console for errors

## Technologies Used

- **React 18** - UI framework
- **TypeScript** - Type safety
- **Vite** - Build tool and dev server
- **Monaco Editor** - Code editor (VS Code's editor)
- **Express** - Backend API server
- **Node.js** - Server runtime

## License

Part of the Array Manipulation Compiler project.
