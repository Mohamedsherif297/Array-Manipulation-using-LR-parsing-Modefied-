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

#include <algorithm>
#include <functional>
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

void CodeGenerator::genFunctionDef(shared_ptr<ASTNode> node) {
    // FunctionDef has: children[0] = return type, children[1] = name, children[2] = body
    if (node->children.size() < 3) return;
    
    // For now, just process the function body (StmtList)
    auto& body = node->children[2];
    if (body->type == "StmtList") {
        for (auto& stmt : body->children) {
            genStatement(stmt);
        }
    }
}

void CodeGenerator::genStatement(shared_ptr<ASTNode> node) {
    if (!node) return;

    const string& t = node->type;

    if (t == "FunctionDef")  { genFunctionDef(node); return; }
    if (t == "Declaration")  { genDecl(node);       return; }
    if (t == "DeclAssign")   { genDeclAssign(node);  return; }
    if (t == "Assignment")   { genAssign(node);      return; }
    if (t == "Program")      { genProgram(node);     return; }
    if (t == "StmtList")     {
        for (auto& child : node->children)
            genStatement(child);
        return;
    }

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

    // Look up symbol to get array metadata
    auto it = symTable_.find(varName);
    if (it != symTable_.end()) {
        const CGSymbol& sym = it->second;
        
        // Emit a DECL annotation with full metadata
        if (sym.isArray) {
            string arrayInfo = typeName;
            if (sym.size2 > 0) {
                arrayInfo += " array[" + to_string(sym.size1) + "][" + to_string(sym.size2) + "]";
            } else {
                arrayInfo += " array[" + to_string(sym.size1) + "]";
            }
            emit("DECL", arrayInfo, "", varName);
        } else {
            emit("DECL", typeName, "", varName);
        }
    } else {
        // Symbol not found in table, emit basic DECL
        emit("DECL", typeName, "", varName);
    }
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

    bool hasArrayDims = (node->children.size() >= 4 &&
                         (node->children[2]->type == "Dimensions" || 
                          node->children[2]->type == "ArraySize" ||
                          node->children[2]->type == "InferredSize" ||
                          node->children[2]->type == "Number"));  // Handle parser issue

    // Look up symbol to get array metadata
    auto it = symTable_.find(varName);
    
    if (hasArrayDims) {
        // Array declaration with literal initialiser
        auto& arrNode = node->children[3]; // "Array" or "ArrayInit" node

        if (it != symTable_.end()) {
            const CGSymbol& sym = it->second;
            
            // Emit DECL with full array metadata
            string arrayInfo = typeName;
            if (sym.size2 > 0) {
                arrayInfo += " array[" + to_string(sym.size1) + "][" + to_string(sym.size2) + "]";
            } else {
                arrayInfo += " array[" + to_string(sym.size1) + "]";
            }
            emit("DECL", arrayInfo, "", varName);
            
            genArrayInit(node->children[1], arrNode, sym);
        } else {
            // Symbol not found - use default
            emit("DECL", typeName, "", varName);
            
            CGSymbol defaultSym;
            defaultSym.name = varName;
            defaultSym.type = typeName;
            defaultSym.isArray = true;
            defaultSym.size1 = 0;
            defaultSym.size2 = 0;
            genArrayInit(node->children[1], arrNode, defaultSym);
        }
    } else {
        // Scalar declaration with expression initialiser
        if (it != symTable_.end()) {
            emit("DECL", typeName, "", varName);
        } else {
            emit("DECL", typeName, "", varName);
        }
        
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
        // ArrayAccess can now be recursive (for multi-dimensional arrays)
        // We need to extract the array name and all indices

        // Collect all indices by traversing the recursive ArrayAccess structure
        vector<string> indices;
        vector<shared_ptr<ASTNode>> indexNodes;  // Keep nodes for bounds checking
        shared_ptr<ASTNode> current = lhsNode;
        string arrName;

        // Traverse down the ArrayAccess chain to collect indices
        while (current && current->type == "ArrayAccess") {
            if (current->children.size() < 2) {
                error("Malformed ArrayAccess node on LHS of assignment.");
                return;
            }
            
            // The last child is always the index for this level
            indexNodes.push_back(current->children.back());
            indices.push_back(genExpr(current->children.back()));
            
            // Move to the next level (children[0])
            if (current->children[0]->type == "ID") {
                // Base case: we've reached the array name
                arrName = current->children[0]->value;
                break;
            } else if (current->children[0]->type == "ArrayAccess") {
                // Recursive case: continue traversing
                current = current->children[0];
            } else {
                error("Unexpected node type in ArrayAccess chain: " + current->children[0]->type);
                return;
            }
        }

        if (arrName.empty()) {
            error("Could not find array name in ArrayAccess chain.");
            return;
        }

        // Reverse indices because we collected them from innermost to outermost
        reverse(indices.begin(), indices.end());
        reverse(indexNodes.begin(), indexNodes.end());

        auto it = symTable_.find(arrName);
        if (it == symTable_.end()) {
            error("Array '" + arrName + "' not found in symbol table.");
            return;
        }
        const CGSymbol& sym = it->second;

        // Bounds checking for constant indices
        for (size_t i = 0; i < indexNodes.size(); ++i) {
            auto& idxNode = indexNodes[i];
            if (idxNode->type == "NUM" || idxNode->type == "Number") {
                try {
                    int constIndex = stoi(idxNode->value);
                    int bound = (i == 0) ? sym.size1 : sym.size2;
                    
                    if (constIndex < 0) {
                        error("Array index cannot be negative: " + arrName + 
                              "[" + to_string(constIndex) + "]");
                    } else if (constIndex >= bound) {
                        error("Array index out of bounds: " + arrName + 
                              "[" + to_string(constIndex) + "] (size is " + 
                              to_string(bound) + ")");
                    }
                } catch (...) {
                    // Not a valid integer, skip bounds check
                }
            }
        }

        string tOff;

        if (indices.size() == 1) {
            // 1-D: Always compute offset explicitly
            // offset = index
            tOff = indices[0];
            emit("STORE", arrName, tOff, rhs);                   // arrName[index] = rhs

        } else if (indices.size() == 2) {
            // 2-D: Always compute offset explicitly
            // offset = i * num_cols + j
            string tRow  = newTemp();
            emit("*", indices[0], to_string(sym.size2), tRow);   // tRow = i * size2

            tOff = newTemp();
            emit("+", tRow, indices[1], tOff);                   // tOff = tRow + j

            emit("STORE", arrName, tOff, rhs);                   // arrName[tOff] = rhs

        } else {
            error("Array assignment with " + to_string(indices.size()) + " dimensions not supported.");
            return;
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
    
    // STANDARDIZED CHAR AND STRING HANDLING:
    // - CHAR: represented as ASCII integer value (e.g., 'A' → 65)
    // - STRING: represented as quoted string literal (e.g., "hello")
    if (t == "STRING") return "\"" + node->value + "\"";
    if (t == "CHAR") {
        // Convert char to ASCII integer for consistent internal representation
        if (!node->value.empty()) {
            int asciiValue = static_cast<int>(node->value[0]);
            return to_string(asciiValue);
        }
        return "0";  // Default for empty char
    }

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

    // ---- ArraySize, ArrayInit, Dimensions, and InferredSize are not expressions, skip them ----
    if (t == "ArraySize" || t == "ArrayInit" || t == "Dimensions" || t == "InferredSize") {
        return "";
    }

    // ---- Array structure nodes are not expressions, skip them ----
    if (t == "ArrayElements" || t == "ArrayElement" || t == ",") {
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
// ArrayAccess can now be recursive:
//   For x[0]:     children[0] = ID(x), children[1] = Expr(0)
//   For x[0][0]:  children[0] = ArrayAccess(x[0]), children[1] = Expr(0)
//
// UNIFIED APPROACH:
// - Always compute index explicitly for ALL arrays (1D and 2D)
// - For 1D: offset = index
// - For 2D: offset = i * num_cols + j
// - Add bounds checking for constant indices
// ---------------------------------------------------------------------------

string CodeGenerator::genArrayAccess(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) {
        error("Malformed ArrayAccess node.");
        return "";
    }

    // Collect all indices by traversing the recursive ArrayAccess structure
    vector<string> indices;
    vector<shared_ptr<ASTNode>> indexNodes;  // Keep nodes for bounds checking
    shared_ptr<ASTNode> current = node;
    string arrName;

    // Traverse down the ArrayAccess chain to collect indices
    while (current && current->type == "ArrayAccess") {
        if (current->children.size() < 2) {
            error("Malformed ArrayAccess node in chain.");
            return "";
        }
        
        // The last child is always the index for this level
        indexNodes.push_back(current->children.back());
        indices.push_back(genExpr(current->children.back()));
        
        // Move to the next level (children[0])
        if (current->children[0]->type == "ID") {
            // Base case: we've reached the array name
            arrName = current->children[0]->value;
            break;
        } else if (current->children[0]->type == "ArrayAccess") {
            // Recursive case: continue traversing
            current = current->children[0];
        } else {
            error("Unexpected node type in ArrayAccess chain: " + current->children[0]->type);
            return "";
        }
    }

    if (arrName.empty()) {
        error("Could not find array name in ArrayAccess chain.");
        return "";
    }

    // Reverse indices because we collected them from innermost to outermost
    reverse(indices.begin(), indices.end());
    reverse(indexNodes.begin(), indexNodes.end());

    // Look up symbol
    auto it = symTable_.find(arrName);
    if (it == symTable_.end()) {
        error("Array '" + arrName + "' not found in symbol table.");
        return "";
    }
    const CGSymbol& sym = it->second;

    // Bounds checking for constant indices
    for (size_t i = 0; i < indexNodes.size(); ++i) {
        auto& idxNode = indexNodes[i];
        if (idxNode->type == "NUM" || idxNode->type == "Number") {
            try {
                int constIndex = stoi(idxNode->value);
                int bound = (i == 0) ? sym.size1 : sym.size2;
                
                if (constIndex < 0) {
                    error("Array index cannot be negative: " + arrName + 
                          "[" + to_string(constIndex) + "]");
                } else if (constIndex >= bound) {
                    error("Array index out of bounds: " + arrName + 
                          "[" + to_string(constIndex) + "] (size is " + 
                          to_string(bound) + ")");
                }
            } catch (...) {
                // Not a valid integer, skip bounds check
            }
        }
    }

    string tOff;

    if (indices.size() == 1) {
        // 1-D access: Always compute offset explicitly
        // offset = index
        tOff = indices[0];
    } else if (indices.size() == 2) {
        // 2-D access: Always compute offset explicitly
        // offset = i * num_cols + j
        string tRow  = newTemp();
        emit("*", indices[0], to_string(sym.size2), tRow);    // tRow = i * size2

        tOff = newTemp();
        emit("+", tRow, indices[1], tOff);                    // tOff = tRow + j
    } else {
        error("Array access with " + to_string(indices.size()) + " dimensions not supported.");
        return "";
    }

    string tVal = newTemp();
    emit("LOAD", arrName, tOff, tVal);                         // tVal = arrName[tOff]
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
// The new grammar uses:
//   ArrayLiteral → { ArrayElements }
//   ArrayElements → ArrayElement | ArrayElements , ArrayElement
//   ArrayElement → Expr | ArrayLiteral
//
// This allows recursive nesting for multi-dimensional arrays.
// ---------------------------------------------------------------------------

void CodeGenerator::genArrayInit(shared_ptr<ASTNode> idNode,
                                  shared_ptr<ASTNode> arrayNode,
                                  const CGSymbol&     sym) {
    const string& arrName = idNode->value;
    int elemSize = sym.elementSize();
    int flatIndex = 0; // linear element index

    // Recursive helper to flatten nested array literals
    function<void(shared_ptr<ASTNode>)> flattenArray = [&](shared_ptr<ASTNode> node) {
        if (!node) return;

        // Handle ArrayLiteral node
        if (node->type == "ArrayLiteral") {
            // ArrayLiteral has one child: ArrayElements
            if (!node->children.empty()) {
                flattenArray(node->children[0]);
            }
            return;
        }

        // Handle ArrayElements node
        if (node->type == "ArrayElements") {
            // Process all children (each is an ArrayElement)
            for (auto& child : node->children) {
                flattenArray(child);
            }
            return;
        }

        // Handle ArrayElement node
        if (node->type == "ArrayElement") {
            // ArrayElement can be either an Expr or a nested ArrayLiteral
            if (!node->children.empty()) {
                auto& child = node->children[0];
                if (child->type == "ArrayLiteral") {
                    // Nested array literal - recurse
                    flattenArray(child);
                } else if (child->type == "ArrayInit") {
                    // Nested ArrayInit - recurse
                    flattenArray(child);
                } else {
                    // Expression - generate code
                    string val = genExpr(child);
                    if (!val.empty()) {
                        emit("STORE", arrName, to_string(flatIndex), val);  // Direct index, no byte offset
                        ++flatIndex;
                    }
                }
            }
            return;
        }

        // Handle legacy ArrayInit node (flat array)
        if (node->type == "ArrayInit") {
            for (auto& elemNode : node->children) {
                // Skip comma nodes
                if (elemNode->type == ",") {
                    continue;
                }
                // Handle ArrayElements
                if (elemNode->type == "ArrayElements") {
                    flattenArray(elemNode);
                    continue;
                }
                // Handle nested ArrayInit
                if (elemNode->type == "ArrayInit") {
                    flattenArray(elemNode);
                    continue;
                }
                // Handle ArrayElement
                if (elemNode->type == "ArrayElement") {
                    flattenArray(elemNode);
                    continue;
                }
                // Otherwise treat as expression
                string val = genExpr(elemNode);
                if (!val.empty()) {
                    emit("STORE", arrName, to_string(flatIndex), val);  // Direct index, no byte offset
                    ++flatIndex;
                }
            }
            return;
        }

        // Handle legacy ArrayRow node
        if (node->type == "ArrayRow") {
            for (auto& elemNode : node->children) {
                string val = genExpr(elemNode);
                if (!val.empty()) {
                    emit("STORE", arrName, to_string(flatIndex), val);  // Direct index, no byte offset
                    ++flatIndex;
                }
            }
            return;
        }

        // Handle legacy Array node
        if (node->type == "Array") {
            for (auto& rowNode : node->children) {
                flattenArray(rowNode);
            }
            return;
        }

        // If it's a direct expression, generate code
        string val = genExpr(node);
        if (!val.empty()) {
            emit("STORE", arrName, to_string(flatIndex), val);  // Direct index, no byte offset
            ++flatIndex;
        }
    };

    // Start flattening from the root array node
    flattenArray(arrayNode);
}
