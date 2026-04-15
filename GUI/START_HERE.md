# 🎯 START HERE - Array Manipulation Compiler GUI

Welcome! This is your **one-stop guide** to get started with the compiler GUI.

## ⚡ Super Quick Start (3 Steps)

```bash
# 1. Navigate to GUI folder
cd GUI

# 2. Run setup (installs dependencies & compiles compiler)
./setup.sh

# 3. Start the application
npm start
```

**Then open:** http://localhost:3000

That's it! 🎉

## 📚 Documentation Guide

Choose your path:

### 🚀 I want to start immediately
→ Read [QUICKSTART.md](QUICKSTART.md)

### 🎨 I want to see what features are available
→ Read [FEATURES.md](FEATURES.md)

### 💡 I want example code to try
→ Read [EXAMPLES.md](EXAMPLES.md)

### 🔧 I want detailed setup instructions
→ Read [SETUP_INSTRUCTIONS.md](SETUP_INSTRUCTIONS.md)

### 📖 I want to understand the architecture
→ Read [README.md](README.md)

## 🎯 What You'll Get

A beautiful web interface with:

1. **Code Editor** 
   - Syntax highlighting like VS Code
   - Line numbers, auto-indent
   - Find & replace

2. **AST Visualizer**
   - Interactive tree view
   - Expand/collapse nodes
   - Color-coded elements

3. **Symbol Table**
   - All declared variables
   - Types and dimensions
   - Clean table format

4. **TAC Output**
   - Three-Address Code
   - Syntax highlighted
   - Line numbers

## 🎬 Quick Demo

1. The editor starts with example code:
   ```cpp
   int x[2][2] = {{1,2},{3,4}};
   ```

2. Click **"▶️ Compile & Run"**

3. See the magic happen:
   - **AST tab**: Tree structure of your code
   - **Symbol Table tab**: Variable `x` with dimensions
   - **TAC tab**: Generated intermediate code

## 🆘 Need Help?

### Setup Issues
```bash
# Re-run setup
./setup.sh

# Or install manually
npm install
```

### Port Conflicts
Edit `vite.config.ts` and `server.js` to change ports

### Compilation Errors
Check the terminal running `npm run server` for detailed logs

### Browser Issues
- Try Chrome or Firefox
- Clear cache (Ctrl/Cmd + Shift + R)
- Check console (F12)

## 🎓 Learning Path

**Beginner:**
1. Start with [QUICKSTART.md](QUICKSTART.md)
2. Try examples from [EXAMPLES.md](EXAMPLES.md)
3. Explore the GUI features

**Intermediate:**
1. Read [FEATURES.md](FEATURES.md)
2. Understand the compiler phases
3. Experiment with complex code

**Advanced:**
1. Read [README.md](README.md)
2. Study the compiler source code
3. Modify and extend the GUI

## 🔥 Pro Tips

- **Keyboard Shortcut**: Ctrl/Cmd + Enter to compile
- **Multi-cursor**: Alt + Click for multiple cursors
- **Find**: Ctrl/Cmd + F to search in editor
- **Expand All**: Click root node in AST to expand tree

## 📊 System Requirements

- **Node.js**: v18 or higher
- **npm**: v9 or higher
- **g++**: C++17 support
- **OS**: macOS, Linux, or WSL
- **Browser**: Chrome, Firefox, or Safari

## 🎯 Next Steps

1. ✅ Run `./setup.sh`
2. ✅ Start with `npm start`
3. ✅ Open http://localhost:3000
4. ✅ Try the example code
5. ✅ Explore the tabs
6. ✅ Write your own code
7. ✅ Have fun! 🎉

## 📞 Quick Reference

| Command | Purpose |
|---------|---------|
| `./setup.sh` | One-time setup |
| `npm start` | Start both servers |
| `npm run dev` | Start frontend only |
| `npm run server` | Start backend only |
| `npm run build` | Build for production |

## 🌟 Features at a Glance

- ✅ Real-time syntax highlighting
- ✅ Interactive AST visualization
- ✅ Symbol table display
- ✅ TAC code generation
- ✅ Error reporting
- ✅ Dark theme
- ✅ Responsive design
- ✅ Fast compilation
- ✅ Modern UI/UX

---

**Ready to compile?** Run `./setup.sh` and let's go! 🚀
