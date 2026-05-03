# cin/cout Implementation Documentation

## Overview
This document describes how `cin` and `cout` are handled in the compiler pipeline. They are **fully supported** in the lexer, parser, and semantic analysis phases, but **do not generate any TAC (Three-Address Code)** in the code generation phase.

## Implementation Details

### Phase 1: Lexical Analysis (Lexer)
**Location**: `lexer/Lexer.h`

- `cin` and `cout` are recognized as **RESERVED** tokens
- The lexer tracks whether `#include <iostream>` is present via the `hasIostreamInclude` flag
- If `cin` or `cout` is used without `#include <iostream>` or `using namespace std;`, a lexical error is raised

**Example Token Stream**:
```
[RESERVED] cin
[OPERATOR] >>
[IDENTIFIER] x
[SYMBOL] ;
```

### Phase 2: Syntax Analysis (Parser)
**Location**: `parser/Grammar.h`

The grammar includes productions for I/O statements:

```cpp
{"IoStmt", {"cout", "CoutList", ";"}},
{"IoStmt", {"cin", "CinList", ";"}},

{"CoutList", {"CoutList", "<<", "Expr"}},
{"CoutList", {"<<", "Expr"}},

{"CinList", {"CinList", ">>", "InputTarget"}},
{"CinList", {">>", "InputTarget"}},

{"InputTarget", {"ID"}},
{"InputTarget", {"ArrayAccess"}}
```

**AST Nodes Generated**:
- `Output` node for `cout` statements
- `Input` node for `cin` statements
- `CoutList` and `CinList` nodes for chained I/O operations

### Phase 3: Semantic Analysis
**Location**: `semantic/semantic_analyzer.cpp`

The semantic analyzer performs the following checks:

1. **Include Validation**: Verifies that `#include <iostream>` is present when `cin` or `cout` is used
2. **Type Checking**: Validates that input targets are valid lvalues (variables or array elements)
3. **Expression Validation**: Checks that output expressions are valid

**Key Functions**:
```cpp
void SemanticAnalyzer::visitOutput(shared_ptr<ASTNode> node) {
    usesCinOrCout_ = true;  // Mark that cout is used
    
    // Process the output expressions
    for (auto& child : node->children) {
        if (child->type == "CoutList") {
            for (auto& expr : child->children) {
                visitExpr(expr);
            }
        } else {
            visitExpr(child);
        }
    }
    
    node->dataType = "void";
}

void SemanticAnalyzer::visitInput(shared_ptr<ASTNode> node) {
    usesCinOrCout_ = true;  // Mark that cin is used
    
    // Process the input targets (must be lvalues)
    // ... validation code ...
    
    node->dataType = "void";
}
```

**Error Detection**:
- If `cin` or `cout` is used without `#include <iostream>`, a semantic error is raised:
  ```
  [Semantic Error] 'cin' or 'cout' requires '#include <iostream>'
  ```

### Phase 4: Code Generation (TAC)
**Location**: `codegen/code_generator.cpp`

**CRITICAL**: `cin` and `cout` statements **DO NOT** generate any TAC code.

```cpp
void CodeGenerator::genOutput(shared_ptr<ASTNode> node) {
    // cout statements are allowed in syntax and semantic analysis
    // but do NOT generate any TAC code - skip code generation entirely
    return;
}

void CodeGenerator::genInput(shared_ptr<ASTNode> node) {
    // cin statements are allowed in syntax and semantic analysis
    // but do NOT generate any TAC code - skip code generation entirely
    return;
}
```

**Result**: When the code generator encounters `cin` or `cout` nodes, it simply returns without emitting any quads.

### Phase 5: Code Optimization
**Location**: `optimizer/optimizer.cpp`

Since no TAC is generated for `cin` and `cout`, there is nothing to optimize. The optimizer processes only the TAC generated from other statements (declarations, assignments, arithmetic operations, etc.).

## Example Compilation

### Input Code:
```cpp
#include <iostream>
using namespace std;
int main() {
    int x;
    int y = 10;
    cin >> x;
    cout << x;
    cout << y;
    return 0;
}
```

