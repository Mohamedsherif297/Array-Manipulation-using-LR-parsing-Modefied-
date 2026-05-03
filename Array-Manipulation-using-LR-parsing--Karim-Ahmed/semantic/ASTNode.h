#pragma once

#include <string>
#include <vector>
#include <memory>

using namespace std;

// ---------------------------------------------------------------------------
// Implicit type conversion log entry.
// Recorded whenever two operands of different numeric types are combined,
// e.g.  int + float  →  the int operand is promoted to float.
// ---------------------------------------------------------------------------
struct TypeConversion {
    string fromType;    // original type of the narrower operand  (e.g. "int")
    string toType;      // promoted / result type                  (e.g. "float")
    string context;     // short description, e.g. "int + float"
    int    line = 0;    // source line where the expression appears
};

// In-memory representation of an AST node loaded from JSON.
// Mirrors the structure produced by Parser.h's printJSON().
struct ASTNode {
    string type;                        // e.g. "DeclAssign", "ID", "NUM"
    string value;                       // leaf value, e.g. "x", "2"
    string op;                          // operator for compound assignments (+=, -=, *=, /=)
    int line = 0;                       // line number for error reporting
    vector<shared_ptr<ASTNode>> children;

    // Semantic annotations (filled during analysis)
    string dataType;                    // resolved type: "int", "float", "double", "char", "string", "unknown"
    string semanticInfo;                // extra annotation (e.g. "array_access", "declared_here")

    // Implicit conversion recorded on this node (non-empty when a promotion occurred)
    TypeConversion conversion;          // filled by SemanticAnalyzer when operands are widened
};
