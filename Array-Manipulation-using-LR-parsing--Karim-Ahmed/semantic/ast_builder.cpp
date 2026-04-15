#include "ast_builder.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace std;

// ---------------------------------------------------------------------------
// Minimal hand-written JSON parser
// We only need to handle the subset produced by printJSON():
//   { "type": "...", "value": "...", "children": [ ... ] }
// ---------------------------------------------------------------------------

static void skipWhitespace(const string& s, size_t& pos) {
    while (pos < s.size() && isspace((unsigned char)s[pos]))
        ++pos;
}

static string parseString(const string& s, size_t& pos) {
    // pos is currently on the opening '"'
    ++pos; // skip "
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
    ++pos; // skip closing "
    return result;
}

// Forward declaration
static shared_ptr<ASTNode> parseObject(const string& s, size_t& pos);

static vector<shared_ptr<ASTNode>> parseArray(const string& s, size_t& pos) {
    // pos is on '['
    ++pos;
    vector<shared_ptr<ASTNode>> items;
    skipWhitespace(s, pos);
    while (pos < s.size() && s[pos] != ']') {
        if (s[pos] == '{') {
            items.push_back(parseObject(s, pos));
        }
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ',') ++pos;
        skipWhitespace(s, pos);
    }
    ++pos; // skip ']'
    return items;
}

static shared_ptr<ASTNode> parseObject(const string& s, size_t& pos) {
    // pos is on '{'
    ++pos;
    auto node = make_shared<ASTNode>();

    skipWhitespace(s, pos);
    while (pos < s.size() && s[pos] != '}') {
        if (s[pos] != '"') { ++pos; continue; }

        string key = parseString(s, pos);
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ':') ++pos;
        skipWhitespace(s, pos);

        if (key == "type") {
            node->type = parseString(s, pos);
        } else if (key == "value") {
            node->value = parseString(s, pos);
        } else if (key == "children") {
            node->children = parseArray(s, pos);
        } else {
            // Unknown key — skip its value (string or nested object/array)
            if (pos < s.size() && s[pos] == '"') {
                parseString(s, pos); // discard
            } else {
                // skip until next comma or closing brace at this depth
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

        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ',') ++pos;
        skipWhitespace(s, pos);
    }
    if (pos < s.size()) ++pos; // skip '}'
    return node;
}

// ---------------------------------------------------------------------------

shared_ptr<ASTNode> loadAST(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "[AST Loader] Cannot open file: " << filePath << "\n";
        return nullptr;
    }

    ostringstream ss;
    ss << file.rdbuf();
    string content = ss.str();

    size_t pos = 0;
    skipWhitespace(content, pos);

    if (pos >= content.size() || content[pos] != '{') {
        cerr << "[AST Loader] Expected JSON object at start of file.\n";
        return nullptr;
    }

    try {
        return parseObject(content, pos);
    } catch (const exception& e) {
        cerr << "[AST Loader] Parse error: " << e.what() << "\n";
        return nullptr;
    }
}
