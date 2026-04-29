#include <iostream>
#include <map>
#include <string>
#include <set>
#include <vector>
#include "Item.h"
#include "Parser_States.cpp"
#include "Grammar.h"

using namespace std;

map<pair<int,string>, string> ACTION;
map<pair<int,string>, int> GOTO_TABLE;

string productionToString(const Production& p) {
    string s = p.lhs + "→";
    for (auto &x : p.rhs) s += x + " ";
    return s;
}

int getProductionIndex(const string& lhs, const vector<string>& rhs) {
    for (int i = 0; i < grammar.size(); i++) {
        if (grammar[i].lhs == lhs && grammar[i].rhs == rhs)
            return i;
    }
    return -1;
}

void buildParsingTable() {
    for (int i = 0; i < states.size(); i++) {

        for (auto &t : transitions) {
            int state = t.first.first;
            string symbol = t.first.second;
            int nextState = t.second;

            if (state != i) continue;

            if (terminals.count(symbol)) {
                string action = "S" + to_string(nextState);

                if (ACTION.count({i, symbol})) {
                    cout << "Shift/Conflict at state " << i 
                         << " symbol " << symbol 
                         << " between " << ACTION[{i, symbol}] 
                         << " and " << action << endl;
                    exit(1);
                }

                ACTION[{i, symbol}] = action;
            }
            else if (nonTerminals.count(symbol)) {
                GOTO_TABLE[{i, symbol}] = nextState;
            }
        }

        for (auto &item : states[i]) {

            if (item.isComplete()) {

                if (item.lhs == "S'" && item.lookahead == "$") {
                    ACTION[{i, "$"}] = "ACC";
                } 
                else {
                    int prodIndex = getProductionIndex(item.lhs, item.rhs);
                    if (prodIndex == -1) continue;

                    string action = "R" + to_string(prodIndex);

                    if (ACTION.count({i, item.lookahead})) {
                        cout << "Reduce/Conflict at state " << i 
                             << " symbol " << item.lookahead 
                             << " between " << ACTION[{i, item.lookahead}] 
                             << " and " << action << endl;
                        exit(1);
                    }

                    ACTION[{i, item.lookahead}] = action;
                }
            }
        }
    }
}

void printParsingTable() {
    cout << "\nACTION TABLE:\n";
    for (auto &a : ACTION) {
        cout << "ACTION[" << a.first.first << ", " 
             << a.first.second << "] = " 
             << a.second << endl;
    }

    cout << "\nGOTO TABLE:\n";
    for (auto &g : GOTO_TABLE) {
        cout << "GOTO[" << g.first.first << ", " 
             << g.first.second << "] = " 
             << g.second << endl;
    }
}