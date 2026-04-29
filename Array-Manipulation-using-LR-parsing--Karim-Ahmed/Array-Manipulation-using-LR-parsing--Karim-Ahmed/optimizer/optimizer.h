#pragma once
// optimizer.h
// Main optimizer class implementing 5 optimization techniques

#include <string>
#include <map>
#include <set>
#include <vector>
#include "optimizer_types.h"

using namespace std;

class Optimizer {
public:
    explicit Optimizer(OptIR& ir);
    
    // Run all optimization passes
    void optimize();
    
    // Individual optimization techniques
    void constantFolding();
    void constantPropagation();
    void commonSubexpressionElimination();
    void codeMovement();
    void deadCodeElimination();
    
    // Get optimization statistics
    const OptimizationStats& getStats() const { return stats_; }
    
    // Get optimized IR
    const OptIR& getIR() const { return ir_; }

private:
    OptIR& ir_;
    OptimizationStats stats_;
    
    // Helper functions
    bool isConstant(const string& s) const;
    bool isTemporary(const string& s) const;
    int evaluateConstant(const string& op, int left, int right);
    string getExpressionKey(const string& op, const string& arg1, const string& arg2);
    
    // Data flow analysis
    map<string, string> constantValues_;  // variable -> constant value
    map<string, set<int>> variableUses_;  // variable -> set of instruction indices using it
    map<string, int> variableDefs_;       // variable -> instruction index defining it
};