### Phase 1 Output (Tokens):
```
[RESERVED] #
[RESERVED] include
[OPERATOR] <
[RESERVED] iostream
[OPERATOR] >
[RESERVED] using
[RESERVED] namespace
[RESERVED] std
[SYMBOL] ;
[DATATYPE] int
[IDENTIFIER] main
...
[RESERVED] cin
[OPERATOR] >>
[IDENTIFIER] x
[SYMBOL] ;
[RESERVED] cout
[OPERATOR] <<
[IDENTIFIER] x
[SYMBOL] ;
...
```

### Phase 2 Output (AST):
```json
{
  "type": "Input",
  "children": [
    {
      "type": "CinList",
      "children": [
        {"type": "ID", "value": "x"}
      ]
    }
  ]
}
```

### Phase 3 Output (Semantic Analysis):
```
[Semantic Analysis] No errors found.

--- Semantic Symbol Table ---
  y | type: int | isArray: false
  x | type: int | isArray: false
```

### Phase 4 Output (TAC):
```
// DECL x  type=int
// DECL y  type=int
y = 10
```

**Notice**: No TAC for `cin >> x`, `cout << x`, or `cout << y`!

### Phase 5 Output (Optimized TAC):
```
// DECL x  type=int
// DECL y  type=int
y = 10
```

## Test Cases

### Test 1: cout only
**File**: `test_cout_only.txt`
```cpp
#include <iostream>
using namespace std;
int main() {
    int x = 5;
    cout << x;
    return 0;
}
```
**Expected TAC**:
```
// DECL x  type=int
x = 5
```

### Test 2: cin only
**File**: `test_cin_only.txt`
```cpp
#include <iostream>
using namespace std;
int main() {
    int x;
    cin >> x;
    return 0;
}
```
**Expected TAC**:
```
// DECL x  type=int
```

### Test 3: cin and cout combined
**File**: `test_cin_cout_combined.txt`
```cpp
#include <iostream>
using namespace std;
int main() {
    int x;
    int y = 10;
    cin >> x;
    cout << x;
    cout << y;
    return 0;
}
```
**Expected TAC**:
```
// DECL x  type=int
// DECL y  type=int
y = 10
```

### Test 4: cin without iostream (Error Case)
**File**: `test_cin_no_iostream.txt`
```cpp
int main() {
    int x;
    cin >> x;
    return 0;
}
```
**Expected Result**: Lexical error
```
Lexical Error: 'cin' requires 'std::' prefix or 'using namespace std;' at line 3
```

## Summary

| Phase | cin/cout Handling |
|-------|-------------------|
| **Lexer** | ✅ Recognized as RESERVED tokens, validates iostream include |
| **Parser** | ✅ Parsed according to grammar rules, AST nodes created |
| **Semantic** | ✅ Type-checked, validated, errors reported if iostream missing |
| **Code Gen** | ❌ **NO TAC generated** - functions return immediately |
| **Optimizer** | ❌ **NO optimization** - no TAC to optimize |

This design allows the compiler to:
1. Accept and validate I/O statements in source code
2. Provide meaningful error messages when iostream is missing
3. Generate clean TAC that focuses only on computational operations
4. Maintain a clear separation between I/O operations and core computation

## Running Tests

To test the implementation:

```bash
# Compile all components
./recompile_all.bat

# Test with cin and cout
./Main.exe test_cin_cout_combined.txt

# Test with cout only
./Main.exe test_cout_only.txt

# Test with cin only
./Main.exe test_cin_only.txt

# Test error case (no iostream)
./Main.exe test_cin_no_iostream.txt
```

## Files Modified

1. `codegen/code_generator.cpp` - Updated `genOutput()` and `genInput()` to skip TAC generation
2. Created test files:
   - `test_cout_only.txt`
   - `test_cin_only.txt`
   - `test_cin_cout_combined.txt`

## Notes

- The semantic analyzer already had the correct implementation for validating cin/cout
- The code generator functions were already empty, but comments were updated for clarity
- No changes were needed to the lexer or parser - they already handled cin/cout correctly
- The optimizer naturally handles this correctly since it only processes the TAC that is generated
