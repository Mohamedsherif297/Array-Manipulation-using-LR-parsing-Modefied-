#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Parser.h"
#include "../lexer/Lexer.h"

using namespace std;

int main() {

    string src = "int x [2] = {1,2};"
                 "int y [2]= {1,2};"
                 "int z [2]= {x[0]+y[0] , x[1]+y[1]};"
                 "int a = z[0];"
    ;

    SymbolTable st;
    Lexer lexer(src, st);
    vector<Token> tokens = lexer.tokenize();

    computeFIRST();
    buildStates();
    buildParsingTable();

    vector<pair<string,string>> input;

    for (auto &t : tokens) {
    switch (t.getType()) {

        case TokenType::DATATYPE:
            input.push_back({t.getLexeme(), t.getLexeme()});
            break;

        case TokenType::IDENTIFIER:
            input.push_back({"ID", t.getLexeme()});
            break;

        case TokenType::CONSTANT:
            input.push_back({"NUM", t.getLexeme()});
            break;

        case TokenType::STRING:
            input.push_back({"STRING", t.getLexeme()});
            break;

        case TokenType::CHAR:
            input.push_back({"CHAR", t.getLexeme()});
            break;

        case TokenType::END_OF_FILE:
            input.push_back({"$", "$"});
            break;

        default:
            input.push_back({t.getLexeme(), t.getLexeme()});
            break;
        }
    }

    Node* root = parse(input);

    if (root) {
        ofstream file("ast.json");

        if (!file) {
            cout << "Error creating file\n";
            return 1;
        }

        printJSON(root, 0, &file);
        file.close();

        cout << "AST saved to ast.json\n";
    }

    return 0;
}