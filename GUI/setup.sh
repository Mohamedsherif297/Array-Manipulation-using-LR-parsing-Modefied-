#!/bin/bash

echo "🔧 Setting up Array Manipulation Compiler GUI..."

# Check if we're in the GUI directory
if [ ! -f "package.json" ]; then
    echo "❌ Error: Please run this script from the GUI directory"
    exit 1
fi

# Install npm dependencies
echo "📦 Installing npm dependencies..."
npm install

# Check if compiler executables exist
COMPILER_DIR="../Array-Manipulation-using-LR-parsing--Karim-Ahmed"

echo ""
echo "🔍 Checking compiler executables..."

# Check Main executable
if [ ! -f "$COMPILER_DIR/Main" ]; then
    echo "⚠️  Main executable not found. Compiling..."
    cd "$COMPILER_DIR"
    g++ -std=c++17 Main.cpp -o Main
    if [ $? -eq 0 ]; then
        echo "✅ Main compiled successfully"
    else
        echo "❌ Failed to compile Main"
        exit 1
    fi
    cd - > /dev/null
else
    echo "✅ Main executable found"
fi

# Check semantic_main executable
if [ ! -f "$COMPILER_DIR/semantic/semantic_main" ]; then
    echo "⚠️  semantic_main executable not found. Compiling..."
    cd "$COMPILER_DIR/semantic"
    g++ -std=c++17 semantic_main.cpp ast_builder.cpp semantic_analyzer.cpp semantic_output.cpp symbol_table.cpp -o semantic_main
    if [ $? -eq 0 ]; then
        echo "✅ semantic_main compiled successfully"
    else
        echo "❌ Failed to compile semantic_main"
        exit 1
    fi
    cd - > /dev/null
else
    echo "✅ semantic_main executable found"
fi

# Check codegen executable
if [ ! -f "$COMPILER_DIR/codegen/codegen" ]; then
    echo "⚠️  codegen executable not found. Compiling..."
    cd "$COMPILER_DIR/codegen"
    make clean && make
    if [ $? -eq 0 ]; then
        echo "✅ codegen compiled successfully"
    else
        echo "❌ Failed to compile codegen"
        exit 1
    fi
    cd - > /dev/null
else
    echo "✅ codegen executable found"
fi

# Create temp directory
mkdir -p temp

echo ""
echo "✅ Setup complete!"
echo ""
echo "To start the application:"
echo "  npm run server    # Start backend (Terminal 1)"
echo "  npm run dev       # Start frontend (Terminal 2)"
echo ""
echo "Or run both together:"
echo "  npm start"
echo ""
