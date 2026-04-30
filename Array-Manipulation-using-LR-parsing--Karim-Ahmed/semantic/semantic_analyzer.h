#pragma once

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include "ASTNode.h"
#include "symbol_table.h"

using namespace std;

struct SemanticError {
    string message;
    string nodeType;   // which AST node triggered it
    string nodeValue;  // value of that node (if any)
    int line;          // line number where error occurred
};

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(SemanticSymbolTable& symTable);

    // Entry point: walk the whole tree, annotate nodes, collect errors.
    // Returns true if no errors were found.
    bool analyze(shared_ptr<ASTNode> root);

    const vector<SemanticError>& errors() const { return errors_; }
    bool hasErrors() const { return !errors_.empty(); }

private:
    SemanticSymbolTable& symTable_;
    vector<SemanticError> errors_;
    string currentScope_ = "global";   // tracks current scope during traversal

    void addError(const string& msg, const ASTNode& node);

    // Node visitors
    void visitFunctionDef(shared_ptr<ASTNode> node);
    void visitProgram(shared_ptr<ASTNode> node);
    void visitStatement(shared_ptr<ASTNode> node);
    void visitDeclStmt(shared_ptr<ASTNode> node);
    void visitDecl(shared_ptr<ASTNode> node);
    void visitDeclAssign(shared_ptr<ASTNode> node);
    void visitAssign(shared_ptr<ASTNode> node);
    void visitOutput(shared_ptr<ASTNode> node);
    void visitInput(shared_ptr<ASTNode> node);
    void visitReturn(shared_ptr<ASTNode> node);
    void visitForStmt(shared_ptr<ASTNode> node);  // for loop — full semantic validation
    void visitIncrement(shared_ptr<ASTNode> node); // ++i / i++
    string visitExpr(shared_ptr<ASTNode> node);   // returns resolved type
    string visitArrayAccess(shared_ptr<ASTNode> node);

    // Helpers
    string resolveType(const string& t1, const string& t2);
    bool   isAssignmentCompatible(const string& lhsType, const string& rhsType);
    bool   isNumericType(const string& t);
    bool   isIntegerType(const string& t);
    bool   isStringType(const string& t);
    bool   isCharType(const string& t);
    int    countArrayElements(shared_ptr<ASTNode> arrayNode);
    bool   usesVariable(shared_ptr<ASTNode> node, const string& varName);
};
