#include "Network.h"
#include <ctime>
#include <cstdlib>
using std::cout;
using std::endl;
using std::list;
using std::cin;
using std::string;

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

void getTokens(list<char *> &tokens, char *src) {
    char delims[] = " ,;()";
    tokens.clear();
    char *tok = strtok(src, delims);
    do {
        tokens.push_back(tok);
        tok = strtok(NULL, delims);
    } while (tok);
}

Path::Path() : rising(false), falling(false)
{}

char *Network::getExpression() {
    string line;
    string expression;
    do {
        getline(inputFile, line);
        expression += line;
    } while (expression[expression.size() - 1] != ';' && !inputFile.eof());
    return strdup(expression.c_str());
}

Network::Network(unsigned int timing, unsigned int slack, std::istream& in)
    : inputFile(in), timing(timing), slack(slack), start("start"), end("end")
{
    srand(time(NULL));
}

Gate *Network::findGateByName(const char *name) {
    GateMap::iterator it;
    if ((it = gatePool.find(name)) != gatePool.end())
        return it->second;
    if ((it = wirePool.find(name)) != wirePool.end())
        return it->second;
    return NULL;
}

void Network::gateWiring(Gate *input, Gate *output) {
    output->fan_in.push_back(input);
    input->fan_out.push_back(output);
}

void Network::createGraph() {
    // clear heading comment
    string line;
    do {
        getline(inputFile, line);
    } while (line.substr(0, 2) == "//");
    /*========================================================================*/
    module_exp = getExpression();
    inputs_exp = getExpression();
    outputs_exp = getExpression();
    wires_exp = getExpression();
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
        }
        else {
            input1 = new Gate;
            input1->name = g1;
            gateWiring(input1, nowGate);
            wirePool[input1->name] = input1;
        }

        if (!strcmp("NOT1", tokens.front())) {
            nowGate->type = NOT;
            if ((input2 = findGateByName(g2))) {
                gateWiring(nowGate, input2);
            }
            else {
                input2 = new Gate;
                input2->name = g2;
                gateWiring(nowGate, input2);
                wirePool[input2->name] = input2;
            }
        }
        else {
            nowGate->type = (!strcmp("NOR2", tokens.front())) ? NOR : NAND;
            if ((input2 = findGateByName(g2))) {
                gateWiring(input2, nowGate);
            }
            else {
                input2 = new Gate;
                input2->name = g2;
                gateWiring(input2, nowGate);
                wirePool[input2->name] = input2;
            }

            std::advance(lp, 2);
            g3 = *lp;
            if ((output = findGateByName(g3))) {
                gateWiring(nowGate, output);
            }
            else {
                output = new Gate;
                output->name = g3;
                gateWiring(nowGate, output);
                wirePool[output->name] = output;
            }
        }
    }

    // rewiring
    for (GateMap::iterator it = wirePool.begin(); it != wirePool.end(); ++it){
        it->second->fan_in.front()->fan_out = it->second->fan_out;
        for (GateList::iterator wire_out_it = it->second->fan_out.begin();
                wire_out_it != it->second->fan_out.end(); ++wire_out_it){
            for (GateList::iterator wire_out_in_it = (*wire_out_it)->fan_in.begin();
                    wire_out_in_it != (*wire_out_it)->fan_in.end(); ++wire_out_in_it){
                if (it->second == (*wire_out_in_it)){
                    (*wire_out_in_it) = it->second->fan_in.front();
                }
            }
        }
    }

    for (GateList::iterator it = end.fan_in.begin();
            it != end.fan_in.end(); ++it) {
        if ((*it)->fan_out.size()) {
            for (GateList::iterator fan_out_it = (*it)->fan_out.begin();
                    fan_out_it != (*it)->fan_out.end(); ++fan_out_it) {
                if ((*fan_out_it)->fan_in.front() == (*it))
                    (*fan_out_it)->fan_in.front() = (*it)->fan_in.front();
                else
                    (*fan_out_it)->fan_in.back() = (*it)->fan_in.front();
                (*it)->fan_in.front()->fan_out.push_back(*fan_out_it);
            }
            (*it)->fan_out.clear();
        }
    }

    // release wires memory
    for (GateMap::iterator it = wirePool.begin(); it != wirePool.end();
            ++it) {
        delete it->second;
    }

    // reset the fan_out_it, after the original iterator is never effect
    for (GateMap::iterator it = gatePool.begin(); it != gatePool.end(); ++it){
        it->second->fan_out_it = it->second->fan_out.begin();
    }
    delete[] module_exp;
    delete[] outputs_exp;
    delete[] wires_exp;
}

void Network::DFS() {
    unsigned int minimun = timing - slack;
    int sum = 0;
    Path path;
    path.push_back(&start);
    while (path.size()) {
        if (path.back()->fan_out_it == path.back()->fan_out.end()) {
            if (path.back()->type == OUTPUT) {
                ++sum;
                if (path.size() - 3ul > minimun) {
                    paths.push_back(path);
                }
            }
            path.back()->fan_out_it = path.back()->fan_out.begin();
            path.pop_back();
        }
        else {
            path.push_back(*(path.back()->fan_out_it++));
        }
    }
    for (list<Path>::iterator it = paths.begin();
            it != paths.end(); ++it)
    {
        it->pop_front();
    }
}

// Generate a sequence for evaluate network value without START GATE
void Network::topologySort() {
    GateList stack;
    stack.push_back(&start);
    start.hasTrav = true;
    while (stack.size()) {
        if (stack.back()->fan_out_it == stack.back()->fan_out.end()) {
            evalSequence.push_front(stack.back());
            if (evalSequence.front()->type == INPUT)
                evalSequence.pop_front();
            stack.pop_back();
        }
        else {
            if (!(*stack.back()->fan_out_it)->hasTrav) {
                stack.push_back(*(stack.back()->fan_out_it++));
                stack.back()->hasTrav = true;
            }
            else {
                ++stack.back()->fan_out_it;
            }
        }
    }
    evalSequence.pop_front();
}

