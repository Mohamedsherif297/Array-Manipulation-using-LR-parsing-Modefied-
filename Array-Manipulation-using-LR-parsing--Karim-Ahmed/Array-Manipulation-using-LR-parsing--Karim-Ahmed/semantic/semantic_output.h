#pragma once

#include <string>
#include <memory>
#include "ASTNode.h"
#include "symbol_table.h"

using namespace std;

// Writes the annotated AST as JSON to the given file path.
void writeAnnotatedAST(shared_ptr<ASTNode> root, const string& filePath);

// Writes the symbol table as JSON to the given file path.
void writeSymbolTable(const SemanticSymbolTable& symTable, const string& filePath);
