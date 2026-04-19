#pragma once

#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <algorithm>
#include "Parsing_Table.cpp"

using namespace std;

struct Node {
    string type;
    string value;
    string op;
    string arrayName;
    Node* left = nullptr;
    Node* right = nullptr;
    Node* index = nullptr;
    vector<Node*> children;
};

Node* parse(vector<pair<string,string>> input) {

    stack<int> stateStack;
    stack<Node*> nodeStack;

    stateStack.push(0);
    int index = 0;

    while (true) {

        int state = stateStack.top();
        string token = input[index].first;
        string value = input[index].second;

        if (!ACTION.count({state, token})) {

            cout << "\n--- SYNTAX ERROR ---\n";
            cout << "Unexpected token: " << token
                 << " (value: " << value << ")\n";
            cout << "At input index: " << index << "\n";
            cout << "Expected one of: ";

            for (auto &entry : ACTION) {
                if (entry.first.first == state) {
                    cout << entry.first.second << " ";
                }
            }

            cout << "\n--------------------\n";
            return nullptr;
        }

        string action = ACTION[{state, token}];

        // ================= SHIFT =================
        if (action[0] == 'S') {
            int nextState = stoi(action.substr(1));

            Node* node = new Node();
            node->type = token;
            node->value = value;

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
                newNode = children[0];
            }

            else if (p.lhs == "StmtList") {
                Node* node = new Node();
                node->type = "Program";

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
                node->children = {children[0], children[1]};
                newNode = node;
            }

            // ================= ASSIGN =================
            else if (p.lhs == "AssignStmt") {
                Node* node = new Node();
                node->type = "Assignment";
                node->children = {children[0], children[2]};
                newNode = node;
            }

            // ================= DECL + ASSIGN =================
            else if (p.lhs == "DeclAssignStmt") {
                Node* node = new Node();
                node->type = "DeclAssign";

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
                    Node* prev = children[0];

                    if (prev->type != "Dimensions") {
                        Node* dims = new Node();
                        dims->type = "Dimensions";

                        Node* first = new Node();
                        first->type = "Number";
                        first->value = prev->value;
                        dims->children.push_back(first);

                        prev = dims;
                    }

                    Node* newDim = new Node();
                    newDim->type = "Number";
                    newDim->value = children[2]->value;

                    prev->children.push_back(newDim);
                    newNode = prev;
                } else {
                    Node* node = new Node();
                    node->type = "Number";
                    node->value = children[1]->value;
                    newNode = node;
                }
            }

            // ================= ARRAY LITERAL =================
            else if (p.lhs == "ArrayLiteral") {
                Node* node = new Node();
                node->type = "ArrayInit";

                node->children = children[1]->children;
                newNode = node;
            }

            else if (p.lhs == "Elements") {
                Node* node = new Node();
                node->type = "Elements";

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

                // Convert to children format for consistency with AST loader
                node->children.push_back(children[0]); // ID or nested ArrayAccess
                node->children.push_back(children[2]); // index expression

                newNode = node;
            }

            // ================= EXPRESSIONS =================
            else if (p.lhs == "Expr" || p.lhs == "Term") {
                if (children.size() == 3) {
                    Node* node = new Node();
                    node->type = "BinaryOp";
                    node->op = children[1]->value.empty() ? children[1]->type : children[1]->value;
                    node->left = children[0];
                    node->right = children[2];
                    newNode = node;
                } else {
                    newNode = children[0];
                }
            }

            else if (p.lhs == "Factor") {
                if (children.size() == 3) {
                    newNode = children[1];
                } else if (children[0]->type == "NUM") {
                    Node* node = new Node();
                    node->type = "Number";
                    node->value = children[0]->value;
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
                cout << "Goto Error\n";
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