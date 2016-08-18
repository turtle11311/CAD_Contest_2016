#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <iostream>
#include <list>
#include <string>
class Gate;
typedef std::list<Gate *> GateList;
enum GateType { NONE, NOT, NOR, NAND, WIRE, INPUT, OUTPUT };

#ifndef GATE_H
#define GATE_H
class Gate{
public:
    Gate(std::string name = "", GateType type = NONE);
	void eval();
	Gate* trueinput();
	std::string name;
	GateList fan_in;
	GateList fan_out;
	GateList::iterator fan_out_it;
	GateType type;
	short value;
	int arrival_time;
	bool hasTrav;
	Gate* true_fan_in;
};
#endif
