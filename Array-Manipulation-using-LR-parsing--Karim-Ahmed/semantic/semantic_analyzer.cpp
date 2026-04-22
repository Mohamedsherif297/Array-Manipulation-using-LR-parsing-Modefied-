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
    errors_.push_back({msg, node.type, node.value, node.line});
    cerr << "[Semantic Error] " << msg
         << "  (node=" << node.type;
    if (!node.value.empty()) cerr << ", value=" << node.value;
    if (node.line > 0) cerr << ", line=" << node.line;
    cerr << ")\n";
}

bool SemanticAnalyzer::isNumericType(const string& t) {
    return t == "int" || t == "float" || t == "double";
}

bool SemanticAnalyzer::isIntegerType(const string& t) {
    return t == "int";
}

bool SemanticAnalyzer::isStringType(const string& t) {
    return t == "string";
}

bool SemanticAnalyzer::isCharType(const string& t) {
    return t == "char";
}

// Widening: int < float < double.  Char can be promoted to string.
string SemanticAnalyzer::resolveType(const string& t1, const string& t2) {
    if (t1 == t2) return t1;
    if (t1 == "unknown" || t2 == "unknown") return "unknown";
    
    // Numeric type widening
    if (isNumericType(t1) && isNumericType(t2)) {
        if (t1 == "double" || t2 == "double") return "double";
        if (t1 == "float"  || t2 == "float")  return "float";
        return "int";
    }
    
    // Char to string promotion
    if ((t1 == "string" && t2 == "char") || (t1 == "char" && t2 == "string")) {
        return "string";
    }
    
    return "mismatch";
}

// Strict assignment checking - allow safe implicit conversions
bool SemanticAnalyzer::isAssignmentCompatible(const string& lhsType, const string& rhsType) {
    if (lhsType == rhsType) return true;
    if (lhsType == "unknown" || rhsType == "unknown") return true; // Don't double-report errors
    
    // Reject assignment of array types to scalar variables
    if (rhsType == "array") return false;
    
    // Allow safe numeric conversions
    if (lhsType == "float" && rhsType == "int") return true;   // int -> float is safe
    if (lhsType == "double" && (rhsType == "int" || rhsType == "float")) return true; // int/float -> double is safe
    
    // Allow char to string promotion
    if (lhsType == "string" && rhsType == "char") return true;
    
    return false; // All other conversions are not allowed
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

bool SemanticAnalyzer::analyze(shared_ptr<ASTNode> root) {
    if (!root) return false;
    
    if (root->type == "FunctionDef") {
        visitFunctionDef(root);
    } else if (root->type == "Program") {
        visitProgram(root);
    } else {
        visitStatement(root);
    }
    return errors_.empty();
}

// ---------------------------------------------------------------------------
// Program / Statement dispatch
// ---------------------------------------------------------------------------

void SemanticAnalyzer::visitFunctionDef(shared_ptr<ASTNode> node) {
    // FunctionDef has: children[0] = return type, children[1] = name, children[2] = body
    if (node->children.size() < 3) {
        addError("Malformed function definition", *node);
        return;
    }
    
    string returnType = node->children[0]->value.empty() 
                        ? node->children[0]->type 
                        : node->children[0]->value;
    string funcName = node->children[1]->value;
    
    // Check if function is main()
    if (funcName != "main") {
        addError("Function name must be 'main' - found '" + funcName + "'", *node->children[1]);
    }
    
    // Check if main() returns int
    if (funcName == "main" && returnType != "int") {
        addError("main() must return 'int', not '" + returnType + "'", *node->children[0]);
    }
    
    node->dataType = returnType;
    node->children[0]->dataType = returnType;
    node->children[1]->dataType = returnType;
    
    // Track if we find a return statement
    bool hasReturn = false;
    
    // Visit the function body (StmtList)
    auto& body = node->children[2];
    if (body->type == "StmtList") {
        for (auto& stmt : body->children) {
            if (stmt->type == "Return") {
                hasReturn = true;
            }
            visitStatement(stmt);
        }
    }
    
    // Check if main() has a return statement
    if (funcName == "main" && !hasReturn) {
        addError("main() function must have a return statement", *node->children[1]);
    }
}

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
    if (node->type == "Output")        { visitOutput(node);      return; }
    if (node->type == "Input")         { visitInput(node);       return; }
    if (node->type == "Return")        { visitReturn(node);      return; }
    if (node->type == "Program")       { visitProgram(node);     return; }
    if (node->type == "FunctionDef")   { visitFunctionDef(node); return; }
    if (node->type == "StmtList")      { 
        for (auto& child : node->children)
            visitStatement(child);
        return;
    }

    // Fallback: recurse into children
    for (auto& child : node->children)
        visitStatement(child);
}

