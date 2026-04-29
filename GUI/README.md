# Compiler GUI - Web Interface

Modern React-based web interface for the Array Manipulation Compiler.

## Features

- **Code Editor** - Syntax-highlighted input area
- **AST Viewer** - Interactive Abstract Syntax Tree visualization
- **Symbol Table** - Variable and type information display
- **Three-Address Code** - Generated intermediate code viewer

## Quick Start

### Install Dependencies
```bash
npm install
```

### Start Development Servers
```bash
# Backend API server (Terminal 1)
node server.js

# Frontend development server (Terminal 2)  
npm run dev
```

### Access the Interface
Open http://localhost:3000 in your browser

## Configuration

- **Backend Port**: 3003 (configured in `server.js`)
- **Frontend Port**: 3000 (configured in `vite.config.ts`)
- **API Proxy**: Frontend proxies `/api/*` requests to backend

## Development

```bash
npm run dev      # Start development server
npm run build    # Build for production
npm run preview  # Preview production build
```
