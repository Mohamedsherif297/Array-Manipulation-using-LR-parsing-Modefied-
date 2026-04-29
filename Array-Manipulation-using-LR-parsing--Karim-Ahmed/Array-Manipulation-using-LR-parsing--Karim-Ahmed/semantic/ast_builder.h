#pragma once

#include <string>
#include <memory>
#include "ASTNode.h"

// Loads a JSON AST file produced by the parser and converts it into
// an in-memory ASTNode tree.  Returns nullptr on failure.
shared_ptr<ASTNode> loadAST(const string& filePath);
