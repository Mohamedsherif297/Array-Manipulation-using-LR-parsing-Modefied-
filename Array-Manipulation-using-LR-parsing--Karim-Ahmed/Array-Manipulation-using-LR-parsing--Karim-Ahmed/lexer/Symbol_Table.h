#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

using namespace std;

// Attributes associated with an identifier
struct SymbolAttributes {
    string type; // e.g., "int", "float", "double"
    // You can add scope or memory address here later
};

/**
 * SymbolTable: Manages the identifiers found in the source code.
 * Used for type checking and ensuring variables are declared before use.
 */
class SymbolTable {
private:
    unordered_map < string, SymbolAttributes > table;

public:
    // Adds a new identifier if it doesn't already exist
   public:
    void addSymbol(const string& name, const string& type = "unknown") {
        if (table.find(name) == table.end()) {
            table[name] = {type};
        }
    }
    void display() const {
        cout << "\n--- Symbol Table ---\n";
        for (const auto& pair : table) {
            cout << "ID: " << pair.first << " | Type: " << pair.second.type << "\n";
        }
    }
};
