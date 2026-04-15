# GUI Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                         Browser                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │              React Frontend (Port 3000)               │  │
│  │                                                       │  │
│  │  ┌──────────────┐  ┌──────────────────────────────┐ │  │
│  │  │              │  │      Output Panel            │ │  │
│  │  │    Code      │  │  ┌────────────────────────┐  │ │  │
│  │  │   Editor     │  │  │   AST Visualizer       │  │ │  │
│  │  │  (Monaco)    │  │  ├────────────────────────┤  │ │  │
│  │  │              │  │  │   Symbol Table View    │  │ │  │
│  │  │              │  │  ├────────────────────────┤  │ │  │
│  │  │              │  │  │   TAC View             │  │ │  │
│  │  └──────────────┘  │  └────────────────────────┘  │ │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ HTTP POST /api/compile
                            │ { code: "..." }
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              Express Server (Port 3001)                      │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                  API Endpoint                         │  │
│  │                                                       │  │
│  │  1. Receive source code                              │  │
│  │  2. Write to temp file                               │  │
│  │  3. Execute compiler pipeline                        │  │
│  │  4. Read output files                                │  │
│  │  5. Return JSON response                             │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            │
                            │ exec()
                            ▼
┌─────────────────────────────────────────────────────────────┐
│              Compiler Pipeline (C++)                         │
│                                                              │
│  ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌─────────┐ │
│  │  Main    │──▶│ Semantic │──▶│ Codegen  │──▶│ Output  │ │
│  │ (Lexer + │   │ Analyzer │   │          │   │ Files   │ │
│  │  Parser) │   │          │   │          │   │         │ │
│  └──────────┘   └──────────┘   └──────────┘   └─────────┘ │
│       │              │               │              │       │
│       ▼              ▼               ▼              ▼       │
│   ast.json   annotated_ast.json  ir.txt    symbol_table.json│
└─────────────────────────────────────────────────────────────┘
```

## Component Hierarchy

```
App
├── Header
│   └── Title + Description
│
├── EditorSection
│   ├── SectionHeader
│   │   ├── Title
│   │   └── CompileButton
│   └── CodeEditor (Monaco)
│
└── OutputSection
    └── OutputPanel
        ├── Tabs
        │   ├── AST Tab
        │   ├── Symbol Table Tab
        │   └── TAC Tab
        │
        └── TabContent
            ├── ASTVisualizer
            │   ├── TreeView
            │   │   └── TreeNode (recursive)
            │   └── JSONView
            │
            ├── SymbolTableView
            │   ├── TableView
            │   └── DetailsView
            │
            └── TACView
                ├── FormattedView
                └── RawView
```

## Data Flow

### 1. User Input
```
User types code → Monaco Editor → React state (code)
```

### 2. Compilation Request
```
User clicks "Compile" 
  → handleCompile()
  → fetch('/api/compile', { code })
  → Express server receives request
```

### 3. Compiler Execution
```
Express server:
  1. Write code to temp file
  2. Execute: Main (lexer + parser)
     → Generates: ast.json
  3. Execute: semantic_main
     → Generates: annotated_ast.json, symbol_table.json
  4. Execute: codegen
     → Generates: ir.txt
  5. Read all output files
  6. Return JSON: { ast, symbolTable, tac }
```

### 4. Display Results
```
Response received
  → setOutput({ ast, symbolTable, tac })
  → OutputPanel renders
  → User switches tabs to view different outputs
```

## File Structure

```
GUI/
├── public/                 # Static assets
│   └── vite.svg
│
├── src/
│   ├── components/         # React components
│   │   ├── CodeEditor.tsx
│   │   ├── CodeEditor.css
│   │   ├── ASTVisualizer.tsx
│   │   ├── ASTVisualizer.css
│   │   ├── SymbolTableView.tsx
│   │   ├── SymbolTableView.css
│   │   ├── TACView.tsx
│   │   ├── TACView.css
│   │   ├── OutputPanel.tsx
│   │   └── OutputPanel.css
│   │
│   ├── App.tsx             # Main component
│   ├── App.css
│   ├── main.tsx            # Entry point
│   ├── index.css           # Global styles
│   └── vite-env.d.ts       # Type definitions
│
├── server.js               # Express API server
├── vite.config.ts          # Vite configuration
├── tsconfig.json           # TypeScript config
├── tsconfig.node.json      # Node TypeScript config
├── package.json            # Dependencies
├── setup.sh                # Setup script
│
└── Documentation/
    ├── START_HERE.md
    ├── QUICKSTART.md
    ├── README.md
    ├── FEATURES.md
    ├── EXAMPLES.md
    ├── SETUP_INSTRUCTIONS.md
    └── ARCHITECTURE.md (this file)
