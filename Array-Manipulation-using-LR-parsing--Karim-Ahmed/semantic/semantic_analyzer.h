#pragma once

#include <memory>
#include <vector>
#include <string>
#include "ASTNode.h"
#include "symbol_table.h"

using namespace std;

struct SemanticError {
    string message;
    string nodeType;   // which AST node triggered it
    string nodeValue;  // value of that node (if any)
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

    void addError(const string& msg, const ASTNode& node);

    // Node visitors
    void visitProgram(shared_ptr<ASTNode> node);
    void visitStatement(shared_ptr<ASTNode> node);
    void visitDecl(shared_ptr<ASTNode> node);
    void visitDeclAssign(shared_ptr<ASTNode> node);
    void visitAssign(shared_ptr<ASTNode> node);
    string visitExpr(shared_ptr<ASTNode> node);   // returns resolved type
    string visitArrayAccess(shared_ptr<ASTNode> node);

    // Helpers
    string resolveType(const string& t1, const string& t2);
    bool   isNumericType(const string& t);
    bool   isIntegerType(const string& t);
};