void Network::random2Shrink(){
    randomInput();
    evalNetwork();
    findTruePath();
}

// pattern + 1
void Network::addOne(std::vector<int>& pattern){
    pattern[0]++;
    for (int i = 0; i < pattern.size(); i++){
        if (pattern[i] == 2){
            pattern[i] = 0;
            if (i + 1 < pattern.size())
                pattern[i + 1]++;
        }
        else
            break;
    }
}

bool isAllOne(std::vector<int> pattern){
    for ( int i = 0 ; i < pattern.size() ; ++i )
        if ( pattern[i] == 0 )
            return false;
    return true;
}

// just for test pattern 1 1 1 1
void Network::forTest() {
    GateList::iterator it = start.fan_out.begin();
    for (; it != start.fan_out.end(); ++it){
        (*it)->value = rand() % 2;
    }
    evalNetwork();
    //test2PrintGateValue();
    findTruePath();
}

void Network::force() {
    std::vector<int> pattern;
    pattern.resize(start.fan_out.size());
    for (int i = 0; i < pattern.size(); i++){
        pattern[i] = 1;
    }

    do {
        int index = 0;
        for (GateList::iterator it = start.fan_out.begin();
                it != start.fan_out.end() ; ++it ){
            (*it)->value = pattern[index++];
        }
        evalNetwork();
        findTruePath();
        addOne(pattern);
    }while(!isAllOne(pattern));
}

void Network::findTruePath() {
    for (list<Path>::iterator paths_it = paths.begin();
            paths_it != paths.end(); ++paths_it)
    {
        short type = ((*paths_it).front())->value;
        if ((!type && paths_it->falling) || (type && paths_it->rising))
            continue;
        bool isTruePath = true;
        Gate* me = paths_it->front();
        for (Path::iterator path_it = paths_it->begin()
                ; path_it != paths_it->end(); ++path_it)
        {
            Gate* you;
            if ((*path_it)->type == NAND || (*path_it)->type == NOR) {
                if (me == (*path_it)->fan_in.front()){
                    you = (*path_it)->fan_in.back();
                }
                else{
                    you = (*path_it)->fan_in.front();
                    me = (*path_it)->fan_in.back();
                }
                isTruePath = subFindTruePath((*path_it)->type, me, you);
            }
            if (!isTruePath)
                break;
            me = (*path_it);
        }
        if (isTruePath) {
            if (type) {
                paths_it->rising = true;
            } else {
                paths_it->falling = true;
            }
            cout << type << " ";
            printContainer(*paths_it);
            cout << endl;
        }
    }
}

bool Network::subFindTruePath(GateType type, Gate* me, Gate* you){
    bool isTruePath = true;
    if (type == NAND) {
        if (me->arrival_time != you->arrival_time){
            if (me->arrival_time > you->arrival_time){
                if (you->value == 0){
                    isTruePath = false;
                }
            }
            else{
                if (me->value == 1){
                    isTruePath = false;
                }
            }
        }
        else{
            if (me->value != you->value){
                if (me->value == 1 && you->value == 0){
                    isTruePath = false;
                }
            }
        }
    }
    else if (type == NOR){
        if (me->arrival_time != you->arrival_time){
            if (me->arrival_time > you->arrival_time){
                if (you->value == 1){
                    isTruePath = false;
                }
            }
            else{
                if (me->value == 0){
                    isTruePath = false;
                }
            }
        }
        else{
            if (me->value != you->value){
                if (me->value == 0 && you->value == 1){
                    isTruePath = false;
                }
            }
        }
    }
    return isTruePath;
}

// test to print  all the gate value
void Network::test2PrintGateValue() {
    cout << "~~~~~~~~~~~~~~~~~~~~~~\n";
    for (GateMap::iterator it = gatePool.begin();
            it != gatePool.end(); ++it){
        cout << it->first << "(" << it->second->value << ")" << endl;
        cout << "time: " << it->second->arrival_time <<
            endl << "~~~~" << endl;
    }
    cout << "~~~~~~~~~~~~~~~~~~~~~~\n";
}

// return the last arrival fan_in, if anyone is unready return NULL
Gate* Network::isReady(Gate* out) {
    Gate* temp = out->fan_in.front();
    for (GateList::iterator it = out->fan_in.begin(); it != out->fan_in.end(); ++it){
        if ((*it)->arrival_time == -1)
            return NULL;
        if (temp->arrival_time < (*it)->arrival_time)
            temp = (*it);
    }
    return temp;
}

// evaluate each gate's value
void Network::evalNetwork() {
    for (GateList::iterator it = evalSequence.begin();
            it != evalSequence.end(); ++it)
    {
        (*it)->eval();
        (*it)->arrival_time = (*it)->trueinput()->arrival_time + 1;
    }
}

// random test pattern
void Network::randomInput() {
    cout << "the random pattern: " << endl;
    for (GateList::iterator it = start.fan_out.begin(); it != start.fan_out.end(); ++it){
        (*it)->value = rand() % 2;
        cout << (*it)->value << " ";
    }
    cout << endl;
}

// reset each gate's value
void Network::clearNetworkValue() {
    for (GateMap::iterator it = gatePool.begin(); it != gatePool.end(); ++it){
        it->second->value = -1;
    }
}

Network::~Network() {
    for (GateMap::iterator it = gatePool.begin(); it != gatePool.end();
            ++it) {
        delete it->second;
    }
}
