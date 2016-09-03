#include "config.hpp"
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
    net.startFindTruePath();
    return 0;
}
