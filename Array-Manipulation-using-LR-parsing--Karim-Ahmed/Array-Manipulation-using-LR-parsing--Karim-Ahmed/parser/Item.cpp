#include "Item.h"
#include "First_Set.h"
#include "Grammar.h"

set<Item> closure(set<Item> I) {
    set<Item> result = I;
    bool changed = true;

    while (changed) {
        changed = false;

        vector<Item> items(result.begin(), result.end());

        for (auto &item : items) {
            string B = item.nextSymbol();

            if (B == "" || !nonTerminals.count(B)) continue;

            vector<string> beta;
            for (int i = item.dotPos + 1; i < item.rhs.size(); i++)
                beta.push_back(item.rhs[i]);

            beta.push_back(item.lookahead);

            set<string> lookaheads = firstOfSequence(beta);

            for (auto &prod : grammar) {
                if (prod.lhs != B) continue;

                for (auto &la : lookaheads) {
                    Item newItem;
                    newItem.lhs = B;
                    newItem.rhs = prod.rhs;
                    newItem.dotPos = 0;
                    newItem.lookahead = la;

                    if (result.insert(newItem).second)
                        changed = true;
                }
            }
        }
    }

    return result;
}

set<Item> GOTO(set<Item> I, string X) {
    set<Item> moved;

    for (auto &item : I) {
        if (item.nextSymbol() == X) {
            moved.insert(item.advanceDot());
        }
    }

    return closure(moved);
}
