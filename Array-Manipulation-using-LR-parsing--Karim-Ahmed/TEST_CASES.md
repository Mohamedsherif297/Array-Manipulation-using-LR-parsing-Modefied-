# Test Cases for Compiler Pipeline

## ✅ Test Case 1: 2D Array Declaration with Initialization
**Input:**
```cpp
int x[2][2] = {{1,2},{3,4}};
```

**Expected Output:**
- ✅ Lexer: 24 tokens
- ✅ Parser: Valid AST
- ✅ Semantic: Type checked, symbol table entry for `x`
- ✅ Codegen: TAC with array initialization

**Status:** ✅ PASSED

---

## Test Case 2: Simple Variable Declaration
**Input:**
```cpp
int y;
```

**How to test:**
1. Edit `Main.cpp` line 24: `string src = "int y;";`
2. Compile: `g++ -std=c++17 Main.cpp -o Main`
3. Run: `./Main`

**Expected Output:**
- Lexer: 3 tokens (int, y, ;)
- Parser: Declaration node
- Semantic: Symbol table entry for `y`
- Codegen: DECL instruction

---

## Test Case 3: Variable Declaration with Initialization
**Input:**
```cpp
int z = 5;
```

**Expected Output:**
- Lexer: 5 tokens
- Parser: DeclAssign node
- Semantic: Type checked assignment
- Codegen: DECL + ASSIGN instructions

---

## Test Case 4: 1D Array Declaration
**Input:**
```cpp
int arr[5];
```

**Expected Output:**
- Lexer: 6 tokens
- Parser: Declaration with ArrayDims
- Semantic: Array symbol with size1=5
- Codegen: DECL instruction

---

## Test Case 5: 1D Array with Initialization
**Input:**
```cpp
int arr[3] = {10,20,30};
```

**Expected Output:**
- Lexer: Multiple tokens
- Parser: DeclAssign with array literal
- Semantic: Type checked array elements
- Codegen: Multiple STORE instructions

---

## Test Case 6: Simple Assignment
**Input:**
```cpp
int x = 10;
```

**Expected Output:**
- Lexer: 5 tokens
- Parser: DeclAssign node
- Semantic: Type checked
- Codegen: DECL + ASSIGN

---

## Test Case 7: Arithmetic Expression
**Input:**
```cpp
int result = 2 + 3 * 4;
```

**Expected Output:**
- Lexer: Tokens for expression
- Parser: Expression tree with correct precedence
- Semantic: Type checked (all int)
- Codegen: Multiple temporaries with correct order

---

## Test Case 8: Float Declaration
**Input:**
```cpp
float pi = 3.14;
```

**Expected Output:**
- Lexer: float, pi, =, 3.14, ;
- Parser: DeclAssign node
- Semantic: Type float
- Codegen: DECL + ASSIGN

---

## Test Case 9: Multiple Data Types
**Input:**
```cpp
double d = 2.5;
```

**Expected Output:**
- Lexer: double, d, =, 2.5, ;
- Parser: Valid AST
- Semantic: Type double
- Codegen: DECL + ASSIGN

---

## Test Case 10: Complex 2D Array
**Input:**
```cpp
int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
```

**Expected Output:**
- Lexer: Many tokens
- Parser: Complex array literal
- Semantic: 3x3 array validation
- Codegen: 9 STORE instructions

---

## How to Run Tests

### Method 1: Edit Main.cpp
```cpp
// Line 24 in Main.cpp
string src = "YOUR_TEST_CASE_HERE";
```

### Method 2: Create Test Script
```bash
#!/bin/bash
# test_all.sh

test_cases=(
    "int y;"
    "int z = 5;"
    "int arr[5];"
    "float pi = 3.14;"
    "int x[2][2] = {{1,2},{3,4}};"
)

for test in "${test_cases[@]}"; do
    echo "Testing: $test"
    # Modify Main.cpp and run
done
```

---

## Expected Failures (Not Supported)

### ❌ Undeclared Variable
```cpp
x = 5;  // x not declared
```
**Expected:** Semantic error

### ❌ Type Mismatch
```cpp
int x = "hello";  // string to int
```
**Expected:** Semantic error

### ❌ Array Without Index
```cpp
int arr[5];
arr = 10;  // Should be arr[0] = 10
```
**Expected:** Semantic error

### ❌ Invalid Syntax
```cpp
int x [;  // Missing closing bracket
```
**Expected:** Parser error

---

## Current Test Results

| Test Case | Lexer | Parser | Semantic | Codegen | Status |
|-----------|-------|--------|----------|---------|--------|
| 2D Array Init | ✅ | ✅ | ✅ | ✅ | PASSED |
| Simple Decl | 🔄 | 🔄 | 🔄 | 🔄 | TODO |
| Decl + Init | 🔄 | 🔄 | 🔄 | 🔄 | TODO |
| 1D Array | 🔄 | 🔄 | 🔄 | 🔄 | TODO |
| Expression | 🔄 | 🔄 | 🔄 | 🔄 | TODO |

---

## Notes

- All test cases should be single statements (no multiple statements yet)
- Semicolon is required at the end
- Array indices must be numeric literals in declarations
- Supported types: int, float, double, char, string
