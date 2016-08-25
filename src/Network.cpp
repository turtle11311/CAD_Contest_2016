#include "Network.h"
#include <pthread.h>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <utility>
using std::cout;
using std::endl;
using std::list;
using std::cin;
using std::string;
using std::setw;
using namespace std::placeholders;
using std::for_each;
using std::bind;

static pthread_mutex_t mutex;

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

Path::Path() : isFind({false, false})
{}

void output_format(args_t arg, Path &path) {
    Network *net = arg.first;
    int pid = arg.second;
    cout << "\nA True Path List\n{\n"
        << "---------------------------------------------------------------------------\n"
        << "Pin" <<"    "<< "type" <<"                                "<< "Incr" <<"        "<< "Path delay\n"
        << "---------------------------------------------------------------------------\n";
    int constrain = net->timing;
    GateList::iterator it = path.begin();
    GateList::iterator temp;
    GateType pattern_type;
    int slack_time = 0;
    std::string buf;
    for (; it != path.end(); ++it){
        if ((*it)->type == 5 || (*it)->type == 6){
            cout << std::left << setw(44);
            buf = (*it)->name + " (";
            switch((*it)->type)
            {
                case 5:
                    buf+="in)";
                    break;
                case 6:
                    buf+="out)";
                    break;
            }
            cout << buf << setw(11) << "0" << (*it)->arrival_time[pid] - ((*it)->type == OUTPUT) << " ";
            if((*it)->value[pid])
                cout << "r\n";
            else
                cout << "f\n";
        }

        if ((*it)->type != 5 && (*it)->type != 6){
            cout << std::left << setw(44);
            buf = (*it)->name;
            if(*temp == (*it)->fan_in.front())
                buf+="/A (";
            else
                buf+="/B (";
            switch((*it)->type)
            {
                case 1:
                    buf+="NOT";
                    break;
                case 2:
                    buf+="NOR";
                    break;
                case 3:
                    buf+="NAND";
                    break;
            }
            if ((*it)->type == 1)
                buf+="1)";
            else if ((*it)->type == 2 || (*it)->type == 3)
                buf+="2)";
            cout << buf << setw(11) << "0" << (*temp)->arrival_time[pid]  << " ";
            if((*temp)->value[pid])
                cout << "r\n";
            else
                cout << "f\n";

            cout << std::left << setw(44);
            buf = (*it)->name;
            buf+="/Y (";

            switch((*it)->type)
            {
                case 1:
                    buf+="NOT";
                    break;
                case 2:
                    buf+="NOR";
                    break;
                case 3:
                    buf+="NAND";
                    break;
            }
            if ((*it)->type == 1)
                buf+="1)";
            else if ((*it)->type == 2 || (*it)->type == 3)
                buf+="2)";
            cout << buf << setw(11) << "1" << (*it)->arrival_time[pid]  << " ";
            if((*it)->value[pid])
                cout << "r\n";
            else
                cout << "f\n";
            slack_time = (*it)->arrival_time[pid];
        }
        temp = it;
    }
    cout << "--------------------------------------------------------------------------\n"
        << setw(30) << "Data Required Time"  << constrain << endl;
    cout << setw(30)  << "Data Arrival Time" ;
    cout << slack_time << endl <<
        "--------------------------------------------------------------------------\n";
    cout << setw(30) << "Slack"   << constrain - slack_time << "\n}\n\n"
        << "Input Vector\n{\n";
    for (GateList::iterator it = net->start.fan_out.begin(); it != net->start.fan_out.end(); ++it)
    {
        cout << (*it)->name << " = ";
        if(*it == *(path.begin()))
        {
            if((*it)->value[pid])
                cout << "r\n";
            else
                cout << "f\n";
        }
        else
            cout << (*it)->value[pid] << endl;
    }
    cout << "}\n";
}

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
    : inputFile(in), timing(timing), slack(slack), start("start"), end("end"),
      pathCounter(0), minimun(timing - slack)
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
    /*=======================================================================*/
    list<char *> tokens;
    module_exp = getExpression();
    inputs_exp = getExpression();
    outputs_exp = getExpression();
    wires_exp = getExpression();
    ///////////////////////////////////////////////////////////////////////////
    // Module
    ///////////////////////////////////////////////////////////////////////////
    getTokens(tokens, module_exp);
    tokens.pop_front();
    moduleName = tokens.front();
    ///////////////////////////////////////////////////////////////////////////
    // SET INPUT
    ///////////////////////////////////////////////////////////////////////////
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
    ///////////////////////////////////////////////////////////////////////////
    // SET OUTPUT
    ///////////////////////////////////////////////////////////////////////////
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
    //////////////////////////////////////////////////////////////////////////
    // SET WIRE
    //////////////////////////////////////////////////////////////////////////
    getTokens(tokens, wires_exp);
    tokens.pop_front();
    for (list<char *>::iterator it = tokens.begin(); it != tokens.end();
            ++it) {
        Gate *newWire = new Gate;
        newWire->name = *it;
        newWire->type = WIRE;
        wirePool[newWire->name] = newWire;
    }
    //////////////////////////////////////////////////////////////////////////
    // SET GATE
    //////////////////////////////////////////////////////////////////////////
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
    for (auto &it : wirePool)
        delete it.second;

    resetAllfan_out_it();
    delete[] module_exp;
    delete[] outputs_exp;
    delete[] wires_exp;
}