void SemanticAnalyzer::visitOutput(shared_ptr<ASTNode> node) {
    // cout << expr << expr ...
    for (auto& child : node->children) {
        visitExpr(child);
    }
    node->dataType = "void";
}

void SemanticAnalyzer::visitInput(shared_ptr<ASTNode> node) {
    // cin >> target >> target ...
    for (auto& child : node->children) {
        if (child->type == "ID") {
            const auto* sym = symTable_.lookup(child->value);
            if (!sym) {
                addError("Undeclared variable '" + child->value + "'", *child);
                child->dataType = "unknown";
            } else if (sym->isArray) {
                addError("Array '" + child->value + "' used without index", *child);
                child->dataType = "unknown";
            } else {
                child->dataType = sym->type;
            }
        } else if (child->type == "ArrayAccess") {
            visitArrayAccess(child);
        } else {
            addError("Invalid input target in cin statement", *child);
        }
    }
    node->dataType = "void";
}

// ---------------------------------------------------------------------------
// Declaration:  Type  ID  [ArrayDims]  ;
//   children[0] = Type
//   children[1] = ID
//   children[2] = Dimensions  (optional, for arrays)
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
    if (node->children.size() >= 3 && 
        (node->children[2]->type == "Dimensions" || 
         node->children[2]->type == "ArraySize" ||
         node->children[2]->type == "InferredSize" ||
         node->children[2]->type == "Number")) {
        auto& dims = node->children[2];
        sym.isArray = true;
        
        // Check for invalid empty brackets without initializer
        if (dims->type == "InferredSize") {
            addError("Array '" + varName + "' declared with empty brackets [] must have an initializer", *dims);
            sym.size1 = 0;
            sym.size2 = 0;
        }
        // Handle different dimension node types
        else if (dims->type == "ArraySize" || dims->type == "Number") {
            // Single dimension stored in value
            sym.size1 = stoi(dims->value);
            sym.size2 = 0;
        } else {
            // Multiple dimensions stored in children (Dimensions node)
            sym.size1   = (dims->children.size() >= 1) ? stoi(dims->children[0]->value) : 0;
            sym.size2   = (dims->children.size() >= 2) ? stoi(dims->children[1]->value) : 0;
        }
        
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
    if (node->children.size() < 3) {
        return;
    }

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

    // Check if this is an array declaration with initialization
    // Array: Type ID ArrayDims = ArrayInit  (4 children)
    // Scalar: Type ID = Expr                (3 children)
    bool hasArrayDims = (node->children.size() >= 4 &&
                         (node->children[2]->type == "Dimensions" || 
                          node->children[2]->type == "ArraySize" ||
                          node->children[2]->type == "InferredSize" ||
                          (node->children[2]->type == "Number" && node->children.size() == 4)));  // Number only if 4 children

    if (hasArrayDims) {
        auto& dims = node->children[2];
        sym.isArray = true;
        
        // Handle different dimension node types
        if (dims->type == "InferredSize") {
            // Empty brackets [] - infer size from initializer
            // We'll count the initializer elements and set the size
            if (node->children.size() >= 4) {
                auto& arrNode = node->children[3];
                int initializerCount = countArrayElements(arrNode);
                sym.size1 = initializerCount;
                sym.size2 = 0;
                
                // Update the dims node with the inferred size for documentation
                dims->value = to_string(initializerCount);
            } else {
                addError("Array with inferred size must have an initializer", *node->children[1]);
                sym.size1 = 0;
                sym.size2 = 0;
            }
        } else if (dims->type == "ArraySize") {
            // Single dimension stored in value
            try {
                sym.size1 = stoi(dims->value);
            } catch (...) {
                sym.size1 = 0;
            }
            sym.size2 = 0;
        } else if (dims->type == "Number") {
            // Handle parser issue where Number appears instead of ArraySize for 1D arrays
            try {
                sym.size1 = stoi(dims->value);
            } catch (...) {
                sym.size1 = 0;
            }
            sym.size2 = 0;
        } else {
            // Multiple dimensions stored in children (Dimensions node)
            sym.size1   = (dims->children.size() >= 1) ? stoi(dims->children[0]->value) : 0;
            sym.size2   = (dims->children.size() >= 2) ? stoi(dims->children[1]->value) : 0;
        }
        
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
        // children[3] is the Array literal (ArrayInit node)
        if (node->children.size() >= 4) {
            auto& arrNode = node->children[3];
            arrNode->dataType = typeName;
            
            // Check for self-initialization in array initializer
            if (usesVariable(arrNode, varName)) {
                addError("Variable '" + varName + "' is used in its own initializer", *arrNode);
            }
            
            // Count the number of initializer elements
            int initializerCount = countArrayElements(arrNode);
            
            // Only validate size if it was explicitly declared (not inferred)
            if (node->children[2]->type != "InferredSize") {
                int expectedSize = sym.size1; // For 1D arrays, use size1
                
                // For 2D arrays, we expect size1 * size2 elements in flattened form
                if (sym.size2 > 0) {
                    expectedSize = sym.size1 * sym.size2;
                }
                
                // Validate array size matches initializer count
                if (initializerCount != expectedSize) {
                    addError("Array size mismatch: declared size " + to_string(expectedSize) +
                             " but provided " + to_string(initializerCount) + " initializers", *arrNode);
                }
            }
            
            // Validate each element in the ArrayInit
            for (auto& elem : arrNode->children) {
                string elemType = visitExpr(elem);
                string resolved = resolveType(typeName, elemType);
                if (resolved == "mismatch") {
                    addError("Array element type mismatch: expected '" + typeName +
                             "', got '" + elemType + "'", *elem);
                }
            }
        }
    } else {
        // Scalar: validate RHS expression with strict type checking
        // Check for self-initialization (e.g., int x = x;)
        if (usesVariable(node->children[2], varName)) {
            addError("Variable '" + varName + "' is used in its own initializer", *node->children[2]);
        }
        
        string rhsType = visitExpr(node->children[2]);
        if (!isAssignmentCompatible(typeName, rhsType)) {
            addError("Type mismatch in assignment: cannot assign '" + rhsType +
                     "' to '" + typeName + "'", *node->children[2]);
        }
    }

    node->dataType = typeName;
}

