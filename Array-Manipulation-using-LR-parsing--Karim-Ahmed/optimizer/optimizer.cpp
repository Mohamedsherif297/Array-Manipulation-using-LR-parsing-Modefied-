#include "optimizer.h"
#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

Optimizer::Optimizer(OptIR& ir) : ir_(ir) {}

void Optimizer::optimize() {
    cout << "\n[Optimizer] Starting optimization passes...\n";
    
    int iteration = 0;
    int prevOptimizations = 0;
    
    // Run optimization passes until no more optimizations are found
    do {
        iteration++;
        prevOptimizations = stats_.totalOptimizations();
        
        cout << "[Optimizer] Iteration " << iteration << "...\n";
        
        // Run optimization passes in order
        constantFolding();
        constantPropagation();
        commonSubexpressionElimination();
        deadCodeElimination();
        codeMovement();
        
        int currentOptimizations = stats_.totalOptimizations();
        
        // Stop if no new optimizations were made
        if (currentOptimizations == prevOptimizations) {
            break;
        }
        
        // Safety limit
        if (iteration > 10) {
            cout << "[Optimizer] Maximum iterations reached\n";
            break;
        }
        
    } while (true);
    
    cout << "[Optimizer] Optimization complete after " << iteration << " iteration(s)\n";
}

bool Optimizer::isConstant(const string& s) const {
    if (s.empty()) return false;
    // Check if it's a number
    return isdigit(s[0]) || (s[0] == '-' && s.length() > 1 && isdigit(s[1]));
}

bool Optimizer::isTemporary(const string& s) const {
    return !s.empty() && s[0] == 't' && s.length() > 1 && isdigit(s[1]);
}

int Optimizer::evaluateConstant(const string& op, int left, int right) {
    if (op == "+") return left + right;
    if (op == "-") return left - right;
    if (op == "*") return left * right;
    if (op == "/" && right != 0) return left / right;
    return 0;
}

string Optimizer::getExpressionKey(const string& op, const string& arg1, const string& arg2) {
    return arg1 + " " + op + " " + arg2;
}

// ============================================================================
// OPTIMIZATION 1: CONSTANT FOLDING
// Evaluates constant expressions at compile time
// Example: t1 = 2 + 3  →  t1 = 5
// ============================================================================
void Optimizer::constantFolding() {
    for (auto& quad : ir_) {
        if (!quad.isAlive) continue;
        
        // Check for binary operations with constant operands
        if ((quad.op == "+" || quad.op == "-" || quad.op == "*" || quad.op == "/") &&
            isConstant(quad.arg1) && isConstant(quad.arg2)) {
            
            int left = stoi(quad.arg1);
            int right = stoi(quad.arg2);
            int result = evaluateConstant(quad.op, left, right);
            
            // Replace with assignment
            quad.op = "ASSIGN";
            quad.arg1 = to_string(result);
            quad.arg2 = "";
            
            stats_.constantFolding++;
            cout << "  [Constant Folding] Folded expression to " << result << "\n";
        }
    }
}

// ============================================================================
// OPTIMIZATION 2: CONSTANT PROPAGATION
// Replaces variables with their constant values
// Example: x = 5; y = x + 3  →  x = 5; y = 5 + 3
// NOTE: LABEL / GOTO / IF_FALSE are control-flow barriers — we flush all
//       tracked constants when we encounter one, because a variable modified
//       inside a loop body must not be treated as constant on the next
//       iteration.
// ============================================================================
void Optimizer::constantPropagation() {
    constantValues_.clear();
    
    for (auto& quad : ir_) {
        if (!quad.isAlive) continue;

        // Control-flow barrier: flush all tracked constants
        if (quad.op == "LABEL" || quad.op == "GOTO" || quad.op == "IF_FALSE") {
            constantValues_.clear();
            continue;
        }
        
        // Track constant assignments
        if (quad.op == "ASSIGN" && isConstant(quad.arg1)) {
            constantValues_[quad.result] = quad.arg1;
        }
        // If variable is reassigned with non-constant, remove from tracking
        else if (quad.op == "ASSIGN" && !isConstant(quad.arg1)) {
            constantValues_.erase(quad.result);
        }
        // For other operations, invalidate result variable
        else if (!quad.result.empty() && quad.op != "DECL" && quad.op != "LOAD" && quad.op != "STORE") {
            constantValues_.erase(quad.result);
        }
        
        // Propagate constants in operands
        bool propagated = false;
        
        if (!quad.arg1.empty() && constantValues_.count(quad.arg1)) {
            quad.arg1 = constantValues_[quad.arg1];
            propagated = true;
        }
        
        if (!quad.arg2.empty() && constantValues_.count(quad.arg2)) {
            quad.arg2 = constantValues_[quad.arg2];
            propagated = true;
        }
        
        if (propagated) {
            stats_.constantPropagation++;
            cout << "  [Constant Propagation] Propagated constant value\n";
        }
    }
}

