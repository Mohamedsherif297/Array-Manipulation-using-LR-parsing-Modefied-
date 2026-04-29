// symbol_table_loader.cpp
// Parses symbol_table.json into a CGSymbolTable.
//
// Expected JSON format (produced by semantic_output.cpp):
// {
//   "varName": {
//     "type": "int",
//     "isArray": true,
//     "size1": 3,
//     "size2": 0
//   },
//   ...
// }

#include "symbol_table_loader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace std;

// ---------------------------------------------------------------------------
// Minimal JSON parser for the flat symbol-table format
// ---------------------------------------------------------------------------

static void skipWS(const string& s, size_t& pos) {
    while (pos < s.size() && isspace((unsigned char)s[pos]))
        ++pos;
}

static string parseStr(const string& s, size_t& pos) {
    ++pos; // skip opening '"'
    string result;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\' && pos + 1 < s.size()) {
            ++pos;
            switch (s[pos]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                default:   result += s[pos]; break;
            }
        } else {
            result += s[pos];
        }
        ++pos;
    }
    ++pos; // skip closing '"'
    return result;
}

// Parse a JSON integer (no sign, no float)
static int parseInt(const string& s, size_t& pos) {
    string digits;
    while (pos < s.size() && isdigit((unsigned char)s[pos]))
        digits += s[pos++];
    return digits.empty() ? 0 : stoi(digits);
}

// Parse a JSON boolean: true | false
static bool parseBool(const string& s, size_t& pos) {
    if (s.substr(pos, 4) == "true")  { pos += 4; return true;  }
    if (s.substr(pos, 5) == "false") { pos += 5; return false; }
    return false;
}

// ---------------------------------------------------------------------------

CGSymbolTable loadSymbolTable(const string& filePath) {
    CGSymbolTable table;

    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "[SymTable Loader] Cannot open file: " << filePath << "\n";
        return table;
    }

    ostringstream ss;
    ss << file.rdbuf();
    string s = ss.str();

    size_t pos = 0;
    skipWS(s, pos);

    // Expect outer '{'
    if (pos >= s.size() || s[pos] != '{') {
        cerr << "[SymTable Loader] Expected '{' at start of symbol table.\n";
        return table;
    }
    ++pos; // skip '{'

    skipWS(s, pos);

    while (pos < s.size() && s[pos] != '}') {
        // Parse variable name (key)
        if (s[pos] != '"') { ++pos; continue; }
        string varName = parseStr(s, pos);

        skipWS(s, pos);
        if (pos < s.size() && s[pos] == ':') ++pos;
        skipWS(s, pos);

        // Parse the symbol object '{ ... }'
        if (pos >= s.size() || s[pos] != '{') {
            cerr << "[SymTable Loader] Expected '{' for symbol '" << varName << "'.\n";
            break;
        }
        ++pos; // skip '{'

        CGSymbol sym;
        sym.name = varName;

        skipWS(s, pos);
        while (pos < s.size() && s[pos] != '}') {
            if (s[pos] != '"') { ++pos; continue; }

            string key = parseStr(s, pos);
            skipWS(s, pos);
            if (pos < s.size() && s[pos] == ':') ++pos;
            skipWS(s, pos);

            if (key == "type") {
                sym.type = parseStr(s, pos);
            } else if (key == "isArray") {
                sym.isArray = parseBool(s, pos);
            } else if (key == "size1") {
                sym.size1 = parseInt(s, pos);
            } else if (key == "size2") {
                sym.size2 = parseInt(s, pos);
            } else {
                // Skip unknown value
                if (pos < s.size() && s[pos] == '"') {
                    parseStr(s, pos);
                } else {
                    while (pos < s.size() && s[pos] != ',' && s[pos] != '}')
                        ++pos;
                }
            }

            skipWS(s, pos);
            if (pos < s.size() && s[pos] == ',') ++pos;
            skipWS(s, pos);
        }
        if (pos < s.size()) ++pos; // skip '}'

        table[varName] = sym;

        skipWS(s, pos);
        if (pos < s.size() && s[pos] == ',') ++pos;
        skipWS(s, pos);
    }

    return table;
}
