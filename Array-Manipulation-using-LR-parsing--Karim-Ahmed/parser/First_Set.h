#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include "Grammar.h"

using namespace std;

inline map<string, set<string>> FIRST;

inline set<string> firstOfSequence(const vector<string>& seq) {
    set<string> result;
    bool epsilon = true;

    for (auto &X : seq) {
        epsilon = false;

        if (terminals.count(X)) {
            result.insert(X);
            return result;
        }

        for (auto &f : FIRST[X]) {
            if (f != "ε") result.insert(f);
        }

        if (!FIRST[X].count("ε")) return result;
        epsilon = true;
    }

    if (epsilon) result.insert("ε");
    return result;
}

inline void computeFIRST() {
    bool changed = true;

    while (changed) {
        changed = false;

        for (auto &prod : grammar) {
            string A = prod.lhs;
            auto &rhs = prod.rhs;

            if (rhs.empty()) {
                if (FIRST[A].insert("ε").second) changed = true;
                continue;
            }

            bool epsilon = true;

            for (auto &X : rhs) {
                epsilon = false;

                if (terminals.count(X)) {
                    if (FIRST[A].insert(X).second) changed = true;
                    break;
                }

                for (auto &f : FIRST[X]) {
                    if (f != "ε") {
                        if (FIRST[A].insert(f).second) changed = true;
                    }
                }

                if (!FIRST[X].count("ε")) break;
                epsilon = true;
            }

            if (epsilon) {
                if (FIRST[A].insert("ε").second) changed = true;
            }
        }
    }
}


inline void printFIRST() {
    for (auto &p : FIRST) {
        cout << "FIRST(" << p.first << ") = { ";
        for (auto &x : p.second) cout << x << " ";
        cout << "}\n";
    }
}



/*
int main() {
    computeFIRST();

    printFIRST();
    cout << "FIRST set computed successfully.\n";

    return 0;
}
    */