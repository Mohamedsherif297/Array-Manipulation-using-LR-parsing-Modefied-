// code_generator.cpp
// Recursive AST traversal that emits Three-Address Code (TAC) quads.
//
// Supported AST node types (as produced by the LR(1) parser and annotated
// by the semantic analyser):
//
//   Program        — root; iterates over statement children
//   Declaration    — variable / array declaration (no initialiser)
//   DeclAssign     — declaration with initialiser (scalar or array literal)
//   Assignment     — assignment to scalar variable or array element
//   +  -  *  /     — binary arithmetic operators
//   ID             — variable reference
//   NUM            — numeric literal
//   STRING         — string literal
//   CHAR           — character literal
//   ArrayAccess    — indexed array read  (ID [ Expr ] or ID [ Expr ][ Expr ])
//   Array          — array literal  { Row, Row, … }
//   ArrayRow       — one row of an array literal
//   Dimensions     — array dimension sizes (used only during declaration)
//   Expr/Term/Factor — transparent wrapper nodes (single child)

#include "code_generator.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

CodeGenerator::CodeGenerator(const CGSymbolTable& symTable)
    : symTable_(symTable) {}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

bool CodeGenerator::generate(shared_ptr<ASTNode> root) {
    if (!root) {
        error("Null AST root passed to code generator.");
        return false;
    }

    if (root->type == "Program") {
        genProgram(root);
    } else {
        genStatement(root);
    }

    return !hasError_;
}

bool CodeGenerator::writeIR(const string& filePath) const {
    ofstream file(filePath);
    if (!file.is_open()) {
        cerr << "[CodeGen] Cannot write IR to: " << filePath << "\n";
        return false;
    }
    printIR(file);
    cout << "[CodeGen] IR written to: " << filePath << "\n";
    return true;
}

void CodeGenerator::printIR(ostream& out) const {
    for (const Quad& q : ir_) {
        // Format each quad as a readable TAC line.
        if (q.op == "ASSIGN") {
            // result = arg1
            out << q.result << " = " << q.arg1 << "\n";
        } else if (q.op == "LOAD") {
            // result = arg1[arg2]   (array read)
            out << q.result << " = " << q.arg1 << "[" << q.arg2 << "]\n";
        } else if (q.op == "STORE") {
            // arg1[arg2] = result   (array write)
            out << q.arg1 << "[" << q.arg2 << "] = " << q.result << "\n";
        } else if (q.op == "DECL") {
            // Declaration annotation (no executable code)
            out << "// DECL " << q.result;
            if (!q.arg1.empty()) out << "  type=" << q.arg1;
            out << "\n";
        } else if (q.arg2.empty()) {
            // Unary:  result = op arg1
            out << q.result << " = " << q.op << " " << q.arg1 << "\n";
        } else {
            // Binary: result = arg1 op arg2
            out << q.result << " = " << q.arg1
                << " " << q.op << " " << q.arg2 << "\n";
        }
    }
}

// ---------------------------------------------------------------------------
// IR emission
// ---------------------------------------------------------------------------

void CodeGenerator::emit(const string& op,
                         const string& arg1,
                         const string& arg2,
                         const string& result) {
    ir_.emplace_back(op, arg1, arg2, result);
}

string CodeGenerator::newTemp() {
    return "t" + to_string(++tempCounter_);
}

void CodeGenerator::error(const string& msg) {
    cerr << "[CodeGen Error] " << msg << "\n";
    hasError_ = true;
}

// ---------------------------------------------------------------------------
// Statement visitors
// ---------------------------------------------------------------------------

void CodeGenerator::genProgram(shared_ptr<ASTNode> node) {
    for (auto& child : node->children)
        genStatement(child);
}

void CodeGenerator::genStatement(shared_ptr<ASTNode> node) {
    if (!node) return;

    const string& t = node->type;

    if (t == "Declaration")  { genDecl(node);       return; }
    if (t == "DeclAssign")   { genDeclAssign(node);  return; }
    if (t == "Assignment")   { genAssign(node);      return; }
    if (t == "Program")      { genProgram(node);     return; }

    // Transparent wrapper — recurse into children
    for (auto& child : node->children)
        genStatement(child);
}

// ---------------------------------------------------------------------------
// Declaration  (no initialiser)
//   children[0] = Type
//   children[1] = ID
//   children[2] = Dimensions  (optional, for arrays)
// ---------------------------------------------------------------------------

void CodeGenerator::genDecl(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;

    const string& varName = node->children[1]->value;
    const string& typeName = node->children[0]->value.empty()
                             ? node->children[0]->type
                             : node->children[0]->value;

    // Emit a DECL annotation so the IR is self-documenting.
    // No executable code is needed for a bare declaration.
    emit("DECL", typeName, "", varName);
}

