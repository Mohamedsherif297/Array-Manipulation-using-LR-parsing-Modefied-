#pragma once
// codegen_types.h
// Core data structures for the Intermediate Code Generation phase.
//
// Three-Address Code (TAC) is represented in Quadruple form:
//   (op, arg1, arg2, result)
//
// Examples:
//   t1 = b * c        →  { "*",    "b",  "c",  "t1" }
//   t2 = a + t1       →  { "+",    "a",  "t1", "t2" }
//   x  = t2           →  { "ASSIGN","t2", "",   "x"  }
//   t1 = i * 4        →  { "*",    "i",  "4",  "t1" }
//   t2 = x[t1]        →  { "LOAD", "x",  "t1", "t2" }
//   x[t1] = t2        →  { "STORE","t2", "t1", "x"  }

#include <string>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------------
// Quad: one Three-Address Code instruction
// ---------------------------------------------------------------------------
struct Quad {
    string op;      // operator / instruction mnemonic
    string arg1;    // first operand  (may be empty)
    string arg2;    // second operand (may be empty)
    string result;  // destination    (may be empty for some ops)

    Quad(string op, string arg1, string arg2, string result)
        : op(move(op)), arg1(move(arg1)), arg2(move(arg2)), result(move(result)) {}
};

// The complete IR for a compilation unit
using IR = vector<Quad>;
