#include "ir_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>

using namespace std;

OptIR loadIR(const string& filePath) {
    OptIR ir;
    ifstream file(filePath);

    if (!file.is_open()) {
        cerr << "[IR Loader] Cannot open file: " << filePath << "\n";
        return ir;
    }

    string line;
    int lineNum = 0;

    while (getline(file, line)) {
        lineNum++;

        // Skip blank lines
        if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos)
            continue;

        // ── DECL comment  (// DECL varName  type=...) ─────────────────────
        if (line.find("//") == 0) {
            if (line.find("DECL") != string::npos) {
                regex declRegex(R"(//\s*DECL\s+(\w+)\s+type=(\S+))");
                smatch m;
                if (regex_search(line, m, declRegex))
                    ir.emplace_back("DECL", m[2], "", m[1], lineNum);
            }
            continue;
        }

        // Strip trailing source annotation:  ; line:N
        {
            size_t pos = line.find(';');
            if (pos != string::npos) {
                line = line.substr(0, pos);
                while (!line.empty() && isspace((unsigned char)line.back()))
                    line.pop_back();
            }
        }
        if (line.empty()) continue;

        smatch m;

        // ── Control-flow ───────────────────────────────────────────────────

        // LABEL  e.g.  L1:
        {
            regex re(R"(^(L\d+):$)");
            if (regex_match(line, m, re)) {
                ir.emplace_back("LABEL", "", "", m[1], lineNum);
                continue;
            }
        }
        // GOTO L1
        {
            regex re(R"(^GOTO\s+(L\d+)$)");
            if (regex_match(line, m, re)) {
                ir.emplace_back("GOTO", "", "", m[1], lineNum);
                continue;
            }
        }
        // IF_FALSE t1 GOTO L2
        {
            regex re(R"(^IF_FALSE\s+(\w+)\s+GOTO\s+(L\d+)$)");
            if (regex_match(line, m, re)) {
                ir.emplace_back("IF_FALSE", m[1], "", m[2], lineNum);
                continue;
            }
        }

        // ── I/O ────────────────────────────────────────────────────────────

        // PRINT <value>
        {
            regex re(R"(^PRINT\s+(.+)$)");
            if (regex_match(line, m, re)) {
                ir.emplace_back("PRINT", m[1], "", "", lineNum);
                continue;
            }
        }
        if (line == "PRINTLN") {
            ir.emplace_back("PRINTLN", "", "", "", lineNum);
            continue;
        }
        // READ varName
        {
            regex re(R"(^READ\s+(\w+)$)");
            if (regex_match(line, m, re)) {
                ir.emplace_back("READ", "", "", m[1], lineNum);
                continue;
            }
        }

        // ── Array access ───────────────────────────────────────────────────

        // result = arr[offset]
        {
            regex re(R"((\w+)\s*=\s*(\w+)\[(\w+)\])");
            if (regex_search(line, m, re)) {
                ir.emplace_back("LOAD", m[2], m[3], m[1], lineNum);
                continue;
            }
        }
        // arr[offset] = value
        {
            regex re(R"((\w+)\[(\w+)\]\s*=\s*(\w+))");
            if (regex_search(line, m, re)) {
                ir.emplace_back("STORE", m[1], m[2], m[3], lineNum);
                continue;
            }
        }

        // ── Arithmetic / assignment ────────────────────────────────────────

        // result = arg1 op arg2  (arithmetic AND relational)
        {
            regex re(R"((\w+)\s*=\s*(\w+)\s*([+\-*/<>!]=?|==|!=)\s*(\w+))");
            if (regex_search(line, m, re)) {
                ir.emplace_back(string(m[3]), string(m[2]), string(m[4]), string(m[1]), lineNum);
                continue;
            }
        }
        // result = arg1  (simple assign, value may be quoted string)
        {
            regex re(R"((\w+)\s*=\s*(\S+))");
            if (regex_search(line, m, re)) {
                ir.emplace_back("ASSIGN", m[2], "", m[1], lineNum);
                continue;
            }
        }
    }

    file.close();
    return ir;
}

bool writeOptimizedIR(const OptIR& ir, const string& filePath) {
    ofstream file(filePath);

    if (!file.is_open()) {
        cerr << "[IR Writer] Cannot write to file: " << filePath << "\n";
        return false;
    }

    for (const OptQuad& q : ir) {
        if (!q.isAlive) continue;

        if (q.op == "DECL") {
            file << "// DECL " << q.result;
            if (!q.arg1.empty()) file << "  type=" << q.arg1;
            file << "\n";
        } else if (q.op == "ASSIGN") {
            file << q.result << " = " << q.arg1 << "\n";
        } else if (q.op == "LOAD") {
            file << q.result << " = " << q.arg1 << "[" << q.arg2 << "]\n";
        } else if (q.op == "STORE") {
            file << q.arg1 << "[" << q.arg2 << "] = " << q.result << "\n";
        } else if (q.op == "LABEL") {
            file << q.result << ":\n";
        } else if (q.op == "GOTO") {
            file << "GOTO " << q.result << "\n";
        } else if (q.op == "IF_FALSE") {
            file << "IF_FALSE " << q.arg1 << " GOTO " << q.result << "\n";
        } else if (q.op == "PRINT") {
            file << "PRINT " << q.arg1 << "\n";
        } else if (q.op == "PRINTLN") {
            file << "PRINTLN\n";
        } else if (q.op == "READ") {
            file << "READ " << q.result << "\n";
        } else if (q.arg2.empty()) {
            file << q.result << " = " << q.op << " " << q.arg1 << "\n";
        } else {
            file << q.result << " = " << q.arg1 << " " << q.op << " " << q.arg2 << "\n";
        }
    }

    file.close();
    return true;
}
