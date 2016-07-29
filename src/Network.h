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
using std::cout;
using std::endl;

struct Gate;
struct Network;
typedef Gate Wire;
enum GateType { NONE, NOT, NOR, NAND, WIRE, INPUT, OUTPUT };

template <typename Container>
void printContainer(Container &container) {
    cout << "( ";
    for (typename Container::const_iterator it = container.begin();
         it != container.end(); ++it) {
        cout << (*it)->name << ", ";
    }

    cout << "\b\b )";
}

struct Gate {
    string name;
    GateType type;
    short value;
    list<Gate *> fan_in;
    list<Gate *> fan_out;
    list<Gate *>::iterator fan_out_it;
    Gate() : type(NONE), value(-1) {}
};

std::ostream &operator<<(std::ostream &out, const Gate &gate) {
    out << "Gate name: " << gate.name << endl;
    out << "Gate type: "
        << ((gate.type == NOT) ? "NOT" : (gate.type == NAND) ? "NAND" : "NOR")
        << endl;
    out << "Gate fan in";
    printContainer(gate.fan_in);
    out << endl;
    out << "Gate fan out";
    printContainer(gate.fan_out);
    return out;
}

char *getExpression() {
    string line;
    string expression;
    do {
        getline(cin, line);
        expression += line;
    } while (expression[expression.size() - 1] != ';' && !cin.eof());
    return strdup(expression.c_str());
}

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
    list<list<Gate *> > paths;
    Network() {
        start.name = "start";
        end.name = "end";
    }

    Gate *accessByGateName(const char *name) {
        list<Gate *>::iterator it = gatePool.begin();
        for (; it != gatePool.end(); ++it) {
            if (!strcmp((*it)->name.c_str(), name)) {
                break;
            }
        }
        return (it == gatePool.end()) ? new Gate : *it;
    }

    Gate *findGateByName(const char *name) {
        for (list<Gate *>::iterator it = gatePool.begin(); it != gatePool.end();
             ++it) {
            if (!strcmp(name, (*it)->name.c_str())) {
                return *it;
            }
        }
        for (list<Gate *>::iterator it = wirePool.begin(); it != wirePool.end();
             ++it) {
            if (!strcmp(name, (*it)->name.c_str())) {
                return *it;
            }
        }
        return NULL;
    }

    void gateWiring(Gate *input, Gate *output) {
        output->fan_in.push_back(input);
        input->fan_out.push_back(output);
    }

    void createGraph() {
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
        start.fan_out_it = start.fan_out.begin();
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
            newGate->fan_out_it = newGate->fan_out.begin();
        }
        end.fan_out_it = end.fan_out.begin();
        ////////////////////////////////////////////////////////////////////////
        // SET WIRE
        ////////////////////////////////////////////////////////////////////////
        getTokens(tokens, wires_exp);
        tokens.pop_front();
        for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            Wire *newWire = new Wire;
            newWire->name = *it;
            newWire->type = WIRE;
            wirePool.push_back(newWire);
        }
        ////////////////////////////////////////////////////////////////////////
        // SET GATE
        ////////////////////////////////////////////////////////////////////////
        char *gate_declare;
        char *g1, *g2, *g3;
        Gate *nowGate;
        while (gate_declare = getExpression(),
               strcmp("endmodule", gate_declare)) {
            getTokens(tokens, gate_declare);
            list<char *>::iterator lp = tokens.begin();
            nowGate = new Gate;
            gatePool.push_back(nowGate);
            std::advance(lp, 1);
            nowGate->name = string(*lp);
            std::advance(lp, 2);
            g1 = *lp;
            std::advance(lp, 2);
            g2 = *lp;
            Gate *input1, *input2, *output;
            if ((input1 = findGateByName(g1))) {
                gateWiring(input1, nowGate);
            } else {
                input1 = new Gate;
                input1->name = g1;
                gateWiring(input1, nowGate);
                wirePool.push_back(input1);
            }
            input1->fan_out_it = input1->fan_out.begin();

            if (!strcmp("NOT1", tokens.front())) {
                nowGate->type = NOT;
                if ((input2 = findGateByName(g2))) {
                    gateWiring(nowGate, input2);
                } else {
                    input2 = new Gate;
                    input2->name = g2;
                    gateWiring(nowGate, input2);
                    wirePool.push_back(input2);
                }
                nowGate->fan_out_it = nowGate->fan_out.begin();
            } else {
                nowGate->type = (!strcmp("NOR2", tokens.front())) ? NOR : NAND;
                if ((input2 = findGateByName(g2))) {
                    gateWiring(input2, nowGate);
                } else {
                    input2 = new Gate;
                    input2->name = g2;
                    gateWiring(input2, nowGate);
                    wirePool.push_back(input2);
                }
                input2->fan_out_it = input2->fan_out.begin();

                std::advance(lp, 2);
                g3 = *lp;
                if ((output = findGateByName(g3))) {
                    gateWiring(nowGate, output);
                } else {
                    output = new Gate;
                    output->name = g3;
                    gateWiring(nowGate, output);
                    wirePool.push_back(output);
                }
                nowGate->fan_out_it = nowGate->fan_out.begin();
            }
        }

        for (list<Gate *>::iterator wire_it = wirePool.begin();
             wire_it != wirePool.end(); ++wire_it) {
            (*wire_it)->fan_in.front()->fan_out = (*wire_it)->fan_out;
        }
        for (list<Wire *>::iterator it = wirePool.begin(); it != wirePool.end();
             ++it) {
            delete *it;
        }
        delete[] module_exp;
        delete[] outputs_exp;
        delete[] wires_exp;
    }

    void Dfs() {
        list<Gate *> path;
        path.push_back(&start);
        while (path.size()) {
            if (path.back()->fan_out_it == path.back()->fan_out.end()) {
                if (path.back()->name == "end") {
                    paths.push_back(path);
                    printContainer(path);
                    cout << endl << endl;
                }
                path.back()->fan_out_it = path.back()->fan_out.begin();
                path.pop_back();
            } else {
                path.push_back(*(path.back()->fan_out_it++));
            }
        }
    }

    ~Network() {
        for (list<Gate *>::iterator it = gatePool.begin(); it != gatePool.end();
             ++it) {
            delete *it;
        }
    }
};
#endif
