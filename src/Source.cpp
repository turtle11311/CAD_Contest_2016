#define _CRT_SECURE_NO_WARNINGS
#include "Network.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <list>
#include <string>
using namespace std;

int main(int argc, char const *argv[]) {
    int timing, slack;
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << "Timing Slack filename" << endl;
        exit(EXIT_FAILURE);
    }
    std::ifstream file(argv[3]);
    if(!file) {
        cerr << "File " << argv[3] << " OPEN FAIL!" << endl;
        exit(EXIT_FAILURE);
    }
    timing = atoi(argv[1]);
    slack = atoi(argv[2]);
    Network net(timing, slack, file);
    net.createGraph();
    net.DFS();
    net.topologySort();
    net.parallelFindTruePath();
    return 0;
}