// reset the fan_out_it, after the original iterator is never effect
void Network::resetAllfan_out_it() {
    for (auto &it : gatePool)
        it.second->fan_out_it = it.second->fan_out.begin();
}

GateSet Network::findAssociatePI(Gate* in) {
    GateSet PISet;
    GateList Queue;
    Queue.push_back(in);
    while( Queue.size() ){
        for ( Gate* gate : Queue.front()->fan_in )
            if ( gate != &start )
                Queue.push_back(gate);
        if ( Queue.front()->type == INPUT )
            PISet.insert(Queue.front());
        Queue.pop_front();
    }
    return PISet;
}

void Network::findAllPath() {
    Path path;
    for (Gate* inputport : start.fan_out) {
        path.push_back(inputport);
        while (path.size()) {
            if (path.back()->fan_out_it == path.back()->fan_out.end()) {
                if (path.back()->type == OUTPUT) {
                    if (path.size() - 2ul > minimun) {
                        IOMap[path.back()].insert(path.front());
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
    }
}

void Network::printIOMap(){
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
    for ( std::map<Gate*, GateSet>::iterator map_it = IOMap.begin() ;
            map_it != IOMap.end() ; ++map_it ){
        cout << "PO's name: " << map_it->first->name << endl;
        for ( GateSet::iterator set_it = map_it->second.begin();
                set_it != map_it->second.end() ; ++set_it ){
            cout << (*set_it)->name << ", ";
        }
        cout << endl;
    }
    cout << "~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void Network::printAllPaths() {
    for (Path &path : paths)
        printContainer(path), cout << endl;
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
    resetAllfan_out_it();
}

void Network::random2Shrink(int pid){
    randomInput(pid);
    evalNetwork(pid);
    findTruePath(pid);
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
        (*it)->value[0] = rand() % 2;
    }
    evalNetwork(0);
    evalFLTime();
    findTruePath(0);
}

void Network::force() {
    std::vector<int> pattern;
    pattern.resize(start.fan_out.size());
    for (int i = 0; i < pattern.size(); i++){
        pattern[i] = 1;
    }

    do {
        int index = 0;
        for (Gate* inputport : start.fan_out) {
            inputport->value[0] = pattern[index++];
        }
        evalNetwork(0);
        findTruePath(0);
        addOne(pattern);
    }while(!isAllOne(pattern));
}

void Network::findTruePath(int pid) {
    for (Path &path : paths) {
        short type = path.front()->value[pid];
        if (path.isFind[type])
            continue;
        bool isTruePath = true;
        Gate* me = path.front();
        for (Gate* gate : path) {
            Gate* you;
            if (gate->type == NAND || gate->type == NOR) {
                if (me == gate->fan_in.front()){
                    you = gate->fan_in.back();
                }
                else{
                    you = gate->fan_in.front();
                    me = gate->fan_in.back();
                }
                isTruePath = subFindTruePath(pid, gate->type, me, you);
            }
            if (!isTruePath)
                break;
            me = gate;
        }
        if (isTruePath) {
            bool Print = false;
            pthread_mutex_lock(&mutex);
            Print = !path.isFind[type];
            path.isFind[type] = true;
            if (Print) {
                cout << "\nPath  {  " << ++pathCounter << "  }" << endl;
                output_format({this, pid}, path);
            }
            pthread_mutex_unlock(&mutex);
        }
    }
}

bool Network::subFindTruePath(int pid, GateType type, Gate* me, Gate* you){
    bool isTruePath = true;
    if (type == NAND) {
        if (me->arrival_time[pid] != you->arrival_time[pid]){
            if (me->arrival_time[pid] > you->arrival_time[pid]){
                if (you->value[pid] == 0){
                    isTruePath = false;
                }
            }
            else{
                if (me->value[pid] == 1){
                    isTruePath = false;
                }
            }
        }
        else{
            if (me->value[pid] != you->value[pid]){
                if (me->value[pid] == 1 && you->value[pid] == 0){
                    isTruePath = false;
                }
            }
        }
    }
    else if (type == NOR){
        if (me->arrival_time[pid] != you->arrival_time[pid]){
            if (me->arrival_time[pid] > you->arrival_time[pid]){
                if (you->value[pid] == 1){
                    isTruePath = false;
                }
            }
            else{
                if (me->value[pid] == 0){
                    isTruePath = false;
                }
            }
        }
        else{
            if (me->value[pid] != you->value[pid]){
                if (me->value[pid] == 0 && you->value[pid] == 1){
                    isTruePath = false;
                }
            }
        }
    }
    return isTruePath;
}

void Network::genPISequence(Path &path) {
    GateList criticList;
    Gate* me = path.front();
    for (GateList::iterator it = start.fan_out.begin();
        it != start.fan_out.end(); ++it) {
        (*it)->hasTrav = false;
    }
    // put critical gate to trySeq
    for (Path::iterator it = path.begin(); it != path.end(); ++it) {
        if ((*it)->type == NAND || (*it)->type == NOR) {
            Gate *you;
            if ((*it)->type == NAND || (*it)->type == NOR) {
                if (me == (*it)->fan_in.front()){
                    you = (*it)->fan_in.back();
                }
                else {
                    you = (*it)->fan_in.front();
                    me = (*it)->fan_in.back();
                }
            }
            if (you->first_in > me->last_in || you->last_in < me->first_in) {
                GateSet set = findAssociatePI(*it);
                for (GateSet::iterator set_it = set.begin();
                     set_it != set.end(); ++set_it) {
                    if (!(*set_it)->hasTrav) {
                        path.PISequence.push_back(*set_it);
                        (*set_it)->hasTrav = true;
                    }
                }
            }
        }
        me = *it;
    }
    //add AccosiateSeq
    GateSet acSet = findAssociatePI(path.back());
    for (Gate *gate : acSet) {
        if (!gate->hasTrav) {
            path.PISequence.push_back(gate);
            gate->hasTrav = true;
        }
    }
}

void Network::genAllPISequence() {
    for (Path &path : paths)
        genPISequence(path);
}

// test to print  all the gate value
void Network::test2PrintGateValue(int pid) {
    cout << "~~~~~~~~~~~~~~~~~~~~~~\n";
    for (auto &eachGate : gatePool) {
        cout << eachGate.first << endl;
        cout << "value: " << eachGate.second->value[pid] << endl;
        cout << "time: " << eachGate.second->arrival_time[pid] << endl;
        cout << "First in: " << "(" << eachGate.second->first_in << ")" << endl;
        cout << "Last in: " << "(" << eachGate.second->last_in << ")" << endl;
    }
    cout << "~~~~~~~~~~~~~~~~~~~~~~\n";
}

// return the last arrival fan_in, if anyone is unready return NULL
Gate* Network::isReady(int pid, Gate* out) {
    Gate* temp = out->fan_in.front();
    for (GateList::iterator it = out->fan_in.begin(); it != out->fan_in.end(); ++it){
        if ((*it)->arrival_time[pid] == -1)
            return NULL;
        if (temp->arrival_time[pid] < (*it)->arrival_time[pid])
            temp = (*it);
    }
    return temp;
}

// evaluate first/last in
void Network::evalFLTime(){
    for ( GateList::iterator it = evalSequence.begin(); it != evalSequence.end() ; ++it){
        (*it)->first_in = ( (*it)->type != NAND && (*it)->type != NOR ) ?
            ((*it)->fan_in.front()->first_in + 1) :
            (((*it)->fan_in.front()->first_in < (*it)->fan_in.back()->first_in) ?
             ((*it)->fan_in.front()->first_in + 1) : ((*it)->fan_in.back()->first_in + 1)
             );
        (*it)->last_in = ( (*it)->type != NAND && (*it)->type != NOR ) ?
            ((*it)->fan_in.front()->last_in + 1) :
            (((*it)->fan_in.front()->last_in > (*it)->fan_in.back()->last_in) ?
             ((*it)->fan_in.front()->last_in + 1) : ((*it)->fan_in.back()->last_in + 1)
             );
    }
}

// evaluate each gate's value
void Network::evalNetwork(int pid) {
    for (Gate* gate : evalSequence) {
        gate->eval(pid);
        gate->arrival_time[pid] = gate->trueinput(pid)->arrival_time[pid] + 1;
    }
}

// random test pattern
void Network::randomInput(int pid) {
    for (Gate* inputport : start.fan_out) {
        inputport->value[pid] = rand() % 2;
    }
}

void Network::parallelFindTruePath() {
    cout << "Header  {  A True Path Set  }" << endl << endl;
    cout << "Benchmark  {  " << moduleName << "  }" << endl;
    pthread_t pid[4];
    args_t args[4];
    pthread_mutex_init(&mutex, NULL);
    int ID[4];
    for (int i = 0; i < 4; ++i) {
        ID[i] = i;
        args[i].first = this;
        args[i].second = i;
        pthread_create(&pid[i], NULL, findPatternTruePath, &args[i]);
    }
    for (int i = 0; i < 4; ++i) {
        pthread_join(pid[i], NULL);
    }
}

void* findPatternTruePath(void *args) {
    args_t arg = *(args_t*)args;
    Network *net = arg.first;
    int ID = arg.second;
    for(int i = 0; i < 100000; ++i) {
        if (net->pathCounter >= net->paths.size() * 2)
            break;
        net->random2Shrink(ID);
    }
    pthread_exit(NULL);
}

Network::~Network() {
    for (auto &it : gatePool)
        delete it.second;
}
