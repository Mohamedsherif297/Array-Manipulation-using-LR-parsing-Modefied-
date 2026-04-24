#pragma once
// ir_loader.h
// Loads Three-Address Code from the codegen phase

#include <string>
#include <vector>
#include "optimizer_types.h"

using namespace std;

// Load IR from TAC file produced by codegen
OptIR loadIR(const string& filePath);

// Write optimized IR back to file
bool writeOptimizedIR(const OptIR& ir, const string& filePath);
