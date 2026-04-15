# 🚀 Quick Start Guide

Get the compiler GUI up and running in 3 minutes!

## Prerequisites Check

```bash
# Check Node.js version (need v18+)
node --version

# Check npm version
npm --version

# Check g++ compiler
g++ --version
```

If any are missing, install them first.

## Step 1: Setup (One-time)

```bash
cd GUI
./setup.sh
```

This installs dependencies and compiles the compiler executables.

## Step 2: Start the Application

### Option A: Start Both Servers Together (Recommended)

```bash
npm start
```

### Option B: Start Separately

**Terminal 1:**
```bash
npm run server
```

**Terminal 2:**
```bash
npm run dev
```

## Step 3: Open in Browser

Navigate to: **http://localhost:3000**

## Step 4: Try It Out!

1. The editor comes with example code pre-loaded
2. Click **"▶️ Compile & Run"**
3. Switch between tabs to see:
   - 🌳 **AST** - Abstract Syntax Tree
   - 📊 **Symbol Table** - Variable declarations
   - ⚙️ **TAC** - Three-Address Code

## Example Code to Try

### Example 1: Simple Array
```cpp
int arr[3] = {1, 2, 3};
```

### Example 2: 2D Array
```cpp
int matrix[2][2] = {{1,2},{3,4}};
```

### Example 3: Variable Declaration
```cpp
int x = 5;
```

### Example 4: Array Assignment
```cpp
int arr[5];
arr[0] = 10;
```

### Example 5: Expression
```cpp
int x = 2 + 3 * 4;
```

## Troubleshooting

### "Cannot find module" error
```bash
cd GUI
rm -rf node_modules package-lock.json
npm install
```

### "Port 3000 already in use"
Kill the process using port 3000:
```bash
# macOS/Linux
lsof -ti:3000 | xargs kill -9

# Or change the port in vite.config.ts
```

### "Compilation failed" in browser
1. Check the terminal running `npm run server` for errors
2. Ensure compiler executables are built:
   ```bash
   ls -la ../Array-Manipulation-using-LR-parsing--Karim-Ahmed/Main
   ls -la ../Array-Manipulation-using-LR-parsing--Karim-Ahmed/semantic/semantic_main
   ls -la ../Array-Manipulation-using-LR-parsing--Karim-Ahmed/codegen/codegen
   ```
3. If missing, run `./setup.sh` again

### Monaco Editor not loading
1. Clear browser cache (Ctrl/Cmd + Shift + R)
2. Check browser console for errors (F12)
3. Try a different browser

## Next Steps

- Read [FEATURES.md](FEATURES.md) for detailed feature list
- Check [README.md](README.md) for architecture details
- See [SETUP_INSTRUCTIONS.md](SETUP_INSTRUCTIONS.md) for manual setup

## Keyboard Shortcuts

- **Ctrl/Cmd + Enter**: Compile code
- **Ctrl/Cmd + F**: Find in editor
- **Ctrl/Cmd + H**: Replace in editor
- **Alt + Click**: Multi-cursor

## Support

If you encounter issues:
1. Check the terminal logs
2. Verify all prerequisites are installed
3. Ensure you're in the correct directory
4. Try running `./setup.sh` again

## Development Mode

The app runs in development mode with:
- Hot Module Replacement (HMR)
- Source maps for debugging
- Detailed error messages
- Fast refresh on code changes

Enjoy coding! 🎉
