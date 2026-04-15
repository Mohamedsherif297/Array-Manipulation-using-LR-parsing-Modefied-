#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cctype>
#include "Token_Class.h"
#include "Symbol_Table.h"




class Lexer {
private:
    string src;
    size_t i;
    int currentLine;
    SymbolTable& st;
    string last_type = "pending"; // To track the last declared type for identifiers

public:
    Lexer(const string& input, SymbolTable& symbolTable) 
        : src(input), i(0), currentLine(1), st(symbolTable) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        size_t n = src.size();

        while (i < n) {
            if (isspace(src[i])) {
                if (src[i] == '\n') currentLine++;
                i++;
                continue;
            }

            // Comments (Skipping logic remains consistent) [cite: 642]
            if (src[i] == '/' && i + 1 < n) {
                if (src[i + 1] == '/') {
                    i += 2;
                    while (i < n && src[i] != '\n') i++;
                    continue;
                }
                if (src[i + 1] == '*') {
                    i += 2;
                    while (i + 1 < n && !(src[i] == '*' && src[i + 1] == '/')) i++;
                    i += 2;
                    continue;
                }
            }

            // IDENTIFIER or KEYWORD [cite: 718, 725]
            if (isalpha(src[i]) || src[i] == '_') {
                string word;
                while (i < n && (isalnum(src[i]) || src[i] == '_')) word += src[i++];
                
                if (word == "int" || word == "float" || word == "double" || word == "char" || word == "string") {
                    tokens.push_back(Token(TokenType::DATATYPE, word, currentLine));
                    last_type = word; // Update last declared type
                } else {
                    st.addSymbol(word, last_type);
                    
                    
                    // Register identifier in Symbol Table
                    tokens.push_back(Token(TokenType::IDENTIFIER, word, currentLine));
                }
                continue;
            }

            // NUMBER [cite: 726]
            if (isdigit(src[i])) {
                string num;
                bool hasDot = false;
                while (i < n && (isdigit(src[i]) || src[i] == '.')) {
                    if (src[i] == '.') { if (hasDot) break; hasDot = true; }
                    num += src[i++];
                }
                tokens.push_back(Token(TokenType::CONSTANT, num, currentLine));
                continue;
            }

            // Operators & Symbols [cite: 729]
            char c = src[i++];
            switch (c) {
                case '+': case '-': case '*': case '/': case '=':
                    tokens.push_back(Token(TokenType::OPERATOR, string(1, c), currentLine)); break;
                case '[': tokens.push_back(Token(TokenType::LBRACKET, "[", currentLine)); break;
                case ']': tokens.push_back(Token(TokenType::RBRACKET, "]", currentLine)); break;
                case '{': tokens.push_back(Token(TokenType::LBRACE, "{", currentLine)); break;
                case '}': tokens.push_back(Token(TokenType::RBRACE, "}", currentLine)); break;
                case '(': tokens.push_back(Token(TokenType::LPAREN, "(", currentLine)); break;
                case ')': tokens.push_back(Token(TokenType::RPAREN, ")", currentLine)); break;
                case ';': case ',':
                    tokens.push_back(Token(TokenType::SYMBOL, string(1, c), currentLine)); break;
                case '\"': // String logic refactored
                    {
                        string str;
                        while (i < n && src[i] != '\"') str += src[i++];
                        i++; // skip closing "
                        tokens.push_back(Token(TokenType::STRING, str, currentLine));
                    } break;
                default:
                    cerr << "Lexical Error: Invalid character '" << c << "' at line " << currentLine << endl;
                    exit(1);
            }
        }
        tokens.push_back(Token(TokenType::END_OF_FILE, "$", currentLine));
        return tokens;
    }
};


/*
int main() {
    cout<<"Mohamed sherif"<<endl; 
    
    string src= "int arr [5] ; ";
   
    SymbolTable st;
    Lexer lexer(src, st);

    vector<Token> tokens = lexer.tokenize();

    for (const auto& t : tokens) t.display();
    st.display();

    return 0;
}

*/