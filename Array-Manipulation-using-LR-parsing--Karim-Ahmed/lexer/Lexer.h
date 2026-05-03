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
    bool hasUsingNamespaceStd = false; // Track if "using namespace std;" is declared

public:
    bool hasIostreamInclude = false; // Global flag to track #include <iostream>
    
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

            // Handle # for preprocessor directives
            if (src[i] == '#') {
                i++; // skip #
                while (i < n && isspace(src[i]) && src[i] != '\n') i++; // skip spaces
                
                string directive;
                while (i < n && isalpha(src[i])) directive += src[i++];
                
                if (directive == "include") {
                    while (i < n && isspace(src[i]) && src[i] != '\n') i++; // skip spaces
                    
                    if (i < n && src[i] == '<') {
                        i++; // skip <
                        string header;
                        while (i < n && src[i] != '>') header += src[i++];
                        if (i < n && src[i] == '>') i++; // skip >
                        
                        // Only allow <iostream>
                        if (header == "iostream") {
                            hasIostreamInclude = true; // Set global flag
                            tokens.push_back(Token(TokenType::RESERVED, "#", currentLine));
                            tokens.push_back(Token(TokenType::RESERVED, "include", currentLine));
                            tokens.push_back(Token(TokenType::OPERATOR, "<", currentLine));
                            tokens.push_back(Token(TokenType::RESERVED, "iostream", currentLine));
                            tokens.push_back(Token(TokenType::OPERATOR, ">", currentLine));
                        } else {
                            cerr << "Lexical Error: Only '#include <iostream>' is supported at line " << currentLine << endl;
                            exit(1);
                        }
                    } else {
                        cerr << "Lexical Error: Expected '<' after #include at line " << currentLine << endl;
                        exit(1);
                    }
                } else {
                    cerr << "Lexical Error: Unknown preprocessor directive '#" << directive << "' at line " << currentLine << endl;
                    exit(1);
                }
                continue;
            }

            // IDENTIFIER or KEYWORD [cite: 718, 725]
            if (isalpha(src[i]) || src[i] == '_') {
                string word;
                while (i < n && (isalnum(src[i]) || src[i] == '_')) word += src[i++];
                
                // Handle "using namespace std;"
                if (word == "using") {
                    while (i < n && isspace(src[i]) && src[i] != '\n') i++;
                    
                    string next;
                    while (i < n && isalpha(src[i])) next += src[i++];
                    
                    if (next == "namespace") {
                        while (i < n && isspace(src[i]) && src[i] != '\n') i++;
                        
                        string ns;
                        while (i < n && isalpha(src[i])) ns += src[i++];
                        
                        if (ns == "std") {
                            while (i < n && isspace(src[i]) && src[i] != '\n') i++;
                            
                            if (i < n && src[i] == ';') {
                                i++; // skip ;
                                hasUsingNamespaceStd = true;
                                tokens.push_back(Token(TokenType::RESERVED, "using", currentLine));
                                tokens.push_back(Token(TokenType::RESERVED, "namespace", currentLine));
                                tokens.push_back(Token(TokenType::RESERVED, "std", currentLine));
                                tokens.push_back(Token(TokenType::SYMBOL, ";", currentLine));
                            } else {
                                cerr << "Lexical Error: Expected ';' after 'using namespace std' at line " << currentLine << endl;
                                exit(1);
                            }
                        } else {
                            cerr << "Lexical Error: Only 'using namespace std;' is supported at line " << currentLine << endl;
                            exit(1);
                        }
                    } else {
                        cerr << "Lexical Error: Expected 'namespace' after 'using' at line " << currentLine << endl;
                        exit(1);
                    }
                    continue;
                }
                
                // Handle std::cin and std::cout
                if (word == "std") {
                    while (i < n && isspace(src[i]) && src[i] != '\n') i++;
                    
                    if (i + 1 < n && src[i] == ':' && src[i + 1] == ':') {
                        i += 2; // skip ::
                        while (i < n && isspace(src[i]) && src[i] != '\n') i++;
                        
                        string member;
                        while (i < n && isalpha(src[i])) member += src[i++];
                        
                        if (member == "cin" || member == "cout") {
                            tokens.push_back(Token(TokenType::RESERVED, member, currentLine));
                        } else {
                            cerr << "Lexical Error: Only 'std::cin' and 'std::cout' are supported at line " << currentLine << endl;
                            exit(1);
                        }
                    } else {
                        cerr << "Lexical Error: Expected '::' after 'std' at line " << currentLine << endl;
                        exit(1);
                    }
                    continue;
                }
                
                // Handle cin and cout (require using namespace std)
                if (word == "cin" || word == "cout") {
                    if (!hasUsingNamespaceStd) {
                        cerr << "Lexical Error: '" << word << "' requires 'std::' prefix or 'using namespace std;' at line " << currentLine << endl;
                        exit(1);
                    }
                    tokens.push_back(Token(TokenType::RESERVED, word, currentLine));
                    continue;
                }
                
                if (word == "int" || word == "float" || word == "double" || word == "char" || word == "string" || word == "bool") {
                    tokens.push_back(Token(TokenType::DATATYPE, word, currentLine));
                    last_type = word; // Update last declared type
                } else if (word == "true" || word == "false") {
                    tokens.push_back(Token(TokenType::CONSTANT, word, currentLine));
                } else if (word == "return" || word == "endl" || word == "endLine" || word == "for" || word == "namespace" || word == "include") {
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
            
            // Check for << operator (cout)
            if (c == '<' && i < n && src[i] == '<') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "<<", currentLine));
                continue;
            }
            
            // Check for >> operator (cin)
            if (c == '>' && i < n && src[i] == '>') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, ">>", currentLine));
                continue;
            }
            
            // Check for <= and >= operators
            if (c == '<' && i < n && src[i] == '=') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "<=", currentLine));
                continue;
            }
            if (c == '>' && i < n && src[i] == '=') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, ">=", currentLine));
                continue;
            }
            
            // Check for == and != operators
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
            
            // Check for :: operator
            if (c == ':' && i < n && src[i] == ':') {
                i++;
                tokens.push_back(Token(TokenType::OPERATOR, "::", currentLine));
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
                case ';': case ',': case ':':
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