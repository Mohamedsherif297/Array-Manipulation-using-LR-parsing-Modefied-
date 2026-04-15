# Code Examples

Try these examples in the GUI to see how the compiler works!

## Basic Examples

### 1. Simple Variable Declaration
```cpp
int x;
```

**Expected Output:**
- AST: Single declaration node
- Symbol Table: `x` of type `int`, not an array
- TAC: Declaration comment only

---

### 2. Variable with Initialization
```cpp
int x = 5;
```

**Expected Output:**
- AST: DeclAssign node with value 5
- Symbol Table: `x` of type `int`
- TAC: Assignment instruction

---

### 3. 1D Array Declaration
```cpp
int arr[5];
```

**Expected Output:**
- AST: Array declaration with dimension 5
- Symbol Table: `arr` as array with size1=5
- TAC: Array declaration

---

### 4. 1D Array with Initialization
```cpp
int arr[3] = {1, 2, 3};
```

**Expected Output:**
- AST: Array with 3 elements
- Symbol Table: `arr[3]`
- TAC: Multiple assignment instructions for each element

---

### 5. 2D Array Declaration
```cpp
int matrix[2][3];
```

**Expected Output:**
- AST: 2D array declaration
- Symbol Table: `matrix` with size1=2, size2=3
- TAC: 2D array declaration

---

## Intermediate Examples

### 6. 2D Array with Initialization
```cpp
int m[2][2] = {{1,2},{3,4}};
```

**Expected Output:**
- AST: Nested array structure with rows
- Symbol Table: `m[2][2]`
- TAC: 4 assignment instructions (one per element)

---

### 7. Array Element Assignment
```cpp
int arr[5];
arr[0] = 10;
```

**Expected Output:**
- AST: Declaration + Assignment nodes
- Symbol Table: `arr[5]`
- TAC: Array indexing and assignment

---

### 8. Multiple Variables
```cpp
int x = 1;
int y = 2;
```

**Expected Output:**
- AST: Two declaration nodes
- Symbol Table: Both `x` and `y`
- TAC: Two assignment instructions

---

## Advanced Examples

### 9. Expression Evaluation
```cpp
int x = 2 + 3 * 4;
```

**Expected Output:**
- AST: Expression tree with operator precedence
- Symbol Table: `x`
- TAC: Temporary variables for intermediate results

---

### 10. Array with Expression
```cpp
int arr[2] = {1 + 2, 3 * 4};
```

**Expected Output:**
- AST: Array with expression nodes
- Symbol Table: `arr[2]`
- TAC: Expression evaluation + array assignment

---

### 11. Complex 2D Array
```cpp
int big[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
```

**Expected Output:**
- AST: Large nested structure
- Symbol Table: `big[3][3]`
- TAC: 9 assignment instructions

---

### 12. Float Array
```cpp
float values[4] = {1.5, 2.5, 3.5, 4.5};
```

**Expected Output:**
- AST: Float type array
- Symbol Table: `values` of type `float`
- TAC: Float assignments

---

## Edge Cases

### 13. Empty Array Declaration
```cpp
int arr[0];
```

**Expected:** May produce error or warning

---

### 14. Large Array
```cpp
int huge[100];
```

**Expected Output:**
- AST: Single declaration
- Symbol Table: `huge[100]`
- TAC: Declaration only

---

### 15. Nested Expressions
```cpp
int x = (2 + 3) * (4 + 5);
```

**Expected Output:**
- AST: Complex expression tree
- Symbol Table: `x`
- TAC: Multiple temporary variables

---

## Testing Workflow

For each example:

1. **Copy the code** into the editor
2. **Click "Compile & Run"**
3. **Check AST tab**: Verify the tree structure
4. **Check Symbol Table**: Verify variable entries
5. **Check TAC**: Verify generated instructions

## Understanding the Output

### AST (Abstract Syntax Tree)
- Shows the hierarchical structure of your code
- Each node represents a language construct
- Colors indicate different node types

### Symbol Table
- Lists all declared variables
- Shows type information
- Indicates array dimensions

### TAC (Three-Address Code)
- Intermediate representation
- Each instruction has at most 3 operands
- Uses temporary variables (t1, t2, etc.)
- Shows array indexing calculations

## Common Patterns

### Pattern 1: Array Indexing
```
Input:  arr[2] = 5;
TAC:    t1 = 2 * 4
        arr[t1] = 5
```

### Pattern 2: Expression Evaluation
```
Input:  x = 2 + 3 * 4;
TAC:    t1 = 3 * 4
        t2 = 2 + t1
        x = t2
```

### Pattern 3: 2D Array Access
```
Input:  m[1][2] = 10;
TAC:    t1 = 1 * cols + 2
        t2 = t1 * 4
        m[t2] = 10
```

## Tips

- Start with simple examples and gradually increase complexity
- Compare the AST structure with your source code
- Notice how expressions are broken down in TAC
- Observe how array indexing is calculated
- Check that symbol table entries match your declarations

## Error Examples

Try these to see error handling:

```cpp
// Undeclared variable
x = 5;

// Type mismatch (if implemented)
int x = "string";

// Invalid syntax
int [5] arr;
```

Happy compiling! 🎉
