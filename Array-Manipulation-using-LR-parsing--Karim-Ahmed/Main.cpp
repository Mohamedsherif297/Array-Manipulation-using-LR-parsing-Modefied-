#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"

using namespace std;

void printSeparator(const string& title) {
    cout << "\n";
    cout << "========================================\n";
    cout << "  " << title << "\n";
    cout << "========================================\n";
}

void printTestCase(const string& code) {
    cout << "\n📝 Testing Code:\n";
    cout << "   \"" << code << "\"\n";
}

int main(int argc, char* argv[]) {
    
    printSeparator("COMPILER PIPELINE TEST");
    
    // Read source code from file argument, stdin, or use default
    string src;
    
    if (argc > 1) {
        // Read from file specified as argument
        ifstream inputFile(argv[1]);
        if (inputFile) {
            string line;
            while (getline(inputFile, line)) {
                src += line + "\n";
            }
            inputFile.close();
            // Remove trailing newline
            if (!src.empty() && src.back() == '\n') {
                src.pop_back();
            }
        } else {
            cerr << "Error: Cannot open input file: " << argv[1] << "\n";
            return 1;
        }
    } else if (!isatty(fileno(stdin))) {
        // Input is piped, read all lines from stdin
        string line;
        while (getline(cin, line)) {
            src += line + "\n";
        }
        // Remove trailing newline
        if (!src.empty() && src.back() == '\n') {
            src.pop_back();
        }
        if (src.empty()) {
            src = "int x[2][2] = {{1,2},{3,4}};";
        }
    } else {
        // No piped input, use default test case
        src = "int x[2][2] = {{1,2},{3,4}};";
    }
    
    printTestCase(src);

    // ========================================
    // PHASE 1: LEXICAL ANALYSIS
    // ========================================
    printSeparator("PHASE 1: LEXICAL ANALYSIS");
    
    SymbolTable st;
    Lexer lexer(src, st);
    vector<Token> tokens = lexer.tokenize();
    
    cout << "✅ Tokenization complete!\n";
    cout << "   Tokens generated: " << tokens.size() << "\n";
    
    cout << "\n📋 Token Stream:\n";
    for (const auto &t : tokens) {
        cout << "   [" << t.getTypeAsString() << "] " << t.getLexeme() << "\n";
    }
    
    cout << "\n📊 Symbol Table:\n";
    st.display();

    // ========================================
    // PHASE 2: SYNTAX ANALYSIS (PARSING)
    // ========================================
    printSeparator("PHASE 2: SYNTAX ANALYSIS");
    
    cout << "Building LR(1) parser...\n";
    computeFIRST();
    cout << "   ✓ FIRST sets computed\n";
    
    buildStates();
    cout << "   ✓ LR(1) states built\n";
    
    buildParsingTable();
    cout << "   ✓ Parsing tables generated\n";

    vector<pair<string,string>> input;

    for (auto &t : tokens) {
        switch (t.getType()) {
            case TokenType::DATATYPE:
                input.push_back({t.getLexeme(), t.getLexeme()});
                break;
            case TokenType::IDENTIFIER:
                input.push_back({"ID", t.getLexeme()});
                break;
            case TokenType::CONSTANT:
                input.push_back({"NUM", t.getLexeme()});
                break;
            case TokenType::STRING:
                input.push_back({"STRING", t.getLexeme()});
                break;
            case TokenType::CHAR:
                input.push_back({"CHAR", t.getLexeme()});
                break;
            case TokenType::END_OF_FILE:
                input.push_back({"$", "$"});
                break;
            default:
                input.push_back({t.getLexeme(), t.getLexeme()});
                break;
        }
    }

    cout << "\nParsing input...\n";
    Node* root = parse(input);

    if (!root) {
        cout << "❌ Parsing failed!\n";
        return 1;
    }

    ofstream astFile("ast.json");
    if (!astFile) {
        cout << "❌ Error creating ast.json\n";
        return 1;
    }

    printJSON(root, 0, &astFile);
    astFile.close();

    cout << "✅ Parsing successful!\n";
    cout << "   AST saved to: ast.json\n";

    // ========================================
    // PHASE 3: SEMANTIC ANALYSIS
    // ========================================
    printSeparator("PHASE 3: SEMANTIC ANALYSIS");
    
    cout << "Running semantic analyzer...\n";
    int semanticResult = system("cd semantic && semantic_main.exe ../ast.json . 2>&1");
    
    if (semanticResult != 0) {
        cout << "\n⚠️  Semantic analyzer not compiled or failed.\n";
        cout << "   To compile: cd semantic && g++ -std=c++17 semantic_main.cpp ast_builder.cpp semantic_analyzer.cpp semantic_output.cpp symbol_table.cpp -o semantic_main\n";
    } else {
        cout << "✅ Semantic analysis complete!\n";
        cout << "   Output: semantic/annotated_ast.json\n";
        cout << "   Output: semantic/symbol_table.json\n";
    }

    // ========================================
    // PHASE 4: CODE GENERATION
    // ========================================
    printSeparator("PHASE 4: CODE GENERATION");
    
    cout << "Running code generator...\n";
    int codegenResult = system("cd codegen && codegen.exe ../semantic/annotated_ast.json ../semantic/symbol_table.json ir.txt 2>&1");
    
    if (codegenResult != 0) {
        cout << "\n⚠️  Code generator not compiled or failed.\n";
        cout << "   To compile: cd codegen && make\n";
    } else {
        cout << "✅ Code generation complete!\n";
        cout << "   Output: codegen/ir.txt\n";
        
        // Display the generated IR
        cout << "\n📄 Generated Three-Address Code:\n";
        cout << "   --------------------------------\n";
        ifstream irFile("codegen/ir.txt");
        if (irFile) {
            string line;
            while (getline(irFile, line)) {
                cout << "   " << line << "\n";
            }
            irFile.close();
        }
        cout << "   --------------------------------\n";
    }

    // ========================================
    // PHASE 5: CODE OPTIMIZATION
    // ========================================
    printSeparator("PHASE 5: CODE OPTIMIZATION");
    
    cout << "Running optimizer...\n";
    int optimizerResult = system("cd optimizer && optimizer.exe ../codegen/ir.txt optimized_ir.txt 2>&1");
    
    if (optimizerResult != 0) {
        cout << "\n⚠️  Optimizer not compiled or failed.\n";
        cout << "   To compile: cd optimizer && make\n";
    } else {
        cout << "✅ Code optimization complete!\n";
        cout << "   Output: optimizer/optimized_ir.txt\n";
        
        // Display the optimized IR
        cout << "\n📄 Optimized Three-Address Code:\n";
        cout << "   --------------------------------\n";
        ifstream optFile("optimizer/optimized_ir.txt");
        if (optFile) {
            string line;
            while (getline(optFile, line)) {
                cout << "   " << line << "\n";
            }
            optFile.close();
        }
        cout << "   --------------------------------\n";
    }

    // ========================================
    // SUMMARY
    // ========================================
    printSeparator("COMPILATION SUMMARY");
    
    cout << "✅ Phase 1: Lexical Analysis    - COMPLETE\n";
    cout << "✅ Phase 2: Syntax Analysis     - COMPLETE\n";
    cout << (semanticResult == 0 ? "✅" : "⚠️ ") << " Phase 3: Semantic Analysis  - " 
         << (semanticResult == 0 ? "COMPLETE" : "SKIPPED (not compiled)") << "\n";
    cout << (codegenResult == 0 ? "✅" : "⚠️ ") << " Phase 4: Code Generation    - " 
         << (codegenResult == 0 ? "COMPLETE" : "SKIPPED (not compiled)") << "\n";
    cout << (optimizerResult == 0 ? "✅" : "⚠️ ") << " Phase 5: Code Optimization  - " 
         << (optimizerResult == 0 ? "COMPLETE" : "SKIPPED (not compiled)") << "\n";
    
    cout << "\n📁 Output Files:\n";
    cout << "   - ast.json (Phase 2)\n";
    if (semanticResult == 0) {
        cout << "   - semantic/annotated_ast.json (Phase 3)\n";
        cout << "   - semantic/symbol_table.json (Phase 3)\n";
    }
    if (codegenResult == 0) {
        cout << "   - codegen/ir.txt (Phase 4)\n";
    }
    if (optimizerResult == 0) {
        cout << "   - optimizer/optimized_ir.txt (Phase 5)\n";
    }
    
    printSeparator("TEST COMPLETE");
    
    return 0;
}