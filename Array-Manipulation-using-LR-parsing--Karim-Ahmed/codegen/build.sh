#!/bin/bash
# build.sh — compile the codegen module
set -e

CXX=g++
FLAGS="-std=c++17 -Wall -Wextra -I.. -I../semantic"
SRCS="codegen_main.cpp ast_loader.cpp symbol_table_loader.cpp code_generator.cpp"
TARGET="codegen"

echo "=== Building codegen ==="
$CXX $FLAGS $SRCS -o $TARGET
echo "=== Build successful: $TARGET ==="
