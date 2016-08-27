#define _CRT_SECURE_NO_WARNINGS
#include "Network.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <list>
#include <string>
#include <cstdio>
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
    net.findAllPath();
    net.topologySort();
    net.evalFLTime();
    net.genAllPISequence();
    for (Path* path : net.paths) {
        net.branchAndBound(0, *path, path->PISequence.begin());
    }
    // auto path = *net.paths.front();
    // net.branchAndBound(0, path, path.PISequence.begin());
    // net.parallelFindTruePath();
    // net.force();
    return 0;
}
