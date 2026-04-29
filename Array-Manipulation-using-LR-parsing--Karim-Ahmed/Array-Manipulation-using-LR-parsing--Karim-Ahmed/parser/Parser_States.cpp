#include <set>
#include <map>
#include "Item.cpp"



vector<set<Item>> states;
map<pair<int,string>, int> transitions;

void buildStates() {
    set<Item> start;

    Item startItem;
    startItem.lhs = "S'";
    startItem.rhs = {"Program"};
    startItem.dotPos = 0;
    startItem.lookahead = "$";

    start.insert(startItem);

    states.push_back(closure(start));

    bool changed = true;

    while (changed) {
        changed = false;

        for (int i = 0; i < states.size(); i++) {
            set<string> symbols;

            for (auto &item : states[i]) {
                string sym = item.nextSymbol();
                if (sym != "")
                    symbols.insert(sym);
            }

            for (auto &X : symbols) {
                set<Item> newState = GOTO(states[i], X);

                if (newState.empty()) continue;

                int index = -1;

                for (int j = 0; j < states.size(); j++) {
                    if (states[j] == newState) {
                        index = j;
                        break;
                    }
                }

                if (index == -1) {
                    states.push_back(newState);
                    index = states.size() - 1;
                    changed = true;
                }

                transitions[{i, X}] = index;
            }
        }
    }
}

bool sameState(const set<Item>& a, const set<Item>& b) {
    return a == b;
}