// ============================================================================
// OPTIMIZATION 3: COMMON SUBEXPRESSION ELIMINATION (CSE)
// Eliminates redundant computations
// Example: t1 = a + b; t2 = a + b  →  t1 = a + b; t2 = t1
// NOTE: LABEL is a barrier — reset the expression table at every label so
//       we don't reuse values computed before a loop back-edge.
// ============================================================================
void Optimizer::commonSubexpressionElimination() {
    map<string, string> expressions; // expression -> result variable
    
    for (auto& quad : ir_) {
        if (!quad.isAlive) continue;

        // Control-flow barrier: reset expression table
        if (quad.op == "LABEL" || quad.op == "GOTO" || quad.op == "IF_FALSE") {
            expressions.clear();
            continue;
        }
        
        // Only consider arithmetic operations
        if (quad.op == "+" || quad.op == "-" || quad.op == "*" || quad.op == "/") {
            string exprKey = getExpressionKey(quad.op, quad.arg1, quad.arg2);
            
            // Check if this expression was already computed
            if (expressions.count(exprKey)) {
                string previousResult = expressions[exprKey];
                
                // Replace with assignment from previous result
                quad.op = "ASSIGN";
                quad.arg1 = previousResult;
                quad.arg2 = "";
                
                stats_.commonSubexprElimination++;
                cout << "  [CSE] Eliminated common subexpression: " << exprKey << "\n";
            } else {
                // Record this expression
                expressions[exprKey] = quad.result;
            }
        }
        
        // Invalidate expressions when variables are modified
        if (quad.op == "ASSIGN" || quad.op == "LOAD") {
            // Remove expressions containing the modified variable
            auto it = expressions.begin();
            while (it != expressions.end()) {
                if (it->first.find(quad.result) != string::npos) {
                    it = expressions.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}

// ============================================================================
// OPTIMIZATION 4: CODE MOVEMENT (Loop-Invariant Code Motion)
// Moves loop-invariant computations outside loops.
// NOTE: We never move or reorder LABEL / GOTO / IF_FALSE instructions —
//       doing so would corrupt the control-flow graph.
// ============================================================================
void Optimizer::codeMovement() {
    vector<OptQuad> constantComputations;
    vector<OptQuad> otherInstructions;
    
    for (auto& quad : ir_) {
        if (!quad.isAlive) continue;

        // Never move control-flow instructions
        if (quad.op == "LABEL" || quad.op == "GOTO" || quad.op == "IF_FALSE") {
            otherInstructions.push_back(quad);
            continue;
        }
        
        // Identify loop-invariant code (constant computations)
        if ((quad.op == "+" || quad.op == "-" || quad.op == "*" || quad.op == "/") &&
            isConstant(quad.arg1) && isConstant(quad.arg2)) {
            constantComputations.push_back(quad);
            stats_.codeMovement++;
            cout << "  [Code Movement] Moved constant computation\n";
        } else {
            otherInstructions.push_back(quad);
        }
    }
    
    // Only apply if we found constant computations to move
    if (!constantComputations.empty()) {
        ir_.clear();
        
        // Add DECL statements first
        for (const auto& quad : otherInstructions) {
            if (quad.op == "DECL") {
                ir_.push_back(quad);
            }
        }
        
        // Add constant computations
        for (const auto& quad : constantComputations) {
            ir_.push_back(quad);
        }
        
        // Add remaining instructions
        for (const auto& quad : otherInstructions) {
            if (quad.op != "DECL") {
                ir_.push_back(quad);
            }
        }
    }
}

// ============================================================================
// OPTIMIZATION 5: DEAD CODE ELIMINATION
// Removes instructions whose results are never used.
// NOTE: LABEL / GOTO / IF_FALSE are never dead — they control execution flow.
//       Temporaries used in IF_FALSE (condition temps) must also be kept.
// ============================================================================
void Optimizer::deadCodeElimination() {
    variableUses_.clear();
    variableDefs_.clear();
    
    // First pass: identify all uses and definitions
    for (size_t i = 0; i < ir_.size(); i++) {
        const auto& quad = ir_[i];
        if (!quad.isAlive) continue;
        
        // Track uses (including the condition operand of IF_FALSE)
        if (!quad.arg1.empty() && !isConstant(quad.arg1)) {
            variableUses_[quad.arg1].insert(i);
        }
        if (!quad.arg2.empty() && !isConstant(quad.arg2)) {
            variableUses_[quad.arg2].insert(i);
        }
        // IF_FALSE uses arg1 as the condition
        if (quad.op == "IF_FALSE" && !quad.arg1.empty()) {
            variableUses_[quad.arg1].insert(i);
        }
        
        // Track definitions
        if (!quad.result.empty() && quad.op != "DECL") {
            variableDefs_[quad.result] = i;
        }
    }
    
    // Second pass: mark dead code
    for (size_t i = 0; i < ir_.size(); i++) {
        auto& quad = ir_[i];
        if (!quad.isAlive) continue;
        
        // Never eliminate control-flow instructions
        if (quad.op == "LABEL" || quad.op == "GOTO" || quad.op == "IF_FALSE") {
            continue;
        }
        
        // Skip DECL, STORE, LOAD (side effects)
        // Note: PRINT, PRINTLN, READ are no longer generated (rejected at semantic analysis)
        if (quad.op == "DECL"  || quad.op == "STORE" || quad.op == "LOAD" ||
            quad.op == "PRINT" || quad.op == "PRINTLN" || quad.op == "READ") {
            continue;
        }
        
        // Check if result is never used
        if (!quad.result.empty()) {
            if (isTemporary(quad.result) && variableUses_[quad.result].empty()) {
                quad.isAlive = false;
                stats_.deadCodeElimination++;
                cout << "  [Dead Code] Eliminated unused instruction: " << quad.result << "\n";
            }
        }
    }
}
