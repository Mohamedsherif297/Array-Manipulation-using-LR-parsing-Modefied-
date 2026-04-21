1- int main (){} into all phases . ✅
2-error report generation 
3-GUI
4- type promoting from char to string ✅
5- conncatenating in Strings . ✅
6- handeling strings as array of charachters . ✅
7- prevent self-initialization (e.g., char y = y;) ✅
8- reject array declaration with empty brackets but no initializer (e.g., int bad[];) ✅
9- Code Generator Improvements ✅
   - Unified array indexing (always compute explicitly)
   - Store array sizes for inferred arrays
   - Bounds checking for constant indices
   - Consistent multi-dimensional indexing (i * num_cols + j)
   - Standardized CHAR (ASCII int) and STRING (quoted) handling
   - Declarations aligned with symbol table metadata  