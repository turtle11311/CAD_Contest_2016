#define _CRT_SECURE_NO_WARNINGS
#ifndef __NETWORK__
#define __NETWORK__
#include <cstring>
#include <iostream>
#include <list>
#include <string>
using std::list;
using std::string;
using std::getline;
using std::cin;

struct Gate;
struct Network;
struct Wire;
enum GateType { NOT, NOR, NAND, INPUT, OUTPUT };

struct Gate {
    char *name;
    GateType type;
    short value;
    list<Gate *> fan_in;
    list<Gate *> fan_out;
};

struct Wire {
    char *name;
    Gate *fan_in;
    list<Gate *> fan_out;
};

void getTokens(list<char *> &tokens, char *src) {
    char delims[] = " ,;()";
    tokens.clear();
    char *tok = strtok(src, delims);
    do {
        tokens.push_back(tok);
        tok = strtok(NULL, delims);
    } while (tok);
}

struct Network {
    Gate start;
    Gate end;
    char *module_exp, *inputs_exp, *outputs_exp, *wires_exp;
    list<Gate *> gatePool;
    list<Wire *> wirePool;
    Gate *accessByGateName(const char *name) {
        list<Gate *>::iterator it = gatePool.begin();
        for (; it != gatePool.end(); ++it) {
            if (!strcmp((*it)->name, name)) {
                break;
            }
        }
        return (it == gatePool.end()) ? new Gate : *it;
    }

    Wire *accessByWireName(const char *name) {
        list<Wire *>::iterator it = wirePool.begin();
        for (; it != wirePool.end(); ++it) {
            if (!strcmp((*it)->name, name)) {
                break;
            }
        }
        return (it == wirePool.end()) ? new Wire : *it;
    }
    void CreateGraph() {
        ////////////////////////////////////////////////////////////////////////
        // SET INPUT
        ////////////////////////////////////////////////////////////////////////
        list<char *> tokens;
        getTokens(tokens, inputs_exp);
        tokens.pop_front();
        for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            Gate *newGate = new Gate;
            newGate->name = *it;
            newGate->type = INPUT;
            newGate->fan_in.push_back(&start);
            start.fan_out.push_back(newGate);
            gatePool.push_back(newGate);
        }
        ////////////////////////////////////////////////////////////////////////
        // SET OUTPUT
        ////////////////////////////////////////////////////////////////////////
        getTokens(tokens, outputs_exp);
        tokens.pop_front();
        for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            Gate *newGate = new Gate;
            newGate->name = *it;
            newGate->type = OUTPUT;
            newGate->fan_out.push_back(&end);
            end.fan_in.push_back(newGate);
            gatePool.push_back(newGate);
        }
        ////////////////////////////////////////////////////////////////////////
        // SET WIRE
        ////////////////////////////////////////////////////////////////////////
        getTokens(tokens, wires_exp);
        tokens.pop_front();
        for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            Wire *newWire = new Wire;
            newWire->name = *it;
            wirePool.push_back(newWire);
        }
        ////////////////////////////////////////////////////////////////////////
        // SET GATE
        ////////////////////////////////////////////////////////////////////////
        string gate_declare;
        while (getline(cin, gate_declare), gate_declare != "endmodule") {
        }

        for (list<Wire *>::iterator it = wirePool.begin(); it != wirePool.end();
             ++it) {
            delete *it;
        }
    }
    ~Network() {
        for (list<Gate *>::iterator it = gatePool.begin(); it != gatePool.end();
             ++it) {
            delete *it;
        }
        delete[] module_exp;
        delete[] inputs_exp;
        delete[] outputs_exp;
        delete[] wires_exp;
    }
};
#endif
