#pragma once
#include "Gate.h"
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <climits>
#include <utility>
class Network;
class Path;
typedef std::map<std::string, Gate*> GateMap;
typedef std::pair<Network*, int> args_t;

template <typename Container>
void printContainer(Container &container) {
    int size = 0;
    std::cout << "( ";
    for (typename Container::const_iterator it = container.begin();
            it != container.end(); ++it) {
        std::cout << (*it)->name << (++size != container.size() ? ", " : "");
    }
    std::cout << " )";
}

void getTokens(std::list<char *> &tokens, char *src);
char *getExpression();

class Path : public GateList {
public:
    Path();
    bool isFind[2];
};

void output_format(args_t arg, Path &path);
void* findPatternTruePath(void *args);

class Network{
    friend std::ostream &operator<<(std::ostream &out, const Gate &gate);
    public:
    // Network data
    std::istream &inputFile;
    int timing, slack;
    Gate start;
    Gate end;
    char *module_exp, *inputs_exp, *outputs_exp, *wires_exp;
    std::string moduleName;
    GateMap gatePool;
    GateMap wirePool;
    GateList evalSequence;
    std::list<Path> paths;
    int pathCounter;
    // Network function
    Network(unsigned int timing = UINT_MAX, unsigned int slack = UINT_MAX,
            std::istream &in = std::cin);
    ~Network();
    Gate *findGateByName(const char *name);
    void gateWiring(Gate *input, Gate *output);
    void createGraph();
    void DFS();
    void topologySort();
    void addOne(std::vector<int>& pattern);
    void forTest();
    void force();
    void printAllPaths();
    void random2Shrink(int pid);
    void findTruePath(int pid);
    void test2PrintGateValue(int pid);
    bool subFindTruePath(int pid, GateType type, Gate* me, Gate* you);
    Gate* isReady(int pid, Gate* out);
    void evalNetwork(int pid);
    void randomInput(int pid);
    char* getExpression();
    void parallelFindTruePath();
};
