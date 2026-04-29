#pragma once
// optimizer_types.h
// Data structures for the optimization phase

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>

using namespace std;

// Reuse the Quad structure from codegen
struct OptQuad {
    string op;      // operator / instruction mnemonic
    string arg1;    // first operand
    string arg2;    // second operand
    string result;  // destination
    bool isAlive;   // for dead code elimination
    int lineNum;    // original line number for tracking

    OptQuad(string op, string arg1, string arg2, string result, int line = 0)
        : op(op), arg1(arg1), arg2(arg2), result(result), isAlive(true), lineNum(line) {}
};

using OptIR = vector<OptQuad>;

// Optimization statistics
struct OptimizationStats {
    int constantFolding = 0;
    int constantPropagation = 0;
    int commonSubexprElimination = 0;
    int codeMovement = 0;
    int deadCodeElimination = 0;
    
    int totalOptimizations() const {
        return constantFolding + constantPropagation + 
               commonSubexprElimination + codeMovement + deadCodeElimination;
    }
    
    void display() const {
        cout << "\n=== Optimization Statistics ===\n";
        cout << "Constant Folding:              " << constantFolding << "\n";
        cout << "Constant Propagation:          " << constantPropagation << "\n";
        cout << "Common Subexpression Elim.:    " << commonSubexprElimination << "\n";
        cout << "Code Movement:                 " << codeMovement << "\n";
        cout << "Dead Code Elimination:         " << deadCodeElimination << "\n";
        cout << "-------------------------------\n";
        cout << "Total Optimizations:           " << totalOptimizations() << "\n";
        cout << "===============================\n";
    }
};
