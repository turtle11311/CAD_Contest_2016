#pragma once
#include "config.hpp"
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
typedef std::vector<Gate*> GateVector;
typedef std::list<std::tuple<Gate*, int, short>> ModifyList;
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

class Path : public GateVector {
public:
    Path();
    Path(Path &path);
    GateList PISequence;
    CriticalList criticList;
    bool isFind[2];
};


class Network{
    friend std::ostream &operator<<(std::ostream &out, const Gate &gate);
    public:
    // Network data
    std::istream &inputFile;
    std::ostream &outputFile;
    int timing, slack;
    Gate start;
    Gate end;
    char *module_exp, *inputs_exp, *outputs_exp, *wires_exp;
    std::string moduleName;
    int truePathCounter;
    int pathCounter;
    unsigned int minimun;
    unsigned long long branchLimit;
    unsigned long long branchCounter[ThreadNumber];
    GateMap gatePool;
    GateMap wirePool;
    GateList evalSequence;
    std::vector<Path*> paths;
    // Network function
    Network(unsigned int timing = UINT_MAX, unsigned int slack = UINT_MAX,
            std::istream &in = std::cin, std::ostream &out = std::cout);
    ~Network();
    Gate *findGateByName(const char *name);
    char* getExpression();
    void gateWiring(Gate *input, Gate *output);
    void output_format(size_t pid, Path &path);
    void createGraph();
    void findAllPath();
    void resetAllfan_out_it();
    void resetAllValueAndTime(size_t pid);
    GateList findAssociatePI( Gate* );
    void topologySort();
    void exhaustiveMethod();
    bool checkInverseValue(size_t pid, Path &path);
    void random2Shrink(size_t pid);
    void checkAllPathNowIsTruePath(size_t pid);
    int isTruePath(size_t pid, Path &path);
    void genPISequence(Path &path);
    void genAllPISequence();
    int subIsTruePath(size_t pid, Gate* curGate, Gate* me, Gate* you);
    bool criticalFalse(size_t pid, CriticalList &criticList);
    void evalNetwork(size_t pid);
    void randomInput(size_t pid);
    void parallelRandomMethod();
    void parallelBranchAndBound();
    void parallelExhaustiveMethod();
    void ExhaustiveMethodThreading(size_t pid);
    void branchAndBoundThreading(size_t pid);
    void findPatternTruePath(size_t pid);
    bool nextPIPattern(size_t pid,
                       GateList::iterator first, GateList::iterator last,
                       int *overflow);
    void branchAndBoundOnePath(size_t pid, Path &path);
    int branchAndBound(size_t pid, Path &path, GateList::iterator pos);
    void evalFLTime();
    void forwardSimulation(size_t pid, Gate* gate, ModifyList &modifyList);
    void resetLastStatusWithModifyList(size_t pid, ModifyList &modifyList);
    void startFindTruePath();

    bool backwardIsConflict( Path& path , Gate* direction , Gate* cur );
    bool forwardIsConflict( Path& path , Gate* direction );
    void backwardImplication( Path& path, Gate* cur );
    void forwardImplication( Path& path, Gate* cur );
};
