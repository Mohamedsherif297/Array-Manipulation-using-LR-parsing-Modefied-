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

{"Program", {"StmtList"}},

{"StmtList", {"StmtList", "Stmt"}},

{"StmtList", {"Stmt"}},

{"Stmt", {"DeclStmt"}},

{"Stmt", {"AssignStmt"}},

{"Stmt", {"DeclAssignStmt"}},

{"DeclStmt", {"Type", "ID", ";"}},

{"DeclStmt", {"Type", "ID", "ArrayDims", ";"}},

{"AssignStmt", {"ID", "=", "Expr", ";"}},

{"AssignStmt", {"ArrayAccess", "=", "Expr", ";"}},

{"DeclAssignStmt", {"Type", "ID", "=", "Expr", ";"}},

{"DeclAssignStmt", {"Type", "ID", "ArrayDims", "=", "ArrayLiteral", ";"}},

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
{"ArrayDims", {"[", "NUM", "]"}},

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

"[","]","{","}",",",

"(",")","$"

};

inline set<string> nonTerminals = {

"S'","Program","StmtList","Stmt",

"DeclStmt","AssignStmt","DeclAssignStmt",

"ArrayLiteral","Elements","Element",

"ArrayAccess","ArrayDims","Expr","Term",

"Factor","Type","ExprList"

};

#endif