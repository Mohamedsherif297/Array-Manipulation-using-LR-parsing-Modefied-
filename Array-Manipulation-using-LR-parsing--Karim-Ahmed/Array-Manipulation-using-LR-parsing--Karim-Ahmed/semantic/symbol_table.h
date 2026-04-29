#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

using namespace std;

// Semantic symbol entry — richer than the lexer's SymbolTable
struct SemanticSymbol {
    string name;
    string type;        // "int", "float", "double", "char", "string"
    bool   isArray;
    int    size1;       // first dimension  (0 if not array)
    int    size2;       // second dimension (0 if 1-D or not array)
    string scope;       // "global" or "function(name)"
};

class SemanticSymbolTable {
public:
    // Returns true if inserted, false if name already exists (duplicate)
    bool declare(const SemanticSymbol& sym);

    // Returns nullptr if not found
    const SemanticSymbol* lookup(const string& name) const;

    bool isDeclared(const string& name) const;

    void display() const;

    // Iterate over all entries (for JSON output)
    const unordered_map<string, SemanticSymbol>& entries() const { return table_; }

private:
    unordered_map<string, SemanticSymbol> table_;
};
