#define _CRT_SECURE_NO_WARNINGS
#include "Network.h"
#include <cstring>
#include <iostream>
#include <list>
#include <string>
using namespace std;

Network net;

int main(int argc, char const *argv[]) {
    string line;
    list<char *> tokens;
    // clear heading comment
    do {
        getline(cin, line);
    } while (line.substr(0, 2) == "//");
    /*========================================================================*/
    net.module_exp = getExpression();
    net.inputs_exp = getExpression();
    net.outputs_exp = getExpression();
    net.wires_exp = getExpression();
    net.createGraph();

    return 0;
}
