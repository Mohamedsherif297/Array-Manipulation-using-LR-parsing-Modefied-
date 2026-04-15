#include "semantic_analyzer.h"
#include <iostream>
#include <stdexcept>

using namespace std;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

SemanticAnalyzer::SemanticAnalyzer(SemanticSymbolTable& symTable)
    : symTable_(symTable) {}

void SemanticAnalyzer::addError(const string& msg, const ASTNode& node) {
    errors_.push_back({msg, node.type, node.value});
    cerr << "[Semantic Error] " << msg
         << "  (node=" << node.type;
    if (!node.value.empty()) cerr << ", value=" << node.value;
    cerr << ")\n";
}

bool SemanticAnalyzer::isNumericType(const string& t) {
    return t == "int" || t == "float" || t == "double";
}

bool SemanticAnalyzer::isIntegerType(const string& t) {
    return t == "int";
}

// Widening: int < float < double.  Non-numeric types must match exactly.
string SemanticAnalyzer::resolveType(const string& t1, const string& t2) {
    if (t1 == t2) return t1;
    if (t1 == "unknown" || t2 == "unknown") return "unknown";
    if (isNumericType(t1) && isNumericType(t2)) {
        if (t1 == "double" || t2 == "double") return "double";
        if (t1 == "float"  || t2 == "float")  return "float";
        return "int";
    }
    return "mismatch";
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

bool SemanticAnalyzer::analyze(shared_ptr<ASTNode> root) {
    if (!root) return false;
    if (root->type == "Program") {
        visitProgram(root);
    } else {
        visitStatement(root);
    }
    return errors_.empty();
}

// ---------------------------------------------------------------------------
// Program / Statement dispatch
// ---------------------------------------------------------------------------

void SemanticAnalyzer::visitProgram(shared_ptr<ASTNode> node) {
    node->dataType = "void";
    for (auto& child : node->children)
        visitStatement(child);
}

void SemanticAnalyzer::visitStatement(shared_ptr<ASTNode> node) {
    if (!node) return;

    if (node->type == "Declaration")   { visitDecl(node);       return; }
    if (node->type == "DeclAssign")    { visitDeclAssign(node);  return; }
    if (node->type == "Assignment")    { visitAssign(node);      return; }
    if (node->type == "Program")       { visitProgram(node);     return; }

    // Fallback: recurse into children
    for (auto& child : node->children)
        visitStatement(child);
}

// ---------------------------------------------------------------------------
// Declaration:  Type  ID  [ArrayDims]  ;
//   children[0] = Type node
//   children[1] = ID   node
//   children[2] = Dimensions node (optional)
// ---------------------------------------------------------------------------

void SemanticAnalyzer::visitDecl(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;

    string typeName = node->children[0]->value.empty()
                      ? node->children[0]->type
                      : node->children[0]->value;
    string varName  = node->children[1]->value;

    node->children[0]->dataType = typeName;
    node->children[1]->dataType = typeName;

    SemanticSymbol sym;
    sym.name    = varName;
    sym.type    = typeName;
    sym.isArray = false;
    sym.size1   = 0;
    sym.size2   = 0;

    // Optional Dimensions node
    if (node->children.size() >= 3 && node->children[2]->type == "Dimensions") {
        auto& dims = node->children[2];
        sym.isArray = true;
        sym.size1   = (dims->children.size() >= 1) ? stoi(dims->children[0]->value) : 0;
        sym.size2   = (dims->children.size() >= 2) ? stoi(dims->children[1]->value) : 0;
        dims->dataType = "int"; // dimensions are always integer
        for (auto& d : dims->children) d->dataType = "int";
    }

    if (!symTable_.declare(sym)) {
        addError("Duplicate declaration of variable '" + varName + "'", *node->children[1]);
        node->semanticInfo = "duplicate";
    } else {
        node->semanticInfo = "declared_here";
    }

    node->dataType = typeName;
}

// ---------------------------------------------------------------------------
// DeclAssign:  Type  ID  [Dims]  =  Expr/ArrayLiteral  ;
//   children[0] = Type
//   children[1] = ID
//   children[2] = Dimensions  (if array)  OR  Expr  (if scalar)
//   children[3] = Array node  (if array)
// ---------------------------------------------------------------------------

void SemanticAnalyzer::visitDeclAssign(shared_ptr<ASTNode> node) {
    if (node->children.size() < 3) return;

    string typeName = node->children[0]->value.empty()
                      ? node->children[0]->type
                      : node->children[0]->value;
    string varName  = node->children[1]->value;

    node->children[0]->dataType = typeName;
    node->children[1]->dataType = typeName;

    SemanticSymbol sym;
    sym.name    = varName;
    sym.type    = typeName;
    sym.isArray = false;
    sym.size1   = 0;
    sym.size2   = 0;

    bool hasArrayDims = (node->children.size() >= 4 &&
                         node->children[2]->type == "Dimensions");

    if (hasArrayDims) {
        auto& dims = node->children[2];
        sym.isArray = true;
        sym.size1   = (dims->children.size() >= 1) ? stoi(dims->children[0]->value) : 0;
        sym.size2   = (dims->children.size() >= 2) ? stoi(dims->children[1]->value) : 0;
        dims->dataType = "int";
        for (auto& d : dims->children) d->dataType = "int";
    }

    if (!symTable_.declare(sym)) {
        addError("Duplicate declaration of variable '" + varName + "'", *node->children[1]);
        node->semanticInfo = "duplicate";
    } else {
        node->semanticInfo = "declared_here";
    }

    // Annotate the RHS
    if (hasArrayDims) {
        // children[3] is the Array literal
        auto& arrNode = node->children[3];
        arrNode->dataType = typeName;
        // Validate each row
        for (auto& row : arrNode->children) {
            row->dataType = typeName;
            for (auto& elem : row->children) {
                string elemType = visitExpr(elem);
                string resolved = resolveType(typeName, elemType);
                if (resolved == "mismatch") {
                    addError("Type mismatch in array literal: expected '" + typeName +
                             "' but got '" + elemType + "'", *elem);
                }
            }
        }
    } else {
        // Scalar assignment — children[2] is the Expr
        string rhsType = visitExpr(node->children[2]);
        string resolved = resolveType(typeName, rhsType);
        if (resolved == "mismatch") {
            addError("Type mismatch in declaration: cannot assign '" + rhsType +
                     "' to variable of type '" + typeName + "'", *node->children[1]);
        }
    }

    node->dataType = typeName;
}

// ---------------------------------------------------------------------------
// Assignment:  LHS  =  Expr
//   children[0] = ID or ArrayAccess
//   children[1] = Expr
// ---------------------------------------------------------------------------

void SemanticAnalyzer::visitAssign(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;

    auto& lhsNode = node->children[0];
    auto& rhsNode = node->children[1];

    string lhsType;

    if (lhsNode->type == "ID") {
        string varName = lhsNode->value;
        const SemanticSymbol* sym = symTable_.lookup(varName);
        if (!sym) {
            addError("Undeclared variable '" + varName + "'", *lhsNode);
            lhsType = "unknown";
        } else {
            if (sym->isArray) {
                addError("Cannot assign to array '" + varName +
                         "' without index", *lhsNode);
            }
            lhsType = sym->type;
        }
        lhsNode->dataType = lhsType;
    } else if (lhsNode->type == "ArrayAccess") {
        lhsType = visitArrayAccess(lhsNode);
    } else {
        lhsType = visitExpr(lhsNode);
    }

    string rhsType = visitExpr(rhsNode);
    string resolved = resolveType(lhsType, rhsType);

    if (resolved == "mismatch") {
        addError("Type mismatch in assignment: cannot assign '" + rhsType +
                 "' to '" + lhsType + "'", *node);
    }

    node->dataType = lhsType;
}

// ---------------------------------------------------------------------------
// Expression evaluator — returns the resolved type and annotates the node
// ---------------------------------------------------------------------------

string SemanticAnalyzer::visitExpr(shared_ptr<ASTNode> node) {
    if (!node) return "unknown";

    // Leaf: numeric literal
    if (node->type == "NUM") {
        // Determine int vs float by presence of '.'
        string t = (node->value.find('.') != string::npos) ? "float" : "int";
        node->dataType = t;
        return t;
    }

    // Leaf: string literal
    if (node->type == "STRING") {
        node->dataType = "string";
        return "string";
    }

    // Leaf: char literal
    if (node->type == "CHAR") {
        node->dataType = "char";
        return "char";
    }

    // Leaf: identifier
    if (node->type == "ID") {
        string varName = node->value;
        const SemanticSymbol* sym = symTable_.lookup(varName);
        if (!sym) {
            addError("Undeclared variable '" + varName + "'", *node);
            node->dataType = "unknown";
            return "unknown";
        }
        if (sym->isArray) {
            addError("Array '" + varName + "' used without index", *node);
            node->dataType = "unknown";
            return "unknown";
        }
        node->dataType = sym->type;
        return sym->type;
    }

    // Array access
    if (node->type == "ArrayAccess") {
        string t = visitArrayAccess(node);
        return t;
    }

    // Binary expression node produced by the parser:
    // The parser collapses Expr/Term/Factor chains, so a binary op node
    // has type "+" / "-" / "*" / "/" with two children.
    if (node->type == "+" || node->type == "-" ||
        node->type == "*" || node->type == "/") {

        if (node->children.size() < 2) {
            node->dataType = "unknown";
            return "unknown";
        }
        string t1 = visitExpr(node->children[0]);
        string t2 = visitExpr(node->children[1]);
        string resolved = resolveType(t1, t2);

        if (resolved == "mismatch") {
            addError("Type mismatch in expression: '" + t1 +
                     "' " + node->type + " '" + t2 + "'", *node);
            resolved = "unknown";
        }
        node->dataType = resolved;
        return resolved;
    }

    // Parenthesised expression — single child
    if (node->type == "Expr" || node->type == "Term" || node->type == "Factor") {
        if (!node->children.empty()) {
            string t = visitExpr(node->children[0]);
            node->dataType = t;
            return t;
        }
    }

    // Fallback: recurse and return first resolved child type
    string result = "unknown";
    for (auto& child : node->children) {
        string t = visitExpr(child);
        if (t != "unknown") result = t;
    }
    node->dataType = result;
    return result;
}

// ---------------------------------------------------------------------------
// ArrayAccess:  ID [ Expr ]
//   children[0] = ID
//   children[1] = index Expr
// ---------------------------------------------------------------------------

string SemanticAnalyzer::visitArrayAccess(shared_ptr<ASTNode> node) {
    node->semanticInfo = "array_access";

    if (node->children.size() < 2) {
        node->dataType = "unknown";
        return "unknown";
    }

    auto& idNode  = node->children[0];
    auto& idxNode = node->children[1];

    string varName = idNode->value;
    const SemanticSymbol* sym = symTable_.lookup(varName);

    if (!sym) {
        addError("Undeclared variable '" + varName + "'", *idNode);
        idNode->dataType  = "unknown";
        node->dataType    = "unknown";
        return "unknown";
    }

    if (!sym->isArray) {
        addError("Variable '" + varName + "' is not an array", *idNode);
        idNode->dataType = sym->type;
        node->dataType   = "unknown";
        return "unknown";
    }

    idNode->dataType = sym->type;

    // Validate index type — must be integer
    string idxType = visitExpr(idxNode);
    if (!isIntegerType(idxType) && idxType != "unknown") {
        addError("Array index for '" + varName +
                 "' must be an integer, got '" + idxType + "'", *idxNode);
    }

    node->dataType = sym->type;
    return sym->type;
}