// ---------------------------------------------------------------------------
// Assignment:  LHS  =  Expr  ;
//   children[0] = ID or ArrayAccess
//   children[1] = Expr
// ---------------------------------------------------------------------------

void SemanticAnalyzer::visitAssign(shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;

    auto& lhsNode = node->children[0];
    auto& rhsNode = node->children[1];

    string lhsType = "unknown";

    if (lhsNode->type == "ID") {
        string varName = lhsNode->value;
        const SemanticSymbol* sym = symTable_.lookup(varName);
        if (!sym) {
            addError("Undeclared variable '" + varName + "'", *lhsNode);
            lhsType = "unknown";
        } else if (sym->isArray) {
            addError("Array '" + varName + "' used without index", *lhsNode);
            lhsType = "unknown";
        } else {
            lhsType = sym->type;
        }
        lhsNode->dataType = lhsType;
    } else if (lhsNode->type == "ArrayAccess") {
        lhsType = visitArrayAccess(lhsNode);
    } else {
        lhsType = visitExpr(lhsNode);
    }

    string rhsType = visitExpr(rhsNode);

    // Special case: allow editing string via indexing.
    // Examples:
    //   string name = "karim";
    //   name[1] = "are";
    auto isStringScalarIndexAccess = [&](shared_ptr<ASTNode> accessNode) -> bool {
        if (!accessNode || accessNode->type != "ArrayAccess") return false;

        auto current = accessNode;
        while (current && current->type == "ArrayAccess" && !current->children.empty()) {
            if (current->children[0]->type == "ArrayAccess") {
                current = current->children[0];
            } else {
                break;
            }
        }

        if (!current || current->children.empty()) return false;
        auto base = current->children[0];
        if (!base || base->type != "ID") return false;

        const SemanticSymbol* sym = symTable_.lookup(base->value);
        return sym && sym->type == "string" && !sym->isArray;
    };

    if (lhsNode->type == "ArrayAccess" && isStringScalarIndexAccess(lhsNode)) {
        if (rhsType == "string" || rhsType == "char") {
            node->dataType = "string";
            return;
        }
    }

    // Convenience: allow single-character string literal for char assignment.
    if (lhsType == "char" && rhsNode->type == "STRING" && rhsNode->value.size() == 1) {
        rhsType = "char";
    }
    
    if (!isAssignmentCompatible(lhsType, rhsType)) {
        addError("Type mismatch in assignment: cannot assign '" + rhsType +
                 "' to '" + lhsType + "'", *rhsNode);
    }

    node->dataType = lhsType;
}

