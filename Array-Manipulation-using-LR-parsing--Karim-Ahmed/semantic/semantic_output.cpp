#include "semantic_output.h"

#include <fstream>
#include <iostream>

using namespace std;

// ---------------------------------------------------------------------------
// Annotated AST writer
// ---------------------------------------------------------------------------

static void writeNode(shared_ptr<ASTNode> node, int indent, ostream& out) {
    if (!node) return;

    string sp(indent, ' ');

    out << sp << "{\n";
    out << sp << "  \"type\": \"" << node->type << "\"";

    if (!node->value.empty()) {
        out << ",\n" << sp << "  \"value\": \"" << node->value << "\"";
    }

    if (node->line > 0) {
        out << ",\n" << sp << "  \"line\": " << node->line;
    }

    if (!node->dataType.empty()) {
        out << ",\n" << sp << "  \"dataType\": \"" << node->dataType << "\"";
    }

    if (!node->semanticInfo.empty()) {
        out << ",\n" << sp << "  \"semanticInfo\": \"" << node->semanticInfo << "\"";
    }

    if (!node->children.empty()) {
        out << ",\n" << sp << "  \"children\": [\n";
        for (size_t i = 0; i < node->children.size(); ++i) {
            writeNode(node->children[i], indent + 4, out);
            if (i + 1 < node->children.size()) out << ",";
            out << "\n";
        }
        out << sp << "  ]\n";
        out << sp << "}";
    } else {
        out << "\n" << sp << "}";
    }
}

void writeAnnotatedAST(shared_ptr<ASTNode> root, const string& filePath) {
    ofstream file(filePath);
    if (!file.is_open()) {
        cerr << "[Output] Cannot write annotated AST to: " << filePath << "\n";
        return;
    }
    writeNode(root, 0, file);
    file << "\n";
    cout << "[Output] Annotated AST written to: " << filePath << "\n";
}

// ---------------------------------------------------------------------------
// Symbol table writer
// ---------------------------------------------------------------------------

void writeSymbolTable(const SemanticSymbolTable& symTable, const string& filePath) {
    ofstream file(filePath);
    if (!file.is_open()) {
        cerr << "[Output] Cannot write symbol table to: " << filePath << "\n";
        return;
    }

    const auto& entries = symTable.entries();

    file << "{\n";
    size_t count = 0;
    for (const auto& [name, sym] : entries) {
        file << "  \"" << name << "\": {\n";
        file << "    \"type\": \""    << sym.type    << "\",\n";
        file << "    \"isArray\": "   << (sym.isArray ? "true" : "false");

        if (sym.isArray) {
            file << ",\n";
            file << "    \"size1\": " << sym.size1 << ",\n";
            file << "    \"size2\": " << sym.size2 << "\n";
        } else {
            file << "\n";
        }

        file << "  }";
        if (++count < entries.size()) file << ",";
        file << "\n";
    }
    file << "}\n";

    cout << "[Output] Symbol table written to: " << filePath << "\n";
}
