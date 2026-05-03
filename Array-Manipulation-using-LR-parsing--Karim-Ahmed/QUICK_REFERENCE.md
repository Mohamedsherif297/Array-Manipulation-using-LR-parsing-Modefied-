# cin/cout Quick Reference Card

## ✅ What Works

| Phase | cin/cout Behavior |
|-------|-------------------|
| **Lexer** | ✅ Recognized as RESERVED tokens |
| **Parser** | ✅ Parsed and AST created |
| **Semantic** | ✅ Validated and type-checked |
| **Code Gen** | ❌ **NO TAC generated** |
| **Optimizer** | ❌ **Nothing to optimize** |

## 🚀 Quick Test

```bash
# Compile
./recompile_all.bat

# Test
./Main.exe test_comprehensive.txt
```

## 📝 Example

### Input:
```cpp
#include <iostream>
using namespace std;
int main() {
    int x = 5;
    cin >> x;      // ← No TAC
    cout << x;     // ← No TAC
    return 0;
}
```

### Output TAC:
```
// DECL x  type=int
x = 5
```

**Notice**: No TAC for `cin >> x` or `cout << x`!

## 📚 Documentation

- `README_CIN_COUT.md` - Start here
- `CIN_COUT_FLOW.md` - Visual diagram
- `CIN_COUT_IMPLEMENTATION.md` - Full details
- `BEFORE_AFTER_COMPARISON.md` - What changed

## 🧪 Test Files

- `test_cout_only.txt`
- `test_cin_only.txt`
- `test_cin_cout_combined.txt`
- `test_comprehensive.txt`
- `test_cin_no_iostream.txt`

## ⚠️ Error Case

Without `#include <iostream>`:
```cpp
int main() {
    cin >> x;  // ← ERROR!
}
```

**Error**:
```
Lexical Error: 'cin' requires 'std::' prefix or 'using namespace std;'
```

## ✅ Status: COMPLETE

cin/cout are:
- ✅ Fully supported in lexer, parser, semantic
- ❌ Excluded from TAC generation
- ❌ Excluded from optimization
