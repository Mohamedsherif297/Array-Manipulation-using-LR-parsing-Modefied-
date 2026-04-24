#pragma once
// ast_loader.h
// Loads the annotated AST JSON (produced by the semantic phase) into an
// in-memory ASTNode tree.
//
// The annotated AST has the same structure as the raw parser AST but also
// carries "dataType" and "semanticInfo" fields on each node.  We reuse the
// same ASTNode struct from the semantic module so that the code generator
// can inspect type information directly.

#include <string>
#include <memory>

// We share ASTNode with the semantic module — include it from there.
#include "../semantic/ASTNode.h"

// Load the annotated AST from a JSON file.
// Returns nullptr on failure.
shared_ptr<ASTNode> loadAnnotatedAST(const string& filePath);