// ---------------------------------------------------------------------------
// DeclAssign  (declaration with initialiser)
//   children[0] = Type
//   children[1] = ID
//   children[2] = Dimensions  (if array)  OR  Expr  (if scalar)
//   children[3] = Array node  (if array)
// ---------------------------------------------------------------------------

void CodeGenerator::genDeclAssign(shared_ptr<ASTNode> node) {
    if (node->children.size() < 3) return;

    const string& varName  = node->children[1]->value;
    const string& typeName = node->children[0]->value.empty()
                             ? node->children[0]->type
                             : node->children[0]->value;

    emit("DECL", typeName, "", varName);

    bool hasArrayDims = (node->children.size() >= 4 &&
                         (node->children[2]->type == "Dimensions" || 
                          node->children[2]->type == "ArraySize"));

    if (hasArrayDims) {
        // Array declaration with literal initialiser
        auto& arrNode = node->children[3]; // "Array" or "ArrayInit" node

        // Look up symbol to get dimension sizes
        auto it = symTable_.find(varName);
        if (it == symTable_.end()) {
            // Symbol not found - use default element size
            CGSymbol defaultSym;
            defaultSym.name = varName;
            defaultSym.type = typeName;
            defaultSym.isArray = true;
            defaultSym.size1 = 0;  // Will be determined by initializer
            defaultSym.size2 = 0;
            genArrayInit(node->children[1], arrNode, defaultSym);
        } else {
            genArrayInit(node->children[1], arrNode, it->second);
        }
    } else {
        // Scalar declaration with expression initialiser
        string rhs = genExpr(node->children[2]);
        if (!rhs.empty()) {
            emit("ASSIGN", rhs, "", varName);
        }
    }
}

// ---------------------------------------------------------------------------
// Assignment
//   children[0] = ID  or  ArrayAccess  (LHS)
//   children[1] = Expr                 (RHS)
// ---------------------------------------------------------------------------

void CodeGenerator::genAssign(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;

    auto& lhsNode = node->children[0];
    auto& rhsNode = node->children[1];

    string rhs = genExpr(rhsNode);

    if (lhsNode->type == "ID") {
        // Simple scalar assignment:  x = rhs
        emit("ASSIGN", rhs, "", lhsNode->value);

    } else if (lhsNode->type == "ArrayAccess") {
        // Array element assignment:  x[offset] = rhs
        //
        // ArrayAccess children:
        //   children[0] = ID   (array name)
        //   children[1] = Expr (first index)
        //   children[2] = Expr (second index, optional)

        if (lhsNode->children.size() < 2) {
            error("Malformed ArrayAccess node on LHS of assignment.");
            return;
        }

        const string& arrName = lhsNode->children[0]->value;

        auto it = symTable_.find(arrName);
        if (it == symTable_.end()) {
            error("Array '" + arrName + "' not found in symbol table.");
            return;
        }
        const CGSymbol& sym = it->second;
        int elemSize = sym.elementSize();

        if (lhsNode->children.size() >= 3) {
            // 2-D:  offset = (i * size2 + j) * elemSize
            string i = genExpr(lhsNode->children[1]);
            string j = genExpr(lhsNode->children[2]);

            string tRow  = newTemp();
            emit("*", i, to_string(sym.size2), tRow);   // tRow = i * size2

            string tFlat = newTemp();
            emit("+", tRow, j, tFlat);                   // tFlat = tRow + j

            string tOff  = newTemp();
            emit("*", tFlat, to_string(elemSize), tOff); // tOff = tFlat * elemSize

            emit("STORE", arrName, tOff, rhs);           // arrName[tOff] = rhs

        } else {
            // 1-D:  offset = index * elemSize
            string idx  = genExpr(lhsNode->children[1]);
            string tOff = newTemp();
            emit("*", idx, to_string(elemSize), tOff);   // tOff = idx * elemSize
            emit("STORE", arrName, tOff, rhs);            // arrName[tOff] = rhs
        }
    } else {
        error("Unsupported LHS node type in assignment: " + lhsNode->type);
    }
}

// ---------------------------------------------------------------------------
// Expression generator — returns the name of the value (temp, var, literal)
// ---------------------------------------------------------------------------

string CodeGenerator::genExpr(shared_ptr<ASTNode> node) {
    if (!node) return "";

    const string& t = node->type;

    // ---- Literals ----
    if (t == "NUM")    return node->value;
    if (t == "Number") return node->value;  // Handle "Number" node type
    if (t == "STRING") return "\"" + node->value + "\"";
    if (t == "CHAR")   return "'" + node->value + "'";

    // ---- Variable reference ----
    if (t == "ID") return node->value;

    // ---- Array element read ----
    if (t == "ArrayAccess") return genArrayAccess(node);

    // ---- Binary arithmetic operators ----
    if (t == "+" || t == "-" || t == "*" || t == "/")
        return genBinaryOp(node);

    // ---- Transparent wrapper nodes (Expr / Term / Factor) ----
    if (t == "Expr" || t == "Term" || t == "Factor") {
        if (!node->children.empty())
            return genExpr(node->children[0]);
        return "";
    }

    // ---- ArraySize and ArrayInit are not expressions, skip them ----
    if (t == "ArraySize" || t == "ArrayInit" || t == "Dimensions") {
        return "";
    }

    // ---- Fallback: recurse and return first non-empty result ----
    for (auto& child : node->children) {
        string v = genExpr(child);
        if (!v.empty()) return v;
    }

    error("Unhandled expression node type: " + t);
    return "";
}

