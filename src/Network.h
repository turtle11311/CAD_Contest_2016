#pragma once
#include "Gate.h"
#include <map>
#include <list>
#include <vector>
#include <set>
#include <iostream>
#include <climits>
#include <utility>
#include <tuple>
class Network;
class Path;
typedef std::map<std::string, Gate*> GateMap;
typedef std::set<Gate*> GateSet;
typedef std::pair<Network*, int> args_t;
typedef std::list<std::tuple<Gate*, int, int>> ModifyList;
typedef std::list<std::pair<Gate*, short>> CriticalList;

template <typename Container>
void printContainer(const Container &container) {
    int size = 0;
    std::cout << "( ";
    for (typename Container::const_iterator it = container.begin();
            it != container.end(); ++it) {
        std::cout << (*it)->name << (++size != container.size() ? ", " : "");
    }
    std::cout << " )";
}

template <typename Container>
void getTokens(Container &tokens, char *src) {
    char delims[] = " ,.;()";
    tokens.clear();
    char *tok = strtok(src, delims);
    do {
        tokens.push_back(tok);
        tok = strtok(NULL, delims);
    } while (tok);
}

class Path : public GateList {
public:
    Path();
    GateList PISequence;
    CriticalList criticList;
    ModifyList modifyList[4];
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
    int pathCounter;
    unsigned int minimun;
    GateMap gatePool;
    GateMap wirePool;
    GateList evalSequence;
    std::vector<Path*> paths;
    std::map<Gate*,GateList> IOMap;
    // Network function
    Network(unsigned int timing = UINT_MAX, unsigned int slack = UINT_MAX,
            std::istream &in = std::cin);
    ~Network();
    Gate *findGateByName(const char *name);
    void gateWiring(Gate *input, Gate *output);
    void createGraph();
    void findAllPath();
    void resetAllfan_out_it();
    GateSet findAssociatePI( Gate* );
    void topologySort();
    void addOne(std::vector<int>& pattern);
    void force();
    void printAllPaths();
    void printIOMap();
    void random2Shrink(int pid);
    void findAllTruePath(int pid);
    int isTruePath(int pid, Path &path);
    void test2PrintGateValue(int pid);
    void genPISequence(Path &path);
    void genAllPISequence();
    int subFindTruePath(int pid, Gate* curGate, Gate* me, Gate* you);
    Gate* isReady(int pid, Gate* out);
    int branchAndBound(int pid, Path &path, GateList::iterator pos);
    bool criticalFalse(int pid, CriticalList &criticList);
    void evalNetwork(int pid);
    void randomInput(int pid);
    char* getExpression();
    void parallelFindTruePath();
    void evalFLTime();
    void forwardSimulation(int pid, Gate* gate, ModifyList &modifyList);
    void clearValueWithModifyList(int pid, ModifyList &modifyList);
};
