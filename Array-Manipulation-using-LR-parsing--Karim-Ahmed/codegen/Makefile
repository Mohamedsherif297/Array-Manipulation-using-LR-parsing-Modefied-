# Makefile for the Intermediate Code Generation phase
#
# Builds a standalone executable: codegen
#
# Usage:
#   make          — build
#   make clean    — remove build artefacts
#   make run      — build and run with default input files

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I.. -I../semantic

# Source files (codegen module only — no semantic sources needed at link time
# because we share only headers and the ASTNode struct definition)
SRCS := codegen_main.cpp \
        ast_loader.cpp \
        symbol_table_loader.cpp \
        code_generator.cpp

OBJS := $(SRCS:.cpp=.o)
TARGET := codegen

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Default run: use the annotated AST and symbol table produced by the
# semantic phase (assumed to be in ../semantic/)
run: all
	./$(TARGET) ../semantic/annotated_ast.json ../semantic/symbol_table.json ir.txt

clean:
	rm -f $(OBJS) $(TARGET) ir.txt
