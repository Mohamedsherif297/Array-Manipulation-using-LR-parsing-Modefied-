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
        
        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos) {
            continue;
        }
        
        // Handle comments (DECL statements)
        if (line.find("//") == 0) {
            // Extract DECL information
            if (line.find("DECL") != string::npos) {
                regex declRegex(R"(//\s*DECL\s+(\w+)\s+type=(\w+))");
                smatch match;
                if (regex_search(line, match, declRegex)) {
                    string varName = match[1];
                    string typeName = match[2];
                    ir.emplace_back("DECL", typeName, "", varName, lineNum);
                }
            }
            continue;
        }
        
        // Parse different TAC formats
        
        // Format: result = arg1 op arg2  (e.g., t1 = a + b)
        regex binaryRegex(R"((\w+)\s*=\s*(\w+)\s*([+\-*/])\s*(\w+))");
        smatch match;
        
        if (regex_search(line, match, binaryRegex)) {
            string result = match[1];
            string arg1 = match[2];
            string op = match[3];
            string arg2 = match[4];
            ir.emplace_back(op, arg1, arg2, result, lineNum);
            continue;
        }
        
        // Format: result = arg1  (e.g., x = 5)
        regex assignRegex(R"((\w+)\s*=\s*(\w+))");
        if (regex_search(line, match, assignRegex)) {
            string result = match[1];
            string arg1 = match[2];
            ir.emplace_back("ASSIGN", arg1, "", result, lineNum);
            continue;
        }
        
        // Format: result = arr[offset]  (e.g., t1 = x[t2])
        regex loadRegex(R"((\w+)\s*=\s*(\w+)\[(\w+)\])");
        if (regex_search(line, match, loadRegex)) {
            string result = match[1];
            string arr = match[2];
            string offset = match[3];
            ir.emplace_back("LOAD", arr, offset, result, lineNum);
            continue;
        }
        
        // Format: arr[offset] = value  (e.g., x[t1] = 5)
        regex storeRegex(R"((\w+)\[(\w+)\]\s*=\s*(\w+))");
        if (regex_search(line, match, storeRegex)) {
            string arr = match[1];
            string offset = match[2];
            string value = match[3];
            ir.emplace_back("STORE", arr, offset, value, lineNum);
            continue;
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
        // Skip dead code
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
        } else if (q.arg2.empty()) {
            // Unary operation
            file << q.result << " = " << q.op << " " << q.arg1 << "\n";
        } else {
            // Binary operation
            file << q.result << " = " << q.arg1 << " " << q.op << " " << q.arg2 << "\n";
        }
    }
    
    file.close();
    return true;
}
