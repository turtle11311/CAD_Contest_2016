#pragma once
#include "Gate.h"
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <climits>
typedef std::map<std::string, Gate*> GateMap;

class Network;
class Path;
template <typename Container>
void printContainer(Container &container) {
    std::cout << "( ";
    for (typename Container::const_iterator it = container.begin();
            it != container.end(); ++it) {
        std::cout << (*it)->name << ", ";
    }
    std::cout << "\b\b )";
}

void getTokens(std::list<char *> &tokens, char *src);
char *getExpression();

class Path : public GateList {
public:
    Path();
    bool rising, falling;
};

class Network{
    friend std::ostream &operator<<(std::ostream &out, const Gate &gate);
    public:
    // Network data
    std::istream &inputFile;
    int timing, slack;
    Gate start;
    Gate end;
    char *module_exp, *inputs_exp, *outputs_exp, *wires_exp;
    GateMap gatePool;
    GateMap wirePool;
    GateList evalSequence;
    std::list<Path> paths;
    // Network function
    Network(unsigned int timing = UINT_MAX, unsigned int slack = UINT_MAX,
            std::istream &in = std::cin);
    ~Network();
    Gate *findGateByName(const char *name);
    void gateWiring(Gate *input, Gate *output);
    void createGraph();
    void DFS();
    void topologySort();
    void random2Shrink();
    void addOne(std::vector<int>& pattern);
    void forTest();
    void force();
    void findTruePath();
    void test2PrintGateValue();
    bool subFindTruePath(GateType type, Gate* me, Gate* you);
    Gate* isReady(Gate* out);
    void evalNetwork();
    void randomInput();
    void clearNetworkValue();
    char* getExpression();
};
