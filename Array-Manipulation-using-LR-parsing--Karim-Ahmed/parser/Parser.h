#pragma once

#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include "Parsing_Table.cpp"

using namespace std;

struct Node {
    string type;
    string value;
    string op;
    string arrayName;
    int lineNumber = 0;  // Track line number for error reporting
    Node* left = nullptr;
    Node* right = nullptr;
    Node* index = nullptr;
    vector<Node*> children;
};

Node* parse(vector<tuple<string,string,int>> input) {

    // Helper function to get line number from children (use first child's line)
    auto getLineFromChildren = [](const vector<Node*>& children) -> int {
        for (auto* child : children) {
            if (child && child->lineNumber > 0) {
                return child->lineNumber;
            }
        }
        return 0;
    };

    stack<int> stateStack;
    stack<Node*> nodeStack;
    int previousLineNum = 1;  // Track previous token's line for better error reporting

    stateStack.push(0);
    int index = 0;

    while (true) {

        int state = stateStack.top();
        string token = get<0>(input[index]);
        string value = get<1>(input[index]);
        int lineNum = get<2>(input[index]);

        if (!ACTION.count({state, token})) {

            // Determine the actual error line
            // If we're expecting a semicolon and the current token is on a new line,
            // the error is likely on the previous line
            int errorLine = lineNum;
            bool expectingSemicolon = false;
            
            for (auto &entry : ACTION) {
                if (entry.first.first == state && entry.first.second == ";") {
                    expectingSemicolon = true;
                    break;
                }
            }
            
            // If expecting semicolon and current token is on next line, error is on previous line
            if (expectingSemicolon && lineNum > previousLineNum) {
                errorLine = previousLineNum;
            }

            cout << "\n========================================\n";
            cout << "  ❌ SYNTAX ERROR DETECTED\n";
            cout << "========================================\n";
            cout << "Error at line " << errorLine << "\n";
            
            if (expectingSemicolon) {
                // If we are still on the same line and got another token,
                // that token is usually the real issue (e.g. "Karim"s).
                if (lineNum == errorLine && token != ";" && token != "$") {
                    cout << "Unexpected token after expression: " << value
                         << " (token: " << token << ")\n";
                    cout << "Expected ';' before this token\n";
                } else {
                    cout << "Missing semicolon ';'\n";
                }
            } else {
                cout << "Unexpected token: " << token
                     << " (value: " << value << ")\n";
            }
            
            cout << "Expected one of: ";

            for (auto &entry : ACTION) {
                if (entry.first.first == state) {
                    cout << entry.first.second << " ";
                }
            }

            cout << "\n========================================\n";
            return nullptr;
        }

        previousLineNum = lineNum;  // Track this line for next iteration
        string action = ACTION[{state, token}];

        // ================= SHIFT =================
        if (action[0] == 'S') {
            int nextState = stoi(action.substr(1));

            Node* node = new Node();
            node->type = token;
            node->value = value;
            node->lineNumber = lineNum;  // Store line number

            nodeStack.push(node);
            stateStack.push(nextState);
            index++;
        }

        // ================= REDUCE =================
        else if (action[0] == 'R') {

            int prodIndex = stoi(action.substr(1));
            Production p = grammar[prodIndex];

            vector<Node*> children;

            for (int i = 0; i < p.rhs.size(); i++) {
                stateStack.pop();
                children.push_back(nodeStack.top());
                nodeStack.pop();
            }

            reverse(children.begin(), children.end());

            Node* newNode = nullptr;

            // ================= PROGRAM =================
            if (p.lhs == "Program") {
                if (children.size() == 2) {
                    // GlobalList FunctionDef — wrap into a Program node
                    Node* node = new Node();
                    node->type = "Program";
                    node->lineNumber = getLineFromChildren(children);
                    // Flatten global decls as children, then append FunctionDef
                    for (auto* c : children[0]->children)
                        node->children.push_back(c);
                    node->children.push_back(children[1]); // FunctionDef
                    newNode = node;
                } else {
                    // Just FunctionDef — keep existing behaviour
                    newNode = children[0];
                }
            }

            // ================= GLOBAL LIST =================
            else if (p.lhs == "GlobalList") {
                Node* node = new Node();
                node->type = "GlobalList";
                node->lineNumber = getLineFromChildren(children);
                if (children.size() == 2) {
                    // GlobalList GlobalDecl
                    node->children = children[0]->children;
                    node->children.push_back(children[1]);
                } else {
                    // GlobalDecl
                    node->children.push_back(children[0]);
                }
                newNode = node;
            }

            // ================= GLOBAL DECL =================
            else if (p.lhs == "GlobalDecl") {
                newNode = children[0]; // transparent — DeclStmt or DeclAssignStmt
            }

            // ================= FUNCTION DEFINITION =================
            else if (p.lhs == "FunctionDef") {
                // Type ID ( ) { StmtList }
                Node* node = new Node();
                node->type = "FunctionDef";
                node->lineNumber = getLineFromChildren(children);
                node->children.push_back(children[0]); // return type
                node->children.push_back(children[1]); // function name
                node->children.push_back(children[5]); // body (StmtList)
                newNode = node;
            }

            else if (p.lhs == "StmtList") {
                Node* node = new Node();
                node->type = "StmtList";
                node->lineNumber = getLineFromChildren(children);

                if (children.size() == 2) {
                    node->children = children[0]->children;
                    node->children.push_back(children[1]);
                } else {
                    node->children.push_back(children[0]);
                }

                newNode = node;
            }

            else if (p.lhs == "Stmt") {
                newNode = children[0];
            }

            // ================= DECL =================
            else if (p.lhs == "DeclStmt") {
                Node* node = new Node();
                node->type = "Declaration";
                node->lineNumber = getLineFromChildren(children);
                node->children.push_back(children[0]); // Type
                node->children.push_back(children[1]); // ID
                
                // Check if there are array dimensions (Type ID ArrayDims ;)
                if (children.size() == 4) {
                    node->children.push_back(children[2]); // ArrayDims
                }
                
                newNode = node;
            }

            // ================= ASSIGN =================
            else if (p.lhs == "AssignStmt") {
                Node* node = new Node();
                node->type = "Assignment";
                node->lineNumber = getLineFromChildren(children);
                node->children = {children[0], children[2]};
                newNode = node;
            }

            // ================= RETURN =================
            else if (p.lhs == "ReturnStmt") {
                Node* node = new Node();
                node->type = "Return";
                node->lineNumber = getLineFromChildren(children);
                if (children.size() == 3) {
                    // return Expr ;
                    node->children.push_back(children[1]); // expression
                }
                // else: return ; (no expression)
                newNode = node;
            }

            // ================= I/O (cout / cin) =================
            else if (p.lhs == "IoStmt") {
                Node* node = new Node();
                node->lineNumber = getLineFromChildren(children);

                if (!children.empty() && children[0]->value == "cout") {
                    node->type = "Output";
                    if (children.size() >= 2) {
                        node->children = children[1]->children;
                    }
                } else {
                    node->type = "Input";
                    if (children.size() >= 2) {
                        node->children = children[1]->children;
                    }
                }

                newNode = node;
            }

            else if (p.lhs == "CoutList") {
                Node* node = new Node();
                node->type = "CoutList";
                node->lineNumber = getLineFromChildren(children);

                if (children.size() == 3) {
                    // CoutList << Expr
                    node->children = children[0]->children;
                    node->children.push_back(children[2]);
                } else {
                    // << Expr
                    node->children.push_back(children[1]);
                }

                newNode = node;
            }

            else if (p.lhs == "CinList") {
                Node* node = new Node();
                node->type = "CinList";
                node->lineNumber = getLineFromChildren(children);

                if (children.size() == 3) {
                    // CinList >> InputTarget
                    node->children = children[0]->children;
                    node->children.push_back(children[2]);
                } else {
                    // >> InputTarget
                    node->children.push_back(children[1]);
                }

                newNode = node;
            }

            else if (p.lhs == "InputTarget") {
                newNode = children[0];
            }

            // ================= DECL + ASSIGN =================
            else if (p.lhs == "DeclAssignStmt") {
                Node* node = new Node();
                node->type = "DeclAssign";
                node->lineNumber = getLineFromChildren(children);

                node->children.push_back(children[0]); // type
                node->children.push_back(children[1]); // id

                if (children.size() == 6) {
                    // Type ID ArrayDims = ArrayLiteral ;
                    node->children.push_back(children[2]); // dims
                    node->children.push_back(children[4]); // value
                } else if (children.size() == 5) {
                    // Type ID = Expr ; 
                    node->children.push_back(children[3]); // value
                } else if (children.size() == 7) {
                    // This might be a case where ArrayDims got parsed as separate tokens
                    // Check if children[2] is '[', children[3] is NUM, children[4] is ']'
                    if (children[2]->type == "[" && children[4]->type == "]" && 
                        (children[3]->type == "NUM" || children[3]->type == "Number")) {
                        // Create ArraySize node
                        Node* arraySize = new Node();
                        arraySize->type = "ArraySize";
                        arraySize->value = children[3]->value;
                        arraySize->lineNumber = children[3]->lineNumber;
                        node->children.push_back(arraySize);
                        node->children.push_back(children[6]); // ArrayLiteral after '='
                    } else {
                        // Fallback: treat as regular assignment
                        node->children.push_back(children[3]); // value
                    }
                } else {
                    // Fallback for other cases
                    if (children.size() > 3) {
                        node->children.push_back(children[children.size() - 2]); // value before ';'
                    }
                }

                newNode = node;
            }

            // ================= ARRAY DIMS =================
            else if (p.lhs == "ArrayDims") {
                if (children.size() == 4) {
                    // Multi-dimensional: ArrayDims [ NUM ]
                    Node* prev = children[0];

                    if (prev->type != "Dimensions") {
                        Node* dims = new Node();
                        dims->type = "Dimensions";
                        dims->lineNumber = prev->lineNumber;

                        Node* first = new Node();
                        first->type = "Number";
                        first->value = prev->value;
                        first->lineNumber = prev->lineNumber;
                        dims->children.push_back(first);

                        prev = dims;
                    }

                    Node* newDim = new Node();
                    newDim->type = "Number";
                    newDim->value = children[2]->value;
                    newDim->lineNumber = children[2]->lineNumber;

                    prev->children.push_back(newDim);
                    newNode = prev;
                } else if (children.size() == 3) {
                    // Single dimension with size: [ NUM ]
                    Node* node = new Node();
                    node->type = "Number";
                    node->value = children[1]->value;
                    node->lineNumber = children[1]->lineNumber;
                    newNode = node;
                } else if (children.size() == 2) {
                    // Empty brackets (inferred size): [ ]
                    Node* node = new Node();
                    node->type = "InferredSize";
                    node->value = "0";  // Placeholder, will be inferred from initializer
                    node->lineNumber = getLineFromChildren(children);
                    newNode = node;
                } else {
                    // Fallback for unexpected cases
                    Node* node = new Node();
                    node->type = "ArrayDims";
                    node->lineNumber = getLineFromChildren(children);
                    node->children = children;
                    newNode = node;
                }
            }

            // ================= ARRAY LITERAL =================
            else if (p.lhs == "ArrayLiteral") {
                Node* node = new Node();
                node->type = "ArrayInit";
                node->lineNumber = getLineFromChildren(children);

                node->children = children[1]->children;
                newNode = node;
            }

            else if (p.lhs == "Elements") {
                Node* node = new Node();
                node->type = "Elements";
                node->lineNumber = getLineFromChildren(children);

                if (children.size() == 3) {
                    node->children = children[0]->children;
                    node->children.push_back(children[2]);
                } else {
                    node->children.push_back(children[0]);
                }

                newNode = node;
            }

            else if (p.lhs == "Element") {
                newNode = children[0];
            }

            // ================= ARRAY ACCESS =================
            else if (p.lhs == "ArrayAccess") {
                Node* node = new Node();
                node->type = "ArrayAccess";
                node->lineNumber = getLineFromChildren(children);

                // Convert to children format for consistency with AST loader
                node->children.push_back(children[0]); // ID or nested ArrayAccess
                node->children.push_back(children[2]); // index expression

                newNode = node;
            }

            // ================= EXPRESSIONS =================
            else if (p.lhs == "Expr" || p.lhs == "Term") {
                if (children.size() == 3) {
                    Node* node = new Node();
                    node->type = children[1]->value.empty() ? children[1]->type : children[1]->value;
                    node->lineNumber = getLineFromChildren(children);
                    node->children.push_back(children[0]); // left operand
                    node->children.push_back(children[2]); // right operand
                    newNode = node;
                } else {
                    newNode = children[0];
                }
            }

            else if (p.lhs == "Factor") {
                if (children.size() == 3) {
                    newNode = children[1];
                } else if (children[0]->type == "endl" || children[0]->type == "endLine") {
                    Node* node = new Node();
                    node->type = "EndLine";
                    node->lineNumber = children[0]->lineNumber;
                    newNode = node;
                } else if (children[0]->type == "NUM") {
                    Node* node = new Node();
                    node->type = "Number";
                    node->value = children[0]->value;
                    node->lineNumber = children[0]->lineNumber;
                    newNode = node;
                } else {
                    newNode = children[0];
                }
            }

            else if (p.lhs == "Type") {
                Node* node = new Node();
                node->type = "DATATYPE";
                node->value = children[0]->value;
                newNode = node;
            }

            else {
                Node* node = new Node();
                node->type = p.lhs;
                node->children = children;
                newNode = node;
            }

            int currentState = stateStack.top();

            if (!GOTO_TABLE.count({currentState, p.lhs})) {
                cout << "\n========================================\n";
                cout << "  ❌ SYNTAX ERROR (GOTO)\n";
                cout << "========================================\n";
                cout << "Parser internal error: Missing GOTO entry\n";
                cout << "State: " << currentState << ", Symbol: " << p.lhs << "\n";
                cout << "This indicates a grammar or parsing table issue.\n";
                cout << "========================================\n";
                return nullptr;
            }

            int nextState = GOTO_TABLE[{currentState, p.lhs}];

            nodeStack.push(newNode);
            stateStack.push(nextState);
        }

        // ================= ACCEPT =================
        else if (action == "ACC") {
            cout << "Parsing Successful\n";
            return nodeStack.top();
        }
    }
}

