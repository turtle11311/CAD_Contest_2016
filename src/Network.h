#define _CRT_SECURE_NO_WARNINGS
#ifndef __NETWORK__
#define __NETWORK__
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <ctime>
#include <cstdlib>
#include <vector>
using std::map;
using std::list;
using std::string;
using std::getline;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
struct Gate;
struct Network;
typedef map<string, Gate *> GateMap;
typedef list<Gate *> GateList;
enum GateType { NONE, NOT, NOR, NAND, WIRE, INPUT, OUTPUT };

template <typename Container>
void printContainer(Container &container) {
    cout << "( ";
    for (typename Container::const_iterator it = container.begin();
         it != container.end(); ++it) {
        cout << (*it)->name << ", ";
    }

    cout << "\b\b )";
}

struct Gate {
    string name;
    GateType type;
    short value;
    int arrival_time;
    GateList fan_in;
    GateList fan_out;
    GateList::iterator fan_out_it;
    bool hasTrav;
    Gate() : type(NONE), value(-1), arrival_time(-1), hasTrav(false) {}
    void eval(){
        if ( type == NOT )
            value = !fan_in.front()->value;
        else if ( type == NAND )
            value = !(fan_in.front()->value&&fan_in.back()->value);
        else if ( type == NOR )
            value = !(fan_in.front()->value||fan_in.back()->value);
        else if ( type == OUTPUT )
            value = fan_in.front()->value;
    }
};

std::ostream &operator<<(std::ostream &out, const Gate &gate) {
    out << "Gate name: " << gate.name << endl;
    out << "Gate type: "
        << ((gate.type == NOT) ? "NOT" : (gate.type == NAND) ? "NAND" : "NOR")
        << endl;
    out << "Gate fan in";
    printContainer(gate.fan_in);
    out << endl;
    out << "Gate fan out";
    printContainer(gate.fan_out);
    return out;
}

char *getExpression() {
    string line;
    string expression;
    do {
        getline(cin, line);
        expression += line;
    } while (expression[expression.size() - 1] != ';' && !cin.eof());
    return strdup(expression.c_str());
}

void getTokens(list<char *> &tokens, char *src) {
    char delims[] = " ,;()";
    tokens.clear();
    char *tok = strtok(src, delims);
    do {
        tokens.push_back(tok);
        tok = strtok(NULL, delims);
    } while (tok);
}

struct Network {
    Gate start;
    Gate end;
    char *module_exp, *inputs_exp, *outputs_exp, *wires_exp;
    GateMap gatePool;
    GateMap wirePool;
    GateList evalSequence;
    list<GateList> paths;
    Network() {
        start.name = "start";
        end.name = "end";
        srand(time(NULL));
    }

    Gate *findGateByName(const char *name) {
        GateMap::iterator it;
        if ((it = gatePool.find(name)) != gatePool.end())
            return it->second;
        if ((it = wirePool.find(name)) != wirePool.end())
            return it->second;
        return NULL;
    }

    void gateWiring(Gate *input, Gate *output) {
        output->fan_in.push_back(input);
        input->fan_out.push_back(output);
    }