// ---------------------------------------------------------------------------
// Return statement:  return  [Expr]  ;
//   children[0] = Expr (optional)
// ---------------------------------------------------------------------------

void SemanticAnalyzer::visitReturn(shared_ptr<ASTNode> node) {
    if (node->children.empty()) {
        // return; (no expression)
        node->dataType = "void";
    } else {
        // return Expr;
        string exprType = visitExpr(node->children[0]);
        node->dataType = exprType;
    }
}

// ---------------------------------------------------------------------------
// Expression visitor — returns the resolved type
// ---------------------------------------------------------------------------

string SemanticAnalyzer::visitExpr(shared_ptr<ASTNode> node) {
    if (!node) return "unknown";

    const string& t = node->type;

    // Leaf: numeric literal
    if (t == "NUM" || t == "Number") {
        // Determine type based on the value
        string value = node->value;
        if (value.find('.') != string::npos) {
            // Contains decimal point - it's a float
            node->dataType = "float";
            return "float";
        } else {
            // No decimal point - it's an integer
            node->dataType = "int";
            return "int";
        }
    }

    // Leaf: string literal
    if (t == "STRING") {
        node->dataType = "string";
        return "string";
    }

    // Leaf: char literal
    if (t == "CHAR") {
        node->dataType = "char";
        return "char";
    }

    if (t == "EndLine") {
        node->dataType = "void";
        return "void";
    }

    // Leaf: identifier
    if (t == "ID") {
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
    if (t == "ArrayAccess") {
        string type = visitArrayAccess(node);
        return type;
    }

    // BinaryOp node (from parser JSON with operator field)
    if (t == "BinaryOp") {
        if (node->children.size() < 3) {
            node->dataType = "unknown";
            return "unknown";
        }
        // children[0] = left, children[1] = operator, children[2] = right
        string t1 = visitExpr(node->children[0]);
        string t2 = visitExpr(node->children[2]);
        string resolved = resolveType(t1, t2);

        if (resolved == "mismatch") {
            string op = node->children[1]->value;
            addError("Type mismatch in expression: '" + t1 +
                     "' " + op + " '" + t2 + "'", *node);
            resolved = "unknown";
        }
        node->dataType = resolved;
        return resolved;
    }

    // Binary expression node produced by the parser:
    // The parser collapses Expr/Term/Factor chains, so a binary op node
    // has type "+" / "-" / "*" / "/" with two children.
    if (t == "+" || t == "-" ||
        t == "*" || t == "/") {

        if (node->children.size() < 2) {
            node->dataType = "unknown";
            return "unknown";
        }
        string t1 = visitExpr(node->children[0]);
        string t2 = visitExpr(node->children[1]);
        
        // Special case: string concatenation with +
        if (t == "+" && (t1 == "string" || t2 == "string")) {
            // Allow string + string, string + char, char + string
            if ((t1 == "string" || t1 == "char") && (t2 == "string" || t2 == "char")) {
                node->dataType = "string";
                return "string";
            } else {
                addError("Cannot concatenate string with '" + 
                         (t1 == "string" ? t2 : t1) + "' type", *node);
                node->dataType = "unknown";
                return "unknown";
            }
        }
        
        // Arithmetic operations: only allowed on numeric types
        if (t1 == "string" || t1 == "char" || t2 == "string" || t2 == "char") {
            addError("Arithmetic operation '" + t + "' not allowed on non-numeric types: '" + 
                     t1 + "' " + t + " '" + t2 + "'", *node);
            node->dataType = "unknown";
            return "unknown";
        }
        
        string resolved = resolveType(t1, t2);

        if (resolved == "mismatch") {
            addError("Type mismatch in expression: '" + t1 +
                     "' " + t + " '" + t2 + "'", *node);
            resolved = "unknown";
        }
        node->dataType = resolved;
        return resolved;
    }

    // Parenthesised expression — single child
    if (t == "Expr" || t == "Term" || t == "Factor") {
        if (!node->children.empty()) {
            string type = visitExpr(node->children[0]);
            node->dataType = type;
            return type;
        }
    }

    // Fallback: recurse and return first resolved child type
    string result = "unknown";
    for (auto& child : node->children) {
        string type = visitExpr(child);
        if (type != "unknown") result = type;
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

    // ArrayAccess nodes should have children[0] = ID or nested ArrayAccess, children[1] = index
    if (node->children.size() < 2) {
        node->dataType = "unknown";
        return "unknown";
    }

    auto& baseNode = node->children[0];
    auto& idxNode = node->children[1];

    string varName;
    const SemanticSymbol* sym = nullptr;
    int accessDepth = 0;

    // Count the depth of array access and find the base variable
    auto current = node;
    while (current && current->type == "ArrayAccess") {
        accessDepth++;
        if (current->children.size() >= 1) {
            current = current->children[0];
        } else {
            break;
        }
    }
    
    if (current && current->type == "ID") {
        varName = current->value;
        sym = symTable_.lookup(varName);
    } else {
        addError("Invalid array access structure", *node);
        node->dataType = "unknown";
        return "unknown";
    }

    if (!sym) {
        addError("Undeclared variable '" + varName + "'", *baseNode);
        node->dataType = "unknown";
        return "unknown";
    }

    // Special case: string indexing (strings are arrays of characters)
    if (sym->type == "string" && !sym->isArray) {
        // Validate index type — must be integer
        string idxType = visitExpr(idxNode);
        if (!isIntegerType(idxType) && idxType != "unknown") {
            addError("String index for '" + varName +
                     "' must be an integer, got '" + idxType + "'", *idxNode);
        }
        
        // String indexing returns a char
        node->dataType = "char";
        return "char";
    }

    if (!sym->isArray) {
        addError("Variable '" + varName + "' is not an array", *baseNode);
        node->dataType = "unknown";
        return "unknown";
    }

    // Determine array dimensionality
    int arrayDimensions = (sym->size2 > 0) ? 2 : 1;
    
    // Validate index type — must be integer
    string idxType = visitExpr(idxNode);
    if (!isIntegerType(idxType) && idxType != "unknown") {
        addError("Array index for '" + varName +
                 "' must be an integer, got '" + idxType + "'", *idxNode);
    }
    
    // Check bounds for constant indices
    if (accessDepth == 1 && arrayDimensions == 1) {
        // 1D array access - check if index is constant and within bounds
        if (idxNode->type == "NUM" || idxNode->type == "Number") {
            try {
                int index = stoi(idxNode->value);
                if (index < 0) {
                    addError("Array index cannot be negative: " + varName + "[" + to_string(index) + "]", *idxNode);
                } else if (index >= sym->size1) {
                    addError("Array index out of bounds: " + varName + "[" + to_string(index) + 
                             "] (array size is " + to_string(sym->size1) + ")", *idxNode);
                }
            } catch (...) {
                // Not a valid integer, skip bounds check
            }
        }
    } else if (accessDepth == 2 && arrayDimensions == 2) {
        // 2D array access - need to check both indices
        // Collect both index nodes
        vector<shared_ptr<ASTNode>> indexNodes;
        auto curr = node;
        while (curr && curr->type == "ArrayAccess" && curr->children.size() >= 2) {
            indexNodes.push_back(curr->children[1]);
            if (curr->children[0]->type == "ArrayAccess") {
                curr = curr->children[0];
            } else {
                break;
            }
        }
        
        // Reverse to get indices in correct order (outermost first)
        reverse(indexNodes.begin(), indexNodes.end());
        
        // Check first index (row)
        if (indexNodes.size() >= 1) {
            auto& idx0 = indexNodes[0];
            if (idx0->type == "NUM" || idx0->type == "Number") {
                try {
                    int index = stoi(idx0->value);
                    if (index < 0) {
                        addError("Array index cannot be negative: " + varName + "[" + to_string(index) + "][...]", *idx0);
                    } else if (index >= sym->size1) {
                        addError("Array index out of bounds: " + varName + "[" + to_string(index) + 
                                 "][...] (first dimension size is " + to_string(sym->size1) + ")", *idx0);
                    }
                } catch (...) {}
            }
        }
        
        // Check second index (column)
        if (indexNodes.size() >= 2) {
            auto& idx1 = indexNodes[1];
            if (idx1->type == "NUM" || idx1->type == "Number") {
                try {
                    int index = stoi(idx1->value);
                    if (index < 0) {
                        addError("Array index cannot be negative: " + varName + "[...][" + to_string(index) + "]", *idx1);
                    } else if (index >= sym->size2) {
                        addError("Array index out of bounds: " + varName + "[...][" + to_string(index) + 
                                 "] (second dimension size is " + to_string(sym->size2) + ")", *idx1);
                    }
                } catch (...) {}
            }
        }
    }

    // Determine the result type based on access depth vs array dimensions
    if (accessDepth > arrayDimensions) {
        addError("Too many indices for array '" + varName + "': expected " + 
                 to_string(arrayDimensions) + " but got " + to_string(accessDepth), *node);
        node->dataType = "unknown";
        return "unknown";
    } else if (accessDepth == arrayDimensions) {
        // Full indexing - returns scalar element
        node->dataType = sym->type;
        return sym->type;
    } else {
        // Partial indexing - returns array of reduced dimensionality
        // This should not be assignable to scalar variables
        node->dataType = "array";
        return "array";
    }
}

// ---------------------------------------------------------------------------
// Helper function to count array elements in initialization
// ---------------------------------------------------------------------------

int SemanticAnalyzer::countArrayElements(shared_ptr<ASTNode> arrayNode) {
    if (!arrayNode) return 0;
    
    // Handle different array initialization structures
    if (arrayNode->type == "ArrayInit") {
        int count = 0;
        for (auto& child : arrayNode->children) {
            if (child->type == "ArrayElements") {
                count += countArrayElements(child);
            } else if (child->type == "ArrayElement") {
                count += countArrayElements(child);
            } else if (child->type == "ArrayInit") {
                // Nested array initialization (for 2D arrays)
                count += countArrayElements(child);
            } else if (child->type != "," && child->type != "{" && child->type != "}") {
                // Skip comma and brace tokens, count actual elements
                count++;
            }
        }
        return count;
    } else if (arrayNode->type == "ArrayElements") {
        int count = 0;
        for (auto& child : arrayNode->children) {
            if (child->type == "ArrayElement") {
                count += countArrayElements(child);
            } else if (child->type == "ArrayElements") {
                count += countArrayElements(child);
            } else if (child->type != "," && child->type != "{" && child->type != "}") {
                // Skip comma and brace tokens, count actual elements
                count++;
            }
        }
        return count;
    } else if (arrayNode->type == "ArrayElement") {
        // ArrayElement contains one actual value
        return 1;
    } else {
        // Direct value nodes (Number, ID, expressions, etc.)
        return 1;
    }
}

// ---------------------------------------------------------------------------
// Helper function to check if an expression uses a specific variable
// ---------------------------------------------------------------------------

bool SemanticAnalyzer::usesVariable(shared_ptr<ASTNode> node, const string& varName) {
    if (!node) return false;
    
    // Check if this node is an ID with the variable name
    if (node->type == "ID" && node->value == varName) {
        return true;
    }
    
    // Recursively check all children
    for (auto& child : node->children) {
        if (usesVariable(child, varName)) {
            return true;
        }
    }
    
    return false;
}
