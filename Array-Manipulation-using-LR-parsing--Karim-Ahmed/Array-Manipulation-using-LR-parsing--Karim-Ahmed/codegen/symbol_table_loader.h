#pragma once
// symbol_table_loader.h
// Loads the symbol_table.json produced by the semantic phase into a simple
// in-memory map that the code generator can query.
//
// We intentionally do NOT depend on SemanticSymbolTable here so that the
// codegen module stays self-contained and does not pull in the full semantic
// analysis headers.

#include <string>
#include <unordered_map>

using namespace std;

// ---------------------------------------------------------------------------
// Lightweight symbol entry used by the code generator
// ---------------------------------------------------------------------------
struct CGSymbol {
    string name;
    string type;        // "int", "float", "double", "char", "string"
    bool   isArray = false;
    int    size1   = 0; // first  dimension (0 if scalar)
    int    size2   = 0; // second dimension (0 if 1-D or scalar)

    // Derived helpers
    int dimensions() const {
        if (!isArray) return 0;
        if (size2 > 0) return 2;
        return 1;
    }

    // Element size in bytes (used for offset calculation)
    int elementSize() const {
        if (type == "double") return 8;
        if (type == "float")  return 4;
        if (type == "char")   return 1;
        return 4; // int, string pointer, etc.
    }
};

using CGSymbolTable = unordered_map<string, CGSymbol>;

// Load symbol_table.json → CGSymbolTable.
// Returns an empty map on failure (errors printed to stderr).
CGSymbolTable loadSymbolTable(const string& filePath);
