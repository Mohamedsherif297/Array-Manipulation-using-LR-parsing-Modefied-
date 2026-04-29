#pragma once

#include <string>
#include <iostream>

using namespace std ; 

/**
 * TokenType: Defines the categories of tokens found in the source code.
 * These categories align with the syntactical units identified by the Scanner.
 */
enum class TokenType {
    DATATYPE,      // int, float, double, char, string [cite: 725]
    IDENTIFIER,    // Variable or array names [cite: 725]
    CONSTANT,      // Numeric values [cite: 726]
    STRING,        // String values [cite: 726]
    CHAR,          // Character literals [cite: 726]
    SYMBOL,        // Punctuation like parentheses, commas [cite: 727]
    LBRACKET,      // [ 
    RBRACKET,      // ] 
    LPAREN,        // (
    RPAREN,        // ) 
    LBRACE,        // {
    RBRACE,        // }  
    RESERVED,      // Language-specific keywords [cite: 729]
    OPERATOR,      // Symbols like +, *, =, [ , ] [cite: 729]
    END_OF_FILE,   // Special marker for end of input ($) [cite: 2114]
    ILLEGAL        // For reporting lexical errors [cite: 723]
};

/*
 * Token: Encapsulates the output of the Lexical Analyzer.
 * This structure is passed to the Syntax Analysis (Parser) phase.
 */
class Token {
private:
    TokenType type;
    string lexeme;
    int lineNumber;

public:
    // Constructor to initialize token properties
    Token(TokenType type, string lexeme, int lineNumber)
        : type(type), lexeme(lexeme), lineNumber(lineNumber) {}

    // Getters for the Parser to access token data
    TokenType getType() const { return type; }
    
    string getLexeme() const { return lexeme; }
    
    int getLineNumber() const { return lineNumber; }

    string getTypeAsString() const {
        switch (type) {
            case TokenType::DATATYPE: return "DATATYPE";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::CONSTANT: return "CONSTANT";
            case TokenType::STRING: return "STRING";
            case TokenType::CHAR: return "CHAR";
            case TokenType::SYMBOL: return "SYMBOL";
            case TokenType::LBRACKET: return "LBRACKET";
            case TokenType::RBRACKET: return "RBRACKET";
            case TokenType::LPAREN: return "LPAREN";
            case TokenType::RPAREN: return "RPAREN";
            case TokenType::LBRACE: return "LBRACE";
            case TokenType::RBRACE: return "RBRACE";
            case TokenType::RESERVED: return "RESERVED";
            case TokenType::OPERATOR: return "OPERATOR";
            case TokenType::END_OF_FILE: return "END_OF_FILE";
            default: return "ILLEGAL";
        }
    }

   

    // Formatted output for debugging and "best view" requirements
    void display() const {
        std::cout << "[Line " << lineNumber << "] Token: " 
                  << lexeme << " | Type: " <<getTypeAsString()<<" [ " <<static_cast<int>(type) <<" ]"<<std::endl;
                  
    }
};

