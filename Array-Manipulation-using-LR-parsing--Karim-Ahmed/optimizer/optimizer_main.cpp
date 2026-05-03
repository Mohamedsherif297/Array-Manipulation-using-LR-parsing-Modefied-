// optimizer_main.cpp
// Entry point for the Code Optimization phase.
//
// Usage:
//   ./optimizer [input_ir] [output_ir]
//
//   input_ir  : path to IR file from codegen (default: ../codegen/ir.txt)
//   output_ir : path for optimized IR output (default: optimized_ir.txt)
//
// Implements 5 optimization techniques:
//   1. Constant Folding
//   2. Constant Propagation
//   3. Common Subexpression Elimination
//   4. Code Movement (Loop-Invariant Code Motion)
//   5. Dead Code Elimination

#include <iostream>
#include <string>
#include "ir_loader.h"
#include "optimizer.h"

using namespace std;

void printIR(const OptIR& ir, const string& title) {
    cout << "\n" << title << "\n";
    cout << string(title.length(), '-') << "\n";
    
    for (const auto& quad : ir) {
        if (!quad.isAlive) continue;
        
        if (quad.op == "DECL") {
            cout << "// DECL " << quad.result;
            if (!quad.arg1.empty()) cout << "  type=" << quad.arg1;
            cout << "\n";
        } else if (quad.op == "ASSIGN") {
            cout << quad.result << " = " << quad.arg1 << "\n";
        } else if (quad.op == "LOAD") {
            cout << quad.result << " = " << quad.arg1 << "[" << quad.arg2 << "]\n";
        } else if (quad.op == "STORE") {
            cout << quad.arg1 << "[" << quad.arg2 << "] = " << quad.result << "\n";
        } else if (quad.op == "LABEL") {
            cout << quad.result << ":\n";
        } else if (quad.op == "GOTO") {
            cout << "GOTO " << quad.result << "\n";
        } else if (quad.op == "IF_FALSE") {
            cout << "IF_FALSE " << quad.arg1 << " GOTO " << quad.result << "\n";
        } else if (quad.op == "PRINT") {
            cout << "PRINT " << quad.arg1 << "\n";
        } else if (quad.op == "PRINTLN") {
            cout << "PRINTLN\n";
        } else if (quad.op == "READ") {
            cout << "READ " << quad.result << "\n";
        } else if (quad.arg2.empty()) {
            cout << quad.result << " = " << quad.op << " " << quad.arg1 << "\n";
        } else {
            cout << quad.result << " = " << quad.arg1 << " " << quad.op << " " << quad.arg2 << "\n";
        }
    }
    cout << string(title.length(), '-') << "\n";
}

int countInstructions(const OptIR& ir) {
    int count = 0;
    for (const auto& quad : ir) {
        if (quad.isAlive && quad.op != "DECL") count++;
    }
    return count;
}

int main(int argc, char* argv[]) {
    
    // -----------------------------------------------------------------------
    // 1. Resolve file paths from arguments
    // -----------------------------------------------------------------------
    string inputIR  = "../codegen/ir.txt";
    string outputIR = "optimized_ir.txt";
    
    if (argc >= 2) inputIR  = argv[1];
    if (argc >= 3) outputIR = argv[2];
    
    cout << "========================================\n";
    cout << "  CODE OPTIMIZATION PHASE\n";
    cout << "========================================\n";
    cout << "Input IR  : " << inputIR << "\n";
    cout << "Output IR : " << outputIR << "\n";
    
    // -----------------------------------------------------------------------
    // 2. Load IR from file
    // -----------------------------------------------------------------------
    OptIR ir = loadIR(inputIR);
    
    if (ir.empty()) {
        cerr << "[Fatal] Failed to load IR or IR is empty.\n";
        return 1;
    }
    
    cout << "[Loader] IR loaded successfully (" << ir.size() << " instruction(s))\n";
    
    int originalInstructions = countInstructions(ir);
    
    // Display original IR
    printIR(ir, "=== Original IR ===");
    
    // -----------------------------------------------------------------------
    // 3. Run optimization passes
    // -----------------------------------------------------------------------
    Optimizer optimizer(ir);
    optimizer.optimize();
    
    // -----------------------------------------------------------------------
    // 4. Display optimized IR
    // -----------------------------------------------------------------------
    const OptIR& optimizedIR = optimizer.getIR();
    printIR(optimizedIR, "=== Optimized IR ===");
    
    // -----------------------------------------------------------------------
    // 5. Display statistics
    // -----------------------------------------------------------------------
    optimizer.getStats().display();
    
    int optimizedInstructions = countInstructions(optimizedIR);
    int reduction = originalInstructions - optimizedInstructions;
    double percentage = originalInstructions > 0 
        ? (reduction * 100.0 / originalInstructions) 
        : 0.0;
    
    cout << "\n=== Code Size Reduction ===\n";
    cout << "Original instructions:  " << originalInstructions << "\n";
    cout << "Optimized instructions: " << optimizedInstructions << "\n";
    cout << "Reduction:              " << reduction << " instruction(s) (" 
         << percentage << "%)\n";
    cout << "===========================\n";
    
    // -----------------------------------------------------------------------
    // 6. Write optimized IR to file
    // -----------------------------------------------------------------------
    if (!writeOptimizedIR(optimizedIR, outputIR)) {
        cerr << "[Fatal] Could not write optimized IR file.\n";
        return 1;
    }
    
    cout << "\n[Output] Optimized IR written to: " << outputIR << "\n";
    
    cout << "\n========================================\n";
    cout << "  OPTIMIZATION COMPLETE\n";
    cout << "========================================\n";
    
    return 0;
}
