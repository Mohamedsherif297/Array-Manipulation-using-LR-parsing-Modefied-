#ifndef ITEM_H
#define ITEM_H

#include <iostream>
#include <vector>
#include <string>
#include <set>
using namespace std;

struct Item {
    string lhs;
    vector<string> rhs;
    int dotPos;
    string lookahead;

    bool operator<(const Item& other) const {
        if (lhs != other.lhs) return lhs < other.lhs;
        if (rhs != other.rhs) return rhs < other.rhs;
        if (dotPos != other.dotPos) return dotPos < other.dotPos;
        return lookahead < other.lookahead;
    }

    bool operator==(const Item& other) const {
    return lhs == other.lhs &&
           rhs == other.rhs &&
           dotPos == other.dotPos &&
           lookahead == other.lookahead;
}

string nextSymbol() const {
    if (dotPos < rhs.size())
        return rhs[dotPos];
    return "";
    }

    bool isComplete() const {
    return dotPos >= rhs.size();
    }

    Item advanceDot() const {
    Item next = *this;
    next.dotPos++;
    return next;
    }

    void print() const {
    cout << lhs << " → ";
    for (int i = 0; i < rhs.size(); i++) {
        if (i == dotPos) cout << "• ";
        cout << rhs[i] << " ";
    }
    if (dotPos == rhs.size()) cout << "• ";
    cout << ", " << lookahead << endl;
 }
};

set<Item> closure(set<Item> I);
set<Item> GOTO(set<Item> I, string X);

#endif