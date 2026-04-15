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

        // SHIFT
        if (action[0] == 'S') {
            int nextState = stoi(action.substr(1));

            Node* node = new Node();
            node->type = token;
            node->value = value; // temporary (better: actual lexeme)

            nodeStack.push(node);
            stateStack.push(nextState);

            index++;
        }

        // REDUCE
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

            // Program
            if (p.lhs == "Program") {
                newNode = children[0];
            }

            // StmtList → flatten
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

            // Stmt
            else if (p.lhs == "Stmt") {
                newNode = children[0];
            }

            // Declaration
            else if (p.lhs == "DeclStmt") {
                Node* node = new Node();
                node->type = "Declaration";

                node->children.push_back(children[0]); // Type
                node->children.push_back(children[1]); // ID

                newNode = node;
            }

            // Assignment
            else if (p.lhs == "AssignStmt") {
                Node* node = new Node();
                node->type = "Assignment";

                node->children.push_back(children[0]); // ID / ArrayAccess
                node->children.push_back(children[2]); // Expr

                newNode = node;
            }

            // DeclAssign
            else if (p.lhs == "DeclAssignStmt") {
                Node* node = new Node();
                node->type = "DeclAssign";

                node->children.push_back(children[0]); // Type
                node->children.push_back(children[1]); // ID

                if (children.size() == 6) {
                    node->children.push_back(children[2]); // ArrayDims
                    node->children.push_back(children[4]); // ArrayLiteral
                } else {
                    node->children.push_back(children[3]); // Expr
                }

                newNode = node;
            }

            // ArrayDims → clean dimensions
            else if (p.lhs == "ArrayDims") {
                Node* node = new Node();
                node->type = "Dimensions";

                if (children.size() == 4) {
                    node->children = children[0]->children;
                    node->children.push_back(children[2]); // NUM
                } else {
                    node->children.push_back(children[1]); // NUM
                }

                newNode = node;
            }

            // ArrayLiteral → remove { }
            else if (p.lhs == "ArrayLiteral") {
                newNode = children[1];
            }

            // RowList → flatten into Array
            else if (p.lhs == "RowList") {
                Node* node = new Node();
                node->type = "Array";

                if (children.size() == 3) {
                    node->children = children[0]->children;
                    node->children.push_back(children[2]);
                } else {
                    node->children.push_back(children[0]);
                }

                newNode = node;
            }

            // Row
            else if (p.lhs == "Row") {
                newNode = (children.size() == 1) ? children[0] : children[1];
            }

            // ExprList → ArrayRow
            else if (p.lhs == "ExprList") {
                Node* node = new Node();
                node->type = "ArrayRow";

                if (children.size() == 3) {
                    node->children = children[0]->children;
                    node->children.push_back(children[2]);
                } else {
                    node->children.push_back(children[0]);
                }

                newNode = node;
            }

            // ArrayAccess
            else if (p.lhs == "ArrayAccess") {
                Node* node = new Node();
                node->type = "ArrayAccess";

                node->children.push_back(children[0]);
                node->children.push_back(children[2]);

                newNode = node;
            }

            // Remove useless layers
            else if (p.lhs == "Expr" || p.lhs == "Term" || p.lhs == "Factor") {
                newNode = children[0];
            }

            else if (p.lhs == "Type") {
                newNode = children[0];
            }

            // Default
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

        // ACCEPT
        else if (action == "ACC") {
            cout << "Parsing Successful\n";
            return nodeStack.top();
        }
    }
}

void printJSON(Node* node, int indent = 0, ostream* outStream = nullptr) {

    ostream& out = (outStream) ? *outStream : cout;

    if (!node) return;

    string space(indent, ' ');

    out << space << "{\n";

    // type
    out << space << "  \"type\": \"" << node->type << "\"";

    // 🔥 print value if exists
    if (!node->value.empty()) {
        out << ",\n";
        out << space << "  \"value\": \"" << node->value << "\"";
    }

    // children
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
    } 
    else {
        out << "\n" << space << "}";
    }
}