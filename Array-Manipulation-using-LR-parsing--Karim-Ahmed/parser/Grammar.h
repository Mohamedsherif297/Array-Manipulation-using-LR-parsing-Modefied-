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

{"Program", {"Preamble", "GlobalList", "FunctionDef"}},
{"Program", {"Preamble", "FunctionDef"}},
{"Program", {"GlobalList", "FunctionDef"}},
{"Program", {"FunctionDef"}},

{"Preamble", {"Preamble", "PreambleStmt"}},
{"Preamble", {"PreambleStmt"}},

{"PreambleStmt", {"IncludeStmt"}},
{"PreambleStmt", {"UsingStmt"}},

{"IncludeStmt", {"#", "include", "<", "iostream", ">"}},

{"UsingStmt", {"using", "namespace", "std", ";"}},

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

{"Stmt", {"ForStmt"}},

{"Stmt", {"IoStmt"}},

{"DeclStmt", {"Type", "DeclList", ";"}},

{"DeclList", {"DeclList", ",", "Declarator"}},
{"DeclList", {"Declarator"}},

{"Declarator", {"ID"}},
{"Declarator", {"ID", "ArrayDims"}},

{"AssignStmt", {"ID", "=", "Expr", ";"}},

{"AssignStmt", {"ArrayAccess", "=", "Expr", ";"}},

{"DeclAssignStmt", {"Type", "ID", "=", "Expr", ";"}},

{"DeclAssignStmt", {"Type", "ID", "ArrayDims", "=", "ArrayLiteral", ";"}},

{"ReturnStmt", {"return", "Expr", ";"}},

{"ReturnStmt", {"return", ";"}},

{"ForStmt", {"for", "(", "ForInit", ";", "ForCond", ";", "ForUpdate", ")", "{", "StmtList", "}"}},
{"ForStmt", {"for", "(", "ForInit", ";", "ForCond", ";", "ForUpdate", ")", "{", "}"}},

{"ForInit", {"Type", "ID", "=", "Expr"}},
{"ForInit", {"ID", "=", "Expr"}},

{"ForCond", {"Expr"}},

{"ForUpdate", {"ID", "=", "Expr"}},

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
{"ArrayDims", {"[", "]"}},

{"Expr", {"Expr", "+", "Term"}},
{"Expr", {"Expr", "-", "Term"}},
{"Expr", {"Term"}},

{"Term", {"Term", "*", "Factor"}},
{"Term", {"Term", "/", "Factor"}},
{"Term", {"Factor"}},

{"Factor", {"(", "Expr", ")"}},
{"Factor", {"Comparison"}},
{"Factor", {"ID"}},
{"Factor", {"NUM"}},
{"Factor", {"ArrayAccess"}},
{"Factor", {"STRING"}},
{"Factor", {"CHAR"}},
{"Factor", {"true"}},
{"Factor", {"false"}},
{"Factor", {"endl"}},
{"Factor", {"endLine"}},

{"Comparison", {"CompTerm", "<", "CompTerm"}},
{"Comparison", {"CompTerm", ">", "CompTerm"}},
{"Comparison", {"CompTerm", "<=", "CompTerm"}},
{"Comparison", {"CompTerm", ">=", "CompTerm"}},
{"Comparison", {"CompTerm", "==", "CompTerm"}},
{"Comparison", {"CompTerm", "!=", "CompTerm"}},

{"CompTerm", {"ID"}},
{"CompTerm", {"NUM"}},
{"CompTerm", {"ArrayAccess"}},

{"Type", {"int"}},
{"Type", {"float"}},
{"Type", {"double"}},
{"Type", {"char"}},
{"Type", {"string"}},
{"Type", {"bool"}},

{"IoStmt", {"cout", "CoutList", ";"}},
{"IoStmt", {"cin", "CinList", ";"}},

{"CoutList", {"CoutList", "<<", "Expr"}},
{"CoutList", {"<<", "Expr"}},

{"CinList", {"CinList", ">>", "InputTarget"}},
{"CinList", {">>", "InputTarget"}},

{"InputTarget", {"ID"}},
{"InputTarget", {"ArrayAccess"}}

};

inline set<string> terminals = {

"int","float","double","char","string","bool",

"ID","NUM","STRING","CHAR",

"+","-","*","/","=",";",

"<",">","<=",">=","==","!=","<<",">>",

"[","]","{","}",",",

"(",")","return","endl","endLine","for",

"true","false",

"#","include","iostream","using","namespace","std",

"cin","cout",

"$"

};

inline set<string> nonTerminals = {

"S'","Program","Preamble","PreambleStmt","IncludeStmt","UsingStmt",

"FunctionDef","GlobalList","GlobalDecl","StmtList","Stmt",

"DeclStmt","DeclList","Declarator","AssignStmt","DeclAssignStmt","ReturnStmt",

"ForStmt","ForInit","ForCond","ForUpdate",

"IoStmt","CoutList","CinList","InputTarget",

"ArrayLiteral","Elements","Element",

"ArrayAccess","ArrayDims","Expr","Term",

"Factor","Comparison","CompTerm","Type","ExprList"

};

#endif