// ---------------------------------------------------------------------------
// Binary operation:  result = left op right
// ---------------------------------------------------------------------------

string CodeGenerator::genBinaryOp(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) {
        error("Binary op '" + node->type + "' has fewer than 2 children.");
        return "";
    }

    string left  = genExpr(node->children[0]);
    string right = genExpr(node->children[1]);
    string temp  = newTemp();

    emit(node->type, left, right, temp);
    return temp;
}

// ---------------------------------------------------------------------------
// Array access (read):  result = array[offset]
//
// ArrayAccess children:
//   children[0] = ID   (array name)
//   children[1] = Expr (first index)
//   children[2] = Expr (second index, optional — 2-D arrays)
// ---------------------------------------------------------------------------

string CodeGenerator::genArrayAccess(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) {
        error("Malformed ArrayAccess node.");
        return "";
    }

    const string& arrName = node->children[0]->value;

    auto it = symTable_.find(arrName);
    if (it == symTable_.end()) {
        error("Array '" + arrName + "' not found in symbol table.");
        return "";
    }
    const CGSymbol& sym = it->second;
    int elemSize = sym.elementSize();

    string tOff;

    if (node->children.size() >= 3) {
        // 2-D access:  offset = (i * size2 + j) * elemSize
        string i = genExpr(node->children[1]);
        string j = genExpr(node->children[2]);

        string tRow  = newTemp();
        emit("*", i, to_string(sym.size2), tRow);    // tRow = i * size2

        string tFlat = newTemp();
        emit("+", tRow, j, tFlat);                    // tFlat = tRow + j

        tOff = newTemp();
        emit("*", tFlat, to_string(elemSize), tOff);  // tOff = tFlat * elemSize

    } else {
        // 1-D access:  offset = index * elemSize
        string idx = genExpr(node->children[1]);
        tOff = newTemp();
        emit("*", idx, to_string(elemSize), tOff);    // tOff = idx * elemSize
    }

    string tVal = newTemp();
    emit("LOAD", arrName, tOff, tVal);                // tVal = arrName[tOff]
    return tVal;
}

// ---------------------------------------------------------------------------
// Array literal initialisation
//
// For a 1-D array  int a[3] = {1, 2, 3}:
//   a[0] = 1
//   a[4] = 2
//   a[8] = 3
//
// For a 2-D array  int x[2][2] = {{1,2},{3,4}}:
//   x[0]  = 1
//   x[4]  = 2
//   x[8]  = 3
//   x[12] = 4
//
// The Array node has ArrayRow children; each ArrayRow has Expr children.
// For a 1-D array the grammar allows a bare Expr as a Row (rule 17), so
// we also handle ArrayRow nodes that are actually single expressions.
// ---------------------------------------------------------------------------

void CodeGenerator::genArrayInit(shared_ptr<ASTNode> idNode,
                                  shared_ptr<ASTNode> arrayNode,
                                  const CGSymbol&     sym) {
    const string& arrName = idNode->value;
    int elemSize = sym.elementSize();
    int flatIndex = 0; // linear element index

    // Handle both "Array" and "ArrayInit" node types
    if (arrayNode->type == "ArrayInit") {
        // Flat array initialization - all children are direct elements
        for (auto& elemNode : arrayNode->children) {
            string val = genExpr(elemNode);
            string tOff = newTemp();
            emit("*", to_string(flatIndex), to_string(elemSize), tOff);
            emit("STORE", arrName, tOff, val);
            ++flatIndex;
        }
        return;
    }

    // Original "Array" node handling with ArrayRow children
    for (auto& rowNode : arrayNode->children) {
        // Each child of Array is an ArrayRow (or a bare Expr for 1-D)
        if (rowNode->type == "ArrayRow") {
            for (auto& elemNode : rowNode->children) {
                string val = genExpr(elemNode);
                string tOff = newTemp();
                emit("*", to_string(flatIndex), to_string(elemSize), tOff);
                emit("STORE", arrName, tOff, val);
                ++flatIndex;
            }
        } else {
            // 1-D array: the row IS the element expression
            string val = genExpr(rowNode);
            string tOff = newTemp();
            emit("*", to_string(flatIndex), to_string(elemSize), tOff);
            emit("STORE", arrName, tOff, val);
            ++flatIndex;
        }
    }
}
