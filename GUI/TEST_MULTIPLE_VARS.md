# Testing Multiple Variable Declarations

## Test Cases

### Test 1: Two simple declarations
```cpp
int x = 5;
int y = 10;
```

### Test 2: Mixed declarations
```cpp
int x = 5;
float y = 3.14;
int z = x + 10;
```

### Test 3: Array and scalar
```cpp
int arr[3] = {1,2,3};
int x = 5;
```

### Test 4: Multiple arrays
```cpp
int a[2] = {1,2};
int b[2] = {3,4};
```

## Expected Behavior

All test cases should:
1. Parse successfully
2. Generate AST with multiple children in Program node
3. Pass semantic analysis
4. Generate TAC for all declarations

## If You See Errors

Please note:
- The exact error message
- Which test case fails
- At which phase (parsing, semantic, codegen)
