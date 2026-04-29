// codegen_main.cpp
// Entry point for the Intermediate Code Generation phase.
//
// Usage:
//   ./codegen [annotated_ast] [symbol_table] [output_ir]
//
//   annotated_ast : path to annotated_ast.json  (default: ../semantic/annotated_ast.json)
//   symbol_table  : path to symbol_table.json   (default: ../semantic/symbol_table.json)
//   output_ir     : path for the output IR file (default: ir.txt)
//
// Outputs:
//   <output_ir>   — Three-Address Code in human-readable TAC format

#include <iostream>
#include <string>
#include <memory>

#include "ast_loader.h"
#include "symbol_table_loader.h"
#include "code_generator.h"

using namespace std;

int main(int argc, char* argv[]) {

    // -----------------------------------------------------------------------
    // 1. Resolve file paths from arguments
    // -----------------------------------------------------------------------
    string astFile    = "../semantic/annotated_ast.json";
    string symFile    = "../semantic/symbol_table.json";
    string irFile     = "ir.txt";

    if (argc >= 2) astFile = argv[1];
    if (argc >= 3) symFile = argv[2];
    if (argc >= 4) irFile  = argv[3];

    cout << "=== Intermediate Code Generation ===\n";
    cout << "Annotated AST : " << astFile << "\n";
    cout << "Symbol Table  : " << symFile << "\n";
    cout << "Output IR     : " << irFile  << "\n\n";

    // -----------------------------------------------------------------------
    // 2. Load the annotated AST
    // -----------------------------------------------------------------------
    shared_ptr<ASTNode> root = loadAnnotatedAST(astFile);
    if (!root) {
        cerr << "[Fatal] Failed to load annotated AST. Aborting.\n";
        return 1;
    }
    cout << "[Loader] Annotated AST loaded successfully.\n";

    // -----------------------------------------------------------------------
    // 3. Load the symbol table
    // -----------------------------------------------------------------------
    CGSymbolTable symTable = loadSymbolTable(symFile);
    if (symTable.empty()) {
        // An empty table is not necessarily fatal (e.g. a program with no
        // variables), but warn the user.
        cerr << "[Warning] Symbol table is empty or could not be loaded.\n";
    } else {
        cout << "[Loader] Symbol table loaded ("
             << symTable.size() << " symbol(s)).\n";
    }

    // -----------------------------------------------------------------------
    // 4. Run code generation
    // -----------------------------------------------------------------------
    CodeGenerator cg(symTable);
    bool ok = cg.generate(root);

    if (!ok) {
        cerr << "[CodeGen] Code generation completed with errors.\n";
        cerr << "[CodeGen] Check error messages above for details.\n";
    } else {
        cout << "[CodeGen] Code generation successful. "
             << cg.getIR().size() << " quad(s) generated.\n";
    }

    // -----------------------------------------------------------------------
    // 5. Print IR to stdout
    // -----------------------------------------------------------------------
    cout << "\n--- Generated IR (TAC) ---\n";
    cg.printIR(cout);
    cout << "--------------------------\n\n";

    // -----------------------------------------------------------------------
    // 6. Write IR to file
    // -----------------------------------------------------------------------
    if (!cg.writeIR(irFile)) {
        cerr << "[Fatal] Could not write IR file.\n";
        return 1;
    }

    return ok ? 0 : 1;
}
