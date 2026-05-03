# cin/cout Flow Through Compiler Phases

## Visual Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    SOURCE CODE                                   │
│  #include <iostream>                                             │
│  using namespace std;                                            │
│  int main() {                                                    │
│      int x;                                                      │
│      int y = 10;                                                 │
│      cin >> x;        ← cin statement                            │
│      cout << x;       ← cout statement                           │
│      cout << y;       ← cout statement                           │
│      return 0;                                                   │
│  }                                                               │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│              PHASE 1: LEXICAL ANALYSIS                           │
│  ✅ cin recognized as RESERVED token                             │
│  ✅ cout recognized as RESERVED token                            │
│  ✅ Validates #include <iostream> is present                     │
│  ✅ Generates token stream                                       │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│              PHASE 2: SYNTAX ANALYSIS (PARSING)                  │
│  ✅ Parses cin statements using grammar rules                    │
│  ✅ Parses cout statements using grammar rules                   │
│  ✅ Creates AST nodes:                                           │
│     - Input node for cin                                         │
│     - Output node for cout                                       │
│     - CinList / CoutList for chained operations                  │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│              PHASE 3: SEMANTIC ANALYSIS                          │
│  ✅ Validates cin/cout usage                                     │
│  ✅ Type-checks input targets (must be lvalues)                  │
│  ✅ Type-checks output expressions                               │
│  ✅ Verifies #include <iostream> is present                      │
│  ✅ Annotates AST with type information                          │
│  ⚠️  Errors if iostream is missing                               │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│              PHASE 4: CODE GENERATION (TAC)                      │
│  ❌ cin statements: NO TAC GENERATED                             │
│  ❌ cout statements: NO TAC GENERATED                            │
│  ✅ Other statements: TAC generated normally                     │
│                                                                  │
│  Generated TAC:                                                  │
│    // DECL x  type=int                                           │
│    // DECL y  type=int                                           │
│    y = 10                                                        │
│                                                                  │
│  (Notice: No TAC for cin >> x, cout << x, cout << y)            │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│              PHASE 5: CODE OPTIMIZATION                          │
│  ❌ cin/cout: Nothing to optimize (no TAC generated)             │
│  ✅ Other TAC: Optimized normally                                │
│                                                                  │
│  Optimized TAC:                                                  │
│    // DECL x  type=int                                           │
│    // DECL y  type=int                                           │
│    y = 10                                                        │
└─────────────────────────────────────────────────────────────────┘
```

## Key Points

### ✅ What Works
1. **Lexer**: Recognizes `cin` and `cout` as reserved keywords
2. **Parser**: Parses I/O statements according to grammar
3. **Semantic Analyzer**: Validates types and checks for `#include <iostream>`
4. **Error Detection**: Reports errors when iostream is missing

### ❌ What Doesn't Generate Code
1. **Code Generator**: `cin` and `cout` statements are **skipped** - no TAC emitted
2. **Optimizer**: Nothing to optimize since no TAC was generated

## Comparison: With vs Without cin/cout

### Example 1: Code WITH cin/cout
```cpp
int x;
int y = 10;
cin >> x;      // ← No TAC
cout << x;     // ← No TAC
cout << y;     // ← No TAC
```
**Generated TAC:**
```
// DECL x  type=int
// DECL y  type=int
y = 10
```

### Example 2: Code WITHOUT cin/cout
```cpp
int x;
int y = 10;
x = 5;         // ← Generates TAC
int z = x + y; // ← Generates TAC
```
**Generated TAC:**
```
// DECL x  type=int
// DECL y  type=int
y = 10
x = 5
t1 = x + y
// DECL z  type=int
z = t1
```

## Implementation Details

### Code Generator Functions

```cpp
// Location: codegen/code_generator.cpp

void CodeGenerator::genOutput(shared_ptr<ASTNode> node) {
    // cout statements are allowed in syntax and semantic analysis
    // but do NOT generate any TAC code - skip code generation entirely
    return;  // ← Simply returns without emitting any quads
}

void CodeGenerator::genInput(shared_ptr<ASTNode> node) {
    // cin statements are allowed in syntax and semantic analysis
    // but do NOT generate any TAC code - skip code generation entirely
    return;  // ← Simply returns without emitting any quads
}
```

### Statement Dispatcher

```cpp
void CodeGenerator::genStatement(shared_ptr<ASTNode> node) {
    // ...
    if (t == "Output")  { genOutput(node);  return; }  // ← Calls genOutput
    if (t == "Input")   { genInput(node);   return; }  // ← Calls genInput
    // ...
}
```

## Error Handling

### Case 1: Missing iostream
```cpp
int main() {
    int x;
    cin >> x;  // ← ERROR!
    return 0;
}
```
**Error Message:**
```
Lexical Error: 'cin' requires 'std::' prefix or 'using namespace std;' at line 3
```

### Case 2: With iostream
```cpp
#include <iostream>
using namespace std;
int main() {
    int x;
    cin >> x;  // ← OK!
    return 0;
}
```
**Result:** ✅ Compiles successfully, no TAC generated for cin

## Testing

Run the test suite:
```bash
./Main.exe test_cin_cout_combined.txt
```

Expected output shows:
- ✅ Phase 1: Lexical Analysis - COMPLETE
- ✅ Phase 2: Syntax Analysis - COMPLETE
- ✅ Phase 3: Semantic Analysis - COMPLETE
- ✅ Phase 4: Code Generation - COMPLETE (but no TAC for cin/cout)
- ✅ Phase 5: Code Optimization - COMPLETE

## Summary Table

| Statement Type | Lexer | Parser | Semantic | Code Gen | Optimizer |
|---------------|-------|--------|----------|----------|-----------|
| `int x = 5;` | ✅ | ✅ | ✅ | ✅ TAC | ✅ Optimized |
| `x = y + z;` | ✅ | ✅ | ✅ | ✅ TAC | ✅ Optimized |
| `cin >> x;` | ✅ | ✅ | ✅ | ❌ No TAC | ❌ Nothing |
| `cout << x;` | ✅ | ✅ | ✅ | ❌ No TAC | ❌ Nothing |

## Conclusion

The implementation successfully:
1. ✅ Accepts cin/cout in source code
2. ✅ Validates them in all front-end phases
3. ✅ Prevents TAC generation for I/O operations
4. ✅ Maintains clean separation between I/O and computation