bool isNumericString(const string& s) {
    if (s.empty()) return false;
    bool hasDigit = false;
    bool hasDot = false;
    for (char c : s) {
        if (isdigit(static_cast<unsigned char>(c))) {
            hasDigit = true;
            continue;
        }
        if (c == '.' && !hasDot) {
            hasDot = true;
            continue;
        }
        return false;
    }
    return hasDigit;
}

void printJSON(Node* node, int indent = 0, ostream* outStream = nullptr) {

    ostream& out = (outStream) ? *outStream : cout;

    if (!node) return;

    string space(indent, ' ');

    out << space << "{\n";

    out << space << "  \"type\": \"" << node->type << "\"";

    if (!node->value.empty()) {
        out << ",\n";
        if ((node->type == "Number") && isNumericString(node->value)) {
            out << space << "  \"value\": " << node->value;
        } else {
            out << space << "  \"value\": \"" << node->value << "\"";
        }
    }

    // Add line number to JSON output
    if (node->lineNumber > 0) {
        out << ",\n";
        out << space << "  \"line\": " << node->lineNumber;
    }

    if (!node->op.empty()) {
        out << ",\n";
        out << space << "  \"operator\": \"" << node->op << "\"";
    }

    if (!node->arrayName.empty()) {
        out << ",\n";
        out << space << "  \"array\": \"" << node->arrayName << "\"";
    }

    if (node->left) {
        out << ",\n";
        out << space << "  \"left\": \n";
        printJSON(node->left, indent + 4, outStream);
    }

    if (node->right) {
        out << ",\n";
        out << space << "  \"right\": \n";
        printJSON(node->right, indent + 4, outStream);
    }

    if (node->index) {
        out << ",\n";
        out << space << "  \"index\": \n";
        printJSON(node->index, indent + 4, outStream);
    }

    if (!node->children.empty()) {
        out << ",\n";
        out << space << "  \"children\": [\n";

        for (int i = 0; i < node->children.size(); i++) {
            printJSON(node->children[i], indent + 4, outStream);
            if (i != node->children.size() - 1)
                out << ",";
            out << "\n";
        }

        out << space << "  ]\n";
        out << space << "}";
    } else {
        out << "\n" << space << "}";
    }
}