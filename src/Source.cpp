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
    std::ios_base::sync_with_stdio(false);
    int timing, slack;
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " Timing Slack [FileName] [OutputFileName]" << endl;
        exit(EXIT_FAILURE);
    }
    std::ifstream file(argv[3]);
    std::ofstream out(argv[4], ios::ate);
    if(!file) {
        cerr << "File " << argv[3] << " OPEN FAIL!" << endl;
        exit(EXIT_FAILURE);
    }
    timing = atoi(argv[1]);
    slack = atoi(argv[2]);
    Network net(timing, slack, file, out);
    net.createGraph();
    net.startFindTruePath();
    file.close();
    out.close();
    return 0;
}