```

## Technology Stack

### Frontend
- **React 18**: UI framework
- **TypeScript**: Type safety
- **Vite**: Build tool & dev server
- **Monaco Editor**: Code editor component
- **CSS3**: Styling (no framework)

### Backend
- **Node.js**: Runtime
- **Express**: Web framework
- **Child Process**: Execute compiler

### Compiler
- **C++17**: Implementation language
- **JSON**: Output format
- **Text**: TAC output

## API Specification

### POST /api/compile

**Request:**
```json
{
  "code": "int x[2][2] = {{1,2},{3,4}};"
}
```

**Response (Success):**
```json
{
  "ast": {
    "type": "Program",
    "children": [...]
  },
  "symbolTable": {
    "x": {
      "type": "int",
      "isArray": true,
      "size1": 2,
      "size2": 2
    }
  },
  "tac": "// DECL x  type=int\nt1 = 0 * 4\n..."
}
```

**Response (Error):**
```json
{
  "error": "Compilation failed",
  "details": "syntax error at line 1"
}
```

## State Management

### App State
```typescript
interface AppState {
  code: string              // Source code
  output: CompilerOutput    // Compilation results
  isCompiling: boolean      // Loading state
}

interface CompilerOutput {
  ast?: any                 // Abstract Syntax Tree
  symbolTable?: any         // Symbol table
  tac?: string             // Three-Address Code
  error?: string           // Error message
}
```

### Component State
- **OutputPanel**: `activeTab` (ast | symbols | tac)
- **ASTVisualizer**: `viewMode` (tree | json)
- **TreeNode**: `isExpanded` (boolean)

## Build Process

### Development
```bash
npm run dev
# Vite dev server with HMR
# Fast refresh on file changes
# Source maps for debugging
```

### Production
```bash
npm run build
# TypeScript compilation
# Vite optimization
# Bundle minification
# Output to dist/
```

## Performance Considerations

### Frontend
- Lazy loading of Monaco Editor
- Efficient re-rendering with React hooks
- CSS-only animations (no JS)
- Minimal bundle size

### Backend
- Async/await for non-blocking I/O
- Timeout protection (10s)
- Temp file cleanup
- Error handling

### Compiler
- Fast C++ execution
- JSON streaming for large ASTs
- Efficient memory management

## Security

### Input Validation
- Code length limits
- Timeout protection
- Sanitized file paths

### File System
- Isolated temp directory
- Automatic cleanup
- No arbitrary file access

### Network
- CORS enabled for localhost
- No external API calls
- Local execution only

## Error Handling

### Frontend
- Network errors
- Timeout errors
- Invalid responses
- Display user-friendly messages

### Backend
- Compilation errors
- File I/O errors
- Process execution errors
- Return structured error responses

### Compiler
- Syntax errors
- Semantic errors
- Type errors
- Propagate to backend

## Future Enhancements

### Potential Features
- [ ] Save/load code snippets
- [ ] Syntax error highlighting in editor
- [ ] Step-by-step execution
- [ ] Optimization visualization
- [ ] Multiple file support
- [ ] Export results (PDF, PNG)
- [ ] Dark/light theme toggle
- [ ] Collaborative editing
- [ ] Code templates
- [ ] Execution trace

### Performance
- [ ] WebAssembly compiler
- [ ] Worker threads for compilation
- [ ] Caching compiled results
- [ ] Progressive loading

### UI/UX
- [ ] Keyboard shortcuts panel
- [ ] Tutorial/walkthrough
- [ ] Code snippets library
- [ ] Diff view for changes
- [ ] Mobile responsive design
