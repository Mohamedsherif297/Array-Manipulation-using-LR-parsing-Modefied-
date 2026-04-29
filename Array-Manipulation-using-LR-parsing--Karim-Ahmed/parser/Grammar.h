#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <vector>
#include <string>
#include <set>

using namespace std;

struct Production {
    string lhs;
    vector<string> rhs;
};

inline vector <Production> grammar = {

{"S'", {"Program"}},

{"Program", {"GlobalList", "FunctionDef"}},
{"Program", {"FunctionDef"}},

{"GlobalList", {"GlobalList", "GlobalDecl"}},
{"GlobalList", {"GlobalDecl"}},

{"GlobalDecl", {"DeclStmt"}},
{"GlobalDecl", {"DeclAssignStmt"}},

{"FunctionDef", {"Type", "ID", "(", ")", "{", "StmtList", "}"}},

{"StmtList", {"StmtList", "Stmt"}},

{"StmtList", {"Stmt"}},

{"Stmt", {"DeclStmt"}},

{"Stmt", {"AssignStmt"}},

{"Stmt", {"DeclAssignStmt"}},

{"Stmt", {"ReturnStmt"}},

{"Stmt", {"IoStmt"}},

{"Stmt", {"ForStmt"}},

{"Stmt", {"IncrStmt"}},

{"DeclStmt", {"Type", "ID", ";"}},

{"DeclStmt", {"Type", "ID", "ArrayDims", ";"}},

{"AssignStmt", {"ID", "=", "Expr", ";"}},

{"AssignStmt", {"ArrayAccess", "=", "Expr", ";"}},

{"DeclAssignStmt", {"Type", "ID", "=", "Expr", ";"}},

{"DeclAssignStmt", {"Type", "ID", "ArrayDims", "=", "ArrayLiteral", ";"}},

{"ReturnStmt", {"return", "Expr", ";"}},

{"ReturnStmt", {"return", ";"}},

// ===== FOR LOOP (CFG only — no semantic/codegen implementation) =====
// Braced body:   for ( ForInit ; ForCond ; ForUpdate ) { StmtList }
// Braceless body: for ( ForInit ; ForCond ; ForUpdate ) Stmt
{"ForStmt", {"for", "(", "ForInit", ";", "ForCond", ";", "ForUpdate", ")", "{", "StmtList", "}"}},
{"ForStmt", {"for", "(", "ForInit", ";", "ForCond", ";", "ForUpdate", ")", "Stmt"}},

// ForInit: declaration with init, declaration only, or plain assignment
{"ForInit", {"Type", "ID", "=", "Expr"}},
{"ForInit", {"Type", "ID"}},
{"ForInit", {"ID", "=", "Expr"}},

// ForCond: a comparison expression or plain expression
{"ForCond", {"Expr", "RelOp", "Expr"}},
{"ForCond", {"Expr"}},

// Relational operators
{"RelOp", {">"}},
{"RelOp", {"<"}},
{"RelOp", {">="}},
{"RelOp", {"<="}},
{"RelOp", {"=="}},
{"RelOp", {"!="}},

// ForUpdate: increment/decrement or assignment
{"ForUpdate", {"IncrExpr"}},
{"ForUpdate", {"ID", "=", "Expr"}},

// IncrExpr: used inside for-update (no semicolon)
{"IncrExpr", {"++", "ID"}},
{"IncrExpr", {"ID", "++"}},

// ===== INCREMENT STATEMENTS (standalone, with semicolon — fully implemented) =====
{"IncrStmt", {"++", "ID", ";"}},
{"IncrStmt", {"ID", "++", ";"}},

{"IoStmt", {"cout", "CoutList", ";"}},
{"IoStmt", {"cin", "CinList", ";"}},

{"CoutList", {"CoutList", "<<", "Expr"}},
{"CoutList", {"<<", "Expr"}},

{"CinList", {"CinList", ">>", "InputTarget"}},
{"CinList", {">>", "InputTarget"}},

{"InputTarget", {"ID"}},
{"InputTarget", {"ArrayAccess"}},

// ===== ARRAY LITERAL (UPDATED) =====
{"ArrayLiteral", {"{", "Elements", "}"}}, 
{"Elements", {"Elements", ",", "Element"}}, 
{"Elements", {"Element"}}, 
{"Element", {"Expr"}}, 
{"Element", {"ArrayLiteral"}},

// ===== ARRAY ACCESS (UPDATED) =====
{"ArrayAccess", {"ArrayAccess", "[", "Expr", "]"}},
{"ArrayAccess", {"ID", "[", "Expr", "]"}}, 

// ===== REST (UNCHANGED) =====

{"ExprList", {"ExprList", ",", "Expr"}},
{"ExprList", {"Expr"}},

{"ArrayDims", {"ArrayDims", "[", "NUM", "]"}},
{"ArrayDims", {"ArrayDims", "[", "ID", "]"}},
{"ArrayDims", {"[", "NUM", "]"}},
{"ArrayDims", {"[", "ID", "]"}},
{"ArrayDims", {"[", "]"}},

{"Expr", {"Expr", "+", "Term"}},
{"Expr", {"Expr", "-", "Term"}},
{"Expr", {"Term"}},

{"Term", {"Term", "*", "Factor"}},
{"Term", {"Term", "/", "Factor"}},
{"Term", {"Factor"}},

{"Factor", {"(", "Expr", ")"}},
{"Factor", {"ID"}},
{"Factor", {"NUM"}},
{"Factor", {"ArrayAccess"}},
{"Factor", {"STRING"}},
{"Factor", {"CHAR"}},
{"Factor", {"endl"}},
{"Factor", {"endLine"}},
{"Factor", {"++", "ID"}},
{"Factor", {"ID", "++"}},

{"Type", {"int"}},
{"Type", {"float"}},
{"Type", {"double"}},
{"Type", {"char"}},
{"Type", {"string"}}

};

inline set<string> terminals = {

"int","float","double","char","string",

"ID","NUM","STRING","CHAR",

"+","-","*","/","=",";",

"<<",">>","++",

">","<",">=","<=","==","!=",

"[","]","{","}",",",

"(",")","return","cout","cin","endl","endLine","for","$"

};

inline set<string> nonTerminals = {

"S'","Program","FunctionDef","GlobalList","GlobalDecl","StmtList","Stmt",

"DeclStmt","AssignStmt","DeclAssignStmt","ReturnStmt",

"ForStmt","ForInit","ForCond","ForUpdate","IncrExpr","IncrStmt","RelOp",

"IoStmt","CoutList","CinList","InputTarget",

"ArrayLiteral","Elements","Element",

"ArrayAccess","ArrayDims","Expr","Term",

"Factor","Type","ExprList"

};

#endif