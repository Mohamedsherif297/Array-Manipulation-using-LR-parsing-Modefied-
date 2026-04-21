#pragma once
// code_generator.h
// Intermediate Code Generator — transforms an annotated AST into
// Three-Address Code (TAC) in Quadruple form.
//
// Usage:
//   CodeGenerator cg(symbolTable);
//   cg.generate(root);                // walk the AST
//   const IR& ir = cg.getIR();        // retrieve generated quads
//   cg.writeIR("ir.txt");             // write human-readable output

#include <string>
#include <memory>
#include <ostream>

#include "codegen_types.h"
#include "symbol_table_loader.h"
#include "../semantic/ASTNode.h"

using namespace std;

class CodeGenerator {
public:
    // Construct with a pre-loaded symbol table.
    explicit CodeGenerator(const CGSymbolTable& symTable);

    // Walk the annotated AST and populate the internal IR.
    // Returns true on success, false if unrecoverable errors occurred.
    bool generate(shared_ptr<ASTNode> root);

    // Access the generated IR.
    const IR& getIR() const { return ir_; }

    // Write the IR to a file in human-readable TAC format.
    // Returns true on success.
    bool writeIR(const string& filePath) const;

    // Write the IR to any output stream (useful for stdout / testing).
    void printIR(ostream& out) const;

private:
    // -----------------------------------------------------------------------
    // Statement visitors
    // -----------------------------------------------------------------------
    void genProgram    (shared_ptr<ASTNode> node);
    void genFunctionDef(shared_ptr<ASTNode> node);
    void genStatement  (shared_ptr<ASTNode> node);
    void genDecl       (shared_ptr<ASTNode> node);
    void genDeclAssign (shared_ptr<ASTNode> node);
    void genAssign     (shared_ptr<ASTNode> node);

    // -----------------------------------------------------------------------
    // Expression visitors — return the name of the temporary (or literal /
    // variable) that holds the expression's value.
    // -----------------------------------------------------------------------
    string genExpr        (shared_ptr<ASTNode> node);
    string genBinaryOp    (shared_ptr<ASTNode> node);
    string genArrayAccess (shared_ptr<ASTNode> node);

    // -----------------------------------------------------------------------
    // Array initialisation helpers
    // -----------------------------------------------------------------------
    void genArrayInit (shared_ptr<ASTNode> idNode,
                       shared_ptr<ASTNode> arrayNode,
                       const CGSymbol&     sym);

    // -----------------------------------------------------------------------
    // IR emission helpers
    // -----------------------------------------------------------------------
    void emit(const string& op,
              const string& arg1,
              const string& arg2,
              const string& result);

    // Generate a fresh temporary name: t1, t2, …
    string newTemp();

    // -----------------------------------------------------------------------
    // Data members
    // -----------------------------------------------------------------------
    const CGSymbolTable& symTable_;
    IR                   ir_;
    int                  tempCounter_ = 0;
    bool                 hasError_    = false;

    void error(const string& msg);
};