    void createGraph() {
        ////////////////////////////////////////////////////////////////////////
        // SET INPUT
        ////////////////////////////////////////////////////////////////////////
        list<char *> tokens;
        getTokens(tokens, inputs_exp);
        tokens.pop_front();
        for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            Gate *newGate = new Gate;
            newGate->name = *it;
            newGate->type = INPUT;
            newGate->fan_in.push_back(&start);
            start.fan_out.push_back(newGate);
            gatePool[newGate->name] = newGate;
        }
        start.fan_out_it = start.fan_out.begin();
        ////////////////////////////////////////////////////////////////////////
        // SET OUTPUT
        ////////////////////////////////////////////////////////////////////////
        getTokens(tokens, outputs_exp);
        tokens.pop_front();
        for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            Gate *newGate = new Gate;
            newGate->name = *it;
            newGate->type = OUTPUT;
            end.fan_in.push_back(newGate);
            gatePool[newGate->name] = newGate;
            newGate->fan_out_it = newGate->fan_out.begin();
        }
        end.fan_out_it = end.fan_out.begin();
        ////////////////////////////////////////////////////////////////////////
        // SET WIRE
        ////////////////////////////////////////////////////////////////////////
        getTokens(tokens, wires_exp);
        tokens.pop_front();
        for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
             ++it) {
            Gate *newWire = new Gate;
            newWire->name = *it;
            newWire->type = WIRE;
            wirePool[newWire->name] = newWire;
        }
        ////////////////////////////////////////////////////////////////////////
        // SET GATE
        ////////////////////////////////////////////////////////////////////////
        char *gate_declare;
        char *g1, *g2, *g3;
        Gate *nowGate;
        while (gate_declare = getExpression(),
               strcmp("endmodule", gate_declare)) {
            getTokens(tokens, gate_declare);
            list<char *>::iterator lp = tokens.begin();
            nowGate = new Gate;
            std::advance(lp, 1);
            nowGate->name = string(*lp);
            gatePool[nowGate->name] = nowGate;
            std::advance(lp, 2);
            g1 = *lp;
            std::advance(lp, 2);
            g2 = *lp;
            Gate *input1, *input2, *output;
            if ((input1 = findGateByName(g1))) {
                gateWiring(input1, nowGate);
            } else {
                input1 = new Gate;
                input1->name = g1;
                gateWiring(input1, nowGate);
                wirePool[input1->name] = input1;
            }

            if (!strcmp("NOT1", tokens.front())) {
                nowGate->type = NOT;
                if ((input2 = findGateByName(g2))) {
                    gateWiring(nowGate, input2);
                } else {
                    input2 = new Gate;
                    input2->name = g2;
                    gateWiring(nowGate, input2);
                    wirePool[input2->name] = input2;
                }
            } else {
                nowGate->type = (!strcmp("NOR2", tokens.front())) ? NOR : NAND;
                if ((input2 = findGateByName(g2))) {
                    gateWiring(input2, nowGate);
                } else {
                    input2 = new Gate;
                    input2->name = g2;
                    gateWiring(input2, nowGate);
                    wirePool[input2->name] = input2;
                }

                std::advance(lp, 2);
                g3 = *lp;
                if ((output = findGateByName(g3))) {
                    gateWiring(nowGate, output);
                } else {
                    output = new Gate;
                    output->name = g3;
                    gateWiring(nowGate, output);
                    wirePool[output->name] = output;
                }
            }
        }

        // rewiring
        for ( GateMap::iterator it = wirePool.begin() ; it != wirePool.end() ; ++it ){
            it->second->fan_in.front()->fan_out = it->second->fan_out;
            for ( GateList::iterator wire_out_it = it->second->fan_out.begin() ;
                    wire_out_it != it->second->fan_out.end() ; ++wire_out_it ){
                for( GateList::iterator wire_out_in_it = (*wire_out_it)->fan_in.begin();
                        wire_out_in_it != (*wire_out_it)->fan_in.end() ; ++wire_out_in_it ){
                    if ( it->second == (*wire_out_in_it) ){
                        (*wire_out_in_it) = it->second->fan_in.front();
                    }
                }
            }
        }

        // release wires memory
        for (GateMap::iterator it = wirePool.begin(); it != wirePool.end();
             ++it) {
            delete it->second;
        }

        // reset the fan_out_it, after the original iterator is never effect
        for ( GateMap::iterator it = gatePool.begin() ; it != gatePool.end(); ++it ){
            it->second->fan_out_it = it->second->fan_out.begin();
        }
        delete[] module_exp;
        delete[] outputs_exp;
        delete[] wires_exp;
    }

    void Dfs( unsigned int timing , unsigned int slack ) {
        unsigned int minimun = timing - slack;
        GateList path;
        path.push_back(&start);
        while (path.size()) {
            if (path.back()->fan_out_it == path.back()->fan_out.end()) {
                if (path.back()->type == OUTPUT ) {
                    if ( path.size() - 3 > minimun )
                        paths.push_back(path);
                }
                path.back()->fan_out_it = path.back()->fan_out.begin();
                path.pop_back();
            } else {
                path.push_back(*(path.back()->fan_out_it++));
            }
        }
        for(list<GateList>::iterator it = paths.begin();
        it != paths.end(); ++it)
        {
            it->pop_front();
        }
    }
    void topologySort() {
        GateList stack;
        stack.push_back(&start);
        start.hasTrav = true;
        while (stack.size()) {
            if (stack.back()->fan_out_it == stack.back()->fan_out.end()) {
                evalSequence.push_front(stack.back());
                if(evalSequence.front()->type == INPUT)
                    evalSequence.pop_front();
                stack.pop_back();
            } else {
                if(!(*stack.back()->fan_out_it)->hasTrav) {
                    stack.push_back(*(stack.back()->fan_out_it++));
                    stack.back()->hasTrav = true;
                } else {
                    ++stack.back()->fan_out_it;
                }
            }
        }
        evalSequence.pop_front();
    }

    void random2Shrink(){
        randomInput();
        evalNetworkValue();
        findTruePath();
    }

    // pattern + 1
    void addOne( vector<int>& pattern ){
        pattern[0]++;
        for ( int i = 0 ; i < pattern.size()-1 ; i ++ ){
            if ( pattern[i] == 2 ){
                pattern[i] = 0;
                pattern[i+1]++;
            }
            else
                break;
        }
    }
    // just for test pattern 1 1 1 1
    void forTest() {
        GateList::iterator it = start.fan_out.begin();
        for ( ;it != start.fan_out.end() ; ++it ){
            (*it)->value = rand() % 2;
        }
        evalNetworkValue();
        findTruePath();
        clearNetworkValue();
    }

    void force(){
        vector<int> pattern;
        pattern.resize( start.fan_out.size() );
        int times = 1;
        for ( int i = 0 ; i < pattern.size() ; i ++ ){
            pattern[i] = 0;
            times *= 2;
        }

        for ( int i = 0 ; i < times ; i ++ ){
            for ( vector<int>::iterator pattern_it = pattern.begin();
                  pattern_it != pattern.end(); ++pattern_it)
                cout << *pattern_it << " ";
            cout << endl;
            int index = 0;
            for ( GateList::iterator it = start.fan_out.begin();
                    it != start.fan_out.end();++it){
                (*it)->value = pattern[index++];
            }
            evalNetworkValue();
            test2PrintGateValue();
            findTruePath();
            clearNetworkValue();
            addOne(pattern);
        }
    }

    void findTruePath(){
        for (list<GateList>::iterator paths_it = paths.begin();
                paths_it != paths.end(); ++paths_it )
        {
            bool isTruePath = true;
            Gate* me = paths_it->front();
            for (GateList::iterator path_it = paths_it->begin()
                ; path_it != paths_it->end(); ++path_it )
            {
                Gate* you;
                if ( (*path_it)->type == NAND ){
                    if ( me == (*path_it)->fan_in.front() ){
                        you =  (*path_it)->fan_in.back();
                    }
                    else{
                        you = (*path_it)->fan_in.front();
                        me = (*path_it)->fan_in.back();
                    }
                    isTruePath = subFindTruePath( (*path_it)->type , me , you );
                }
                else if ( (*path_it)->type == NOR ){
                    if ( me == (*path_it)->fan_in.front() ){
                        you =  (*path_it)->fan_in.back();
                    }
                    else{
                        you = (*path_it)->fan_in.front();
                        me = (*path_it)->fan_in.back();
                    }
                    isTruePath = subFindTruePath( (*path_it)->type , me , you );
                }
                if ( !isTruePath )
                    break;
                me = (*path_it);
            }
            if ( isTruePath ){
                cout << ((*paths_it).front())->value << " ";
                printContainer(*paths_it);
                cout << endl;
            }
        }
    }

    // test to print  all the gate value
    void test2PrintGateValue(){
        cout << "~~~~~~~~~~~~~~~~~~~~~~\n";
        for ( GateMap::iterator it = gatePool.begin();
                it != gatePool.end() ; ++it ){
                cout << it->first << "(" << it->second->value << ")" << endl;
                cout << it->first << "time: " << it->second->arrival_time <<
                endl << "~~~~" << endl;
        }
        cout << "~~~~~~~~~~~~~~~~~~~~~~\n";
    }

    bool subFindTruePath( GateType type , Gate* me , Gate* you ){
        bool isTruePath = true;
        if ( type == NAND ){
            if ( me->arrival_time != you->arrival_time){
                if ( me->arrival_time > you->arrival_time ){
                    if ( you->value == 0 ){
                        isTruePath = false;
                    }
                }
                else{
                    if ( me->value == 1 ){
                        isTruePath = false;
                    }
                }
            }
            else{
                if ( me->value != you->value ){
                    if ( me->value == 1 && you->value == 0 ){
                        isTruePath = false;
                    }
                }
            }
        }
        else if ( type == NOR ){
            if ( me->arrival_time != you->arrival_time){
                if ( me->arrival_time > you->arrival_time ){
                    if ( you->value == 1 ){
                        isTruePath = false;
                    }
                }
                else{
                    if ( me->value == 0 ){
                        isTruePath = false;
                    }
                }
            }
            else{
                if ( me->value != you->value ){
                    if ( me->value == 0 && you->value == 1 ){
                        isTruePath = false;
                    }
                }
            }
        }
        return isTruePath;
    }

    // return the last arrival fan_in, if anyone is unready return NULL
    Gate* isReady( Gate* out ){
        Gate* temp = out->fan_in.front();
        for ( GateList::iterator it = out->fan_in.begin() ; it != out->fan_in.end() ; ++it ){
            if ( (*it)->arrival_time == -1 )
                return NULL;
            if ( temp->arrival_time < (*it)->arrival_time )
                temp = (*it);
        }
        return temp;
    }

    // evaluate each gate's arrival time
    void evalArrivalTime(){
        for (GateList::iterator it = start.fan_out.begin();
             it != start.fan_out.end(); ++it)
        {
            (*it)->arrival_time = 0;
        }

        for (GateList::iterator it = evalSequence.begin();
             it != evalSequence.end(); ++it)
        {
            (*it)->arrival_time = isReady(*it)->arrival_time + 1;
        }
    }
    // support the evalNetworkValue function
    bool isReady2Eval( Gate* out ){

        for ( GateList::iterator it = out->fan_in.begin() ; it != out->fan_in.end() ; ++it ){
            if ( (*it)->value == -1 )
                return false;
        }
        return true;
    }

    // evaluate each gate's value
    void evalNetworkValue(){
        for (GateList::iterator it = evalSequence.begin();
             it != evalSequence.end(); ++it)
        {
            (*it)->eval();
        }
    }

    // random test pattern
    void randomInput(){
        /**************************************************************************************/
        srand(time(0));
        cout << "the random pattern: " << endl;
        for ( GateList::iterator it = start.fan_out.begin() ; it != start.fan_out.end() ; ++it ){
            (*it)->value = rand() % 2;
            cout << (*it)->value << " ";
        }
        cout << endl;
        /**************************************************************************************/
    }

    // reset each gate's value
    void clearNetworkValue(){
        for(GateMap::iterator it = gatePool.begin(); it != gatePool.end() ; ++it ){
            it->second->value = -1;
        }
    }

    ~Network() {
        for (GateMap::iterator it = gatePool.begin(); it != gatePool.end();
             ++it) {
            delete it->second;
        }
    }
};
#endif
