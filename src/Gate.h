#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <iostream>
#include <list>
#include <string>
class Gate;
typedef std::list<Gate *> GateList;
enum GateType { NONE, NOT, NOR, NAND, WIRE, INPUT, OUTPUT };

class Gate {
public:
    Gate(std::string name = "", GateType type = NONE);
    void eval(int pid);
    Gate* trueinput(int pid);
    Gate* ctrlinput(int pid);
    short inline ctrlValue() { return type == NOR; }
    std::string name;
    GateList fan_in;
    GateList fan_out;
    GateList::iterator fan_out_it;
    GateType type;
    short value[4];
    int arrival_time[4];
    bool hasTrav;

    int first_in;
    int last_in;
};
