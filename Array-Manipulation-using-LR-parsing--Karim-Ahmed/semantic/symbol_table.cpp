#include "symbol_table.h"

bool SemanticSymbolTable::declare(const SemanticSymbol& sym) {
    if (table_.count(sym.name)) return false; // duplicate
    table_[sym.name] = sym;
    return true;
}

const SemanticSymbol* SemanticSymbolTable::lookup(const string& name) const {
    auto it = table_.find(name);
    if (it == table_.end()) return nullptr;
    return &it->second;
}

bool SemanticSymbolTable::isDeclared(const string& name) const {
    return table_.count(name) > 0;
}

void SemanticSymbolTable::display() const {
    cout << "\n--- Semantic Symbol Table ---\n";
    for (auto& [name, sym] : table_) {
        cout << "  " << name
             << " | type: " << sym.type
             << " | isArray: " << (sym.isArray ? "true" : "false");
        if (sym.isArray) {
            cout << " | size1: " << sym.size1
                 << " | size2: " << sym.size2;
        }
        cout << "\n";
    }
}
