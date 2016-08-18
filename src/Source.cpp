#define _CRT_SECURE_NO_WARNINGS
#include "Network.h"
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <list>
#include <string>
using namespace std;


int main(int argc, char const *argv[]) {
    Network net;
    string line;
    list<char *> tokens;
    int timing, slack;
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << "Timing Slack" << endl;
        exit(EXIT_FAILURE);
    }
    timing = atoi(argv[1]);
    slack = atoi(argv[2]);
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
    net.Dfs(timing, slack);
    net.topologySort();
    net.forTest();
    return 0;
}
