// semantic_main.cpp
// Entry point for the Semantic Analysis phase.
//
// Usage:
//   ./semantic_main [ast_file] [output_dir]
//
//   ast_file   : path to the JSON AST produced by the parser (default: ast.json)
//   output_dir : directory for output files              (default: current dir)
//
// Outputs:
//   <output_dir>/annotated_ast.json
//   <output_dir>/symbol_table.json

#include <iostream>
#include <string>
#include "ast_builder.h"
#include "symbol_table.h"
#include "semantic_analyzer.h"
#include "semantic_output.h"

using namespace std;

int main(int argc, char* argv[]) {

    // -----------------------------------------------------------------------
    // 1. Resolve file paths from arguments
    // -----------------------------------------------------------------------
    string astFile   = "ast.json";   // default
    string outputDir = ".";          // default: current directory

    if (argc >= 2) astFile   = argv[1];
    if (argc >= 3) outputDir = argv[2];

    // Ensure output dir ends without trailing slash issues
    if (!outputDir.empty() && outputDir.back() == '/') outputDir.pop_back();
    if (!outputDir.empty() && outputDir.back() == '\\') outputDir.pop_back();

    string annotatedASTFile  = outputDir + "/annotated_ast.json";
    string symbolTableFile   = outputDir + "/symbol_table.json";

    cout << "=== Semantic Analysis ===\n";
    cout << "Input AST   : " << astFile          << "\n";
    cout << "Output AST  : " << annotatedASTFile  << "\n";
    cout << "Symbol Table: " << symbolTableFile   << "\n\n";

    // -----------------------------------------------------------------------
    // 2. Load AST from JSON
    // -----------------------------------------------------------------------
    shared_ptr<ASTNode> root = loadAST(astFile);
    if (!root) {
        cerr << "[Fatal] Failed to load AST. Aborting.\n";
        return 1;
    }
    cout << "[AST Loader] AST loaded successfully.\n";

    // -----------------------------------------------------------------------
    // 3. Run semantic analysis
    // -----------------------------------------------------------------------
    SemanticSymbolTable symTable;
    SemanticAnalyzer    analyzer(symTable);

    bool ok = analyzer.analyze(root);

    // -----------------------------------------------------------------------
    // 4. Report errors / success
    // -----------------------------------------------------------------------
    if (!ok) {
        cout << "\n[Semantic Analysis] Completed with "
             << analyzer.errors().size() << " error(s):\n";
        for (auto& err : analyzer.errors()) {
            cout << "  - " << err.message << "\n";
        }
    } else {
        cout << "[Semantic Analysis] No errors found.\n";
    }

    symTable.display();

    // -----------------------------------------------------------------------
    // 5. Write outputs
    // -----------------------------------------------------------------------
    writeAnnotatedAST(root,    annotatedASTFile);
    writeSymbolTable (symTable, symbolTableFile);

    return ok ? 0 : 1;
}
