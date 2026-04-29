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

        // ── Pre-pass: strip preprocessor directives and using-declarations ──
        // Lines starting with # (e.g. #include <...>) and
        // "using namespace ...;" are valid C++ but outside our grammar.
        // We remove them before tokenizing so the parser never sees them.
        {
            string cleaned;
            cleaned.reserve(n);
            size_t pos = 0;
            while (pos < n) {
                // Skip leading whitespace to check line start
                size_t lineStart = pos;
                // Find the first non-space character on this line
                size_t nonSpace = pos;
                while (nonSpace < n && (src[nonSpace] == ' ' || src[nonSpace] == '\t')) nonSpace++;

                bool skipLine = false;
                // #include, #define, #pragma, etc.
                if (nonSpace < n && src[nonSpace] == '#') skipLine = true;
                // using namespace std; (and similar)
                if (!skipLine && nonSpace + 5 < n && src.substr(nonSpace, 5) == "using") {
                    // Check the rest of the line for "namespace"
                    size_t tmp = nonSpace + 5;
                    while (tmp < n && (src[tmp] == ' ' || src[tmp] == '\t')) tmp++;
                    if (tmp + 9 <= n && src.substr(tmp, 9) == "namespace") skipLine = true;
                }

                if (skipLine) {
                    // Replace the entire line with a newline to preserve line numbers
                    while (pos < n && src[pos] != '\n') pos++;
                    cleaned += '\n';
                    if (pos < n) pos++; // skip the \n itself
                } else {
                    // Copy line as-is
                    while (pos < n && src[pos] != '\n') cleaned += src[pos++];
                    if (pos < n) { cleaned += '\n'; pos++; }
                }
            }
            src = cleaned;
            n   = src.size();
            i   = 0; // reset position
        }

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
                } else if (word == "return" || word == "cout" || word == "cin" || word == "endl" || word == "endLine" || word == "for") {
                    tokens.push_back(Token(TokenType::RESERVED, word, currentLine));
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
            
            // Check for << and >> operators
            if (c == '<' && i < n && src[i] == '<') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "<<", currentLine));
                continue;
            }
            if (c == '>' && i < n && src[i] == '>') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, ">>", currentLine));
                continue;
            }

            // Check for ++ (pre/post increment)
            if (c == '+' && i < n && src[i] == '+') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "++", currentLine));
                continue;
            }

            // Check for two-character relational operators: >=, <=, ==, !=
            if (c == '>' && i < n && src[i] == '=') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, ">=", currentLine));
                continue;
            }
            if (c == '<' && i < n && src[i] == '=') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "<=", currentLine));
                continue;
            }
            if (c == '=' && i < n && src[i] == '=') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "==", currentLine));
                continue;
            }
            if (c == '!' && i < n && src[i] == '=') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "!=", currentLine));
                continue;
            }

            switch (c) {
                case '+': case '-': case '*': case '/': case '=': case '<': case '>':
                    tokens.push_back(Token(TokenType::OPERATOR, string(1, c), currentLine)); break;
                case '[': tokens.push_back(Token(TokenType::LBRACKET, "[", currentLine)); break;
                case ']': tokens.push_back(Token(TokenType::RBRACKET, "]", currentLine)); break;
                case '{': tokens.push_back(Token(TokenType::LBRACE, "{", currentLine)); break;
                case '}': tokens.push_back(Token(TokenType::RBRACE, "}", currentLine)); break;
                case '(': tokens.push_back(Token(TokenType::LPAREN, "(", currentLine)); break;
                case ')': tokens.push_back(Token(TokenType::RPAREN, ")", currentLine)); break;
                case ';': case ',':
                    tokens.push_back(Token(TokenType::SYMBOL, string(1, c), currentLine)); break;
                case '\"': // String literal
                    {
                        string str;
                        while (i < n && src[i] != '\"') str += src[i++];
                        i++; // skip closing "
                        tokens.push_back(Token(TokenType::STRING, str, currentLine));
                    } break;
                case '\'': // Char literal
                    {
                        string ch;
                        if (i < n && src[i] != '\'') {
                            ch += src[i++];
                            if (i < n && src[i] == '\'') {
                                i++; // skip closing '
                                tokens.push_back(Token(TokenType::CHAR, ch, currentLine));
                            } else {
                                cerr << "Lexical Error: Unterminated character literal at line " << currentLine << endl;
                                exit(1);
                            }
                        } else {
                            cerr << "Lexical Error: Empty character literal at line " << currentLine << endl;
                            exit(1);
                        }
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