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
    cout << "Creat Complete" << endl;
    net.findAllPath();
    cout << "All Path Complete" << endl;
    net.topologySort();
    cout << "TopologySort Complete" << endl;
    // net.evalFLTime();
    // cout << "FLtime Complete" << endl;
    // net.genAllPISequence();
    // cout << "PISequnece Complete" << endl;
    // for (Path* path : net.paths) {
    //     ModifyList modifyList;
    //     // cout << path->PISequence.front()->name << " ";
    //     printContainer(path->PISequence), cout << endl;
    //     // cout << "00000000" << endl;
    //     // for (auto Gate : net.gatePool) {
    //     //     if (Gate.second->value[0] != -1 || Gate.second->arrival_time[0] != -1)
    //     //         cout << Gate.second->name << " ";
    //     // }
    //     // cout << endl;
    //     path->PISequence.front()->arrival_time[0] = 0;
    //     path->PISequence.front()->value[0] = 0;
    //     for (Gate *nowPIFanOut : path->PISequence.front()->fan_out) {
    //         net.forwardSimulation(0, nowPIFanOut, modifyList);
    //     }
    //     net.branchAndBound(0, *path, ++path->PISequence.begin());
    //     net.clearValueWithModifyList(0, modifyList);
    //     modifyList.clear();
    //     // cout << "11111111" << endl;
    //     // for (auto Gate : net.gatePool) {
    //     //     if (Gate.second->value[0] != -1 || Gate.second->arrival_time[0] != -1)
    //     //         cout << Gate.second->name << " ";
    //     // }
    //     // cout << endl;
    //     path->PISequence.front()->value[0] = 1;
    //     for (Gate *nowPIFanOut : path->PISequence.front()->fan_out) {
    //         net.forwardSimulation(0, nowPIFanOut, modifyList);
    //     }
    //     net.branchAndBound(0, *path, ++path->PISequence.begin());
    //     net.clearValueWithModifyList(0, modifyList);
    //     path->PISequence.front()->value[0] = -1;
    //     path->PISequence.front()->arrival_time[0] = -1;
    // }
    net.parallelFindTruePath();
    // net.force();
    return 0;
}
