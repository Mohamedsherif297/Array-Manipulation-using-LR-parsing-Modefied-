#pragma once

#include <string>
#include <vector>
#include <memory>

using namespace std;

// In-memory representation of an AST node loaded from JSON.
// Mirrors the structure produced by Parser.h's printJSON().
struct ASTNode {
    string type;                        // e.g. "DeclAssign", "ID", "NUM"
    string value;                       // leaf value, e.g. "x", "2"
    int line = 0;                       // line number for error reporting
    vector<shared_ptr<ASTNode>> children;

    // Semantic annotations (filled during analysis)
    string dataType;                    // resolved type: "int", "float", "double", "char", "string", "unknown"
    string semanticInfo;                // extra annotation (e.g. "array_access", "declared_here")
};
