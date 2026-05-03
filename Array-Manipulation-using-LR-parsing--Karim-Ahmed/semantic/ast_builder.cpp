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

static string parseValue(const string& s, size_t& pos) {
    // Parse either a string (quoted) or a number
    skipWhitespace(s, pos);
    if (pos < s.size() && s[pos] == '"') {
        return parseString(s, pos);
    } else {
        // Parse a number
        string result;
        while (pos < s.size() && (isdigit(s[pos]) || s[pos] == '-' || s[pos] == '.')) {
            result += s[pos];
            ++pos;
        }
        return result;
    }
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
    
    // Temporary storage for special fields
    string arrayField;
    shared_ptr<ASTNode> indexNode;
    shared_ptr<ASTNode> leftNode;
    shared_ptr<ASTNode> rightNode;
    string operatorField;

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
            node->value = parseValue(s, pos);
        } else if (key == "op") {
            node->op = parseString(s, pos);
        } else if (key == "line") {
            // Parse line number
            skipWhitespace(s, pos);
            string lineStr;
            while (pos < s.size() && isdigit(s[pos])) {
                lineStr += s[pos++];
            }
            if (!lineStr.empty()) {
                node->line = stoi(lineStr);
            }
        } else if (key == "children") {
            node->children = parseArray(s, pos);
        } else if (key == "array") {
            // ArrayAccess: store array name
            arrayField = parseString(s, pos);
        } else if (key == "index") {
            // ArrayAccess: store index node
            if (pos < s.size() && s[pos] == '{') {
                indexNode = parseObject(s, pos);
            }
        } else if (key == "left") {
            // BinaryOp: store left node
            if (pos < s.size() && s[pos] == '{') {
                leftNode = parseObject(s, pos);
            }
        } else if (key == "right") {
            // BinaryOp: store right node
            if (pos < s.size() && s[pos] == '{') {
                rightNode = parseObject(s, pos);
            }
        } else if (key == "operator") {
            // BinaryOp: store operator
            operatorField = parseString(s, pos);
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
    
    // Convert special fields to children format
    if (node->type == "ArrayAccess" && !arrayField.empty() && indexNode) {
        // Convert to children format: children[0] = ID node, children[1] = index
        auto idNode = make_shared<ASTNode>();
        idNode->type = "ID";
        idNode->value = arrayField;
        node->children.push_back(idNode);
        node->children.push_back(indexNode);
    }
    
    if (node->type == "BinaryOp" && leftNode && rightNode) {
        // Convert to children format: children[0] = left, children[1] = operator, children[2] = right
        node->children.push_back(leftNode);
        
        auto opNode = make_shared<ASTNode>();
        opNode->type = operatorField;
        opNode->value = operatorField;
        node->children.push_back(opNode);
        
        node->children.push_back(rightNode);
    }
    
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
