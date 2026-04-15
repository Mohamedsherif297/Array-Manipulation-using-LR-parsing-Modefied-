# Lexer Phase (Lexical Analysis)

## Overview
The lexer phase performs lexical analysis, converting raw source code into a stream of tokens.

## Files
- **Token_Class.h** - Token type definitions and Token class
- **Symbol_Table.h** - Basic symbol table for tracking identifiers
- **Lexer.h** - Main lexer implementation

## Functionality
- Tokenizes source code into meaningful units
- Recognizes keywords, identifiers, operators, literals, and delimiters
- Handles single-line (`//`) and multi-line (`/* */`) comments
- Builds initial symbol table with variable names and types
- Tracks line numbers for error reporting

## Supported Tokens
- **Data Types**: `int`, `float`, `double`, `char`, `string`
- **Operators**: `+`, `-`, `*`, `/`, `=`
- **Delimiters**: `[`, `]`, `{`, `}`, `(`, `)`, `;`, `,`
- **Literals**: Numbers, strings, characters
- **Identifiers**: Variable and array names

## Usage
```cpp
#include "lexer/Lexer.h"

string src = "int x = 5;";
SymbolTable st;
Lexer lexer(src, st);
vector<Token> tokens = lexer.tokenize();
```
