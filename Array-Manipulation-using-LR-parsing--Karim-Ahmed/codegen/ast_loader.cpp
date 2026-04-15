// ast_loader.cpp
// Parses the annotated AST JSON file into an in-memory ASTNode tree.
//
// The annotated AST is a superset of the raw parser AST: it adds
// "dataType" and "semanticInfo" string fields to each node.  We extend
// the minimal JSON parser from ast_builder.cpp to also read those fields.

#include "ast_loader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace std;

// ---------------------------------------------------------------------------
// Minimal hand-written JSON parser (mirrors ast_builder.cpp, extended for
// the extra semantic fields).
// ---------------------------------------------------------------------------

static void skipWS(const string& s, size_t& pos) {
    while (pos < s.size() && isspace((unsigned char)s[pos]))
        ++pos;
}

static string parseStr(const string& s, size_t& pos) {
    // pos is on the opening '"'
    ++pos;
    string result;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\' && pos + 1 < s.size()) {
            ++pos;
            switch (s[pos]) {
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                case '/':  result += '/';  break;
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
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

// Forward declaration
static shared_ptr<ASTNode> parseObj(const string& s, size_t& pos);

static vector<shared_ptr<ASTNode>> parseArr(const string& s, size_t& pos) {
    ++pos; // skip '['
    vector<shared_ptr<ASTNode>> items;
    skipWS(s, pos);
    while (pos < s.size() && s[pos] != ']') {
        if (s[pos] == '{') {
            items.push_back(parseObj(s, pos));
        }
        skipWS(s, pos);
        if (pos < s.size() && s[pos] == ',') ++pos;
        skipWS(s, pos);
    }
    ++pos; // skip ']'
    return items;
}

static shared_ptr<ASTNode> parseObj(const string& s, size_t& pos) {
    ++pos; // skip '{'
    auto node = make_shared<ASTNode>();

    skipWS(s, pos);
    while (pos < s.size() && s[pos] != '}') {
        if (s[pos] != '"') { ++pos; continue; }

        string key = parseStr(s, pos);
        skipWS(s, pos);
        if (pos < s.size() && s[pos] == ':') ++pos;
        skipWS(s, pos);

        if (key == "type") {
            node->type = parseStr(s, pos);
        } else if (key == "value") {
            node->value = parseStr(s, pos);
        } else if (key == "dataType") {
            node->dataType = parseStr(s, pos);
        } else if (key == "semanticInfo") {
            node->semanticInfo = parseStr(s, pos);
        } else if (key == "children") {
            node->children = parseArr(s, pos);
        } else {
            // Unknown key — skip its value
            if (pos < s.size() && s[pos] == '"') {
                parseStr(s, pos); // discard string value
            } else {
                // Skip nested object / array / primitive
                int depth = 0;
                while (pos < s.size()) {
                    char c = s[pos];
                    if (c == '{' || c == '[') ++depth;
                    else if (c == '}' || c == ']') {
                        if (depth == 0) break;
                        --depth;
                    } else if (c == ',' && depth == 0) break;
                    ++pos;
                }
            }
        }

        skipWS(s, pos);
        if (pos < s.size() && s[pos] == ',') ++pos;
        skipWS(s, pos);
    }
    if (pos < s.size()) ++pos; // skip '}'
    return node;
}

// ---------------------------------------------------------------------------

shared_ptr<ASTNode> loadAnnotatedAST(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "[AST Loader] Cannot open file: " << filePath << "\n";
        return nullptr;
    }

    ostringstream ss;
    ss << file.rdbuf();
    string content = ss.str();

    size_t pos = 0;
    skipWS(content, pos);

    if (pos >= content.size() || content[pos] != '{') {
        cerr << "[AST Loader] Expected JSON object at start of file.\n";
        return nullptr;
    }

    try {
        return parseObj(content, pos);
    } catch (const exception& e) {
        cerr << "[AST Loader] Parse error: " << e.what() << "\n";
        return nullptr;
    }
}
