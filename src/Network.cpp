#include "Network.h"
#include <thread>
#include <mutex>
#include <ctime>
#include <climits>
#include <cmath>
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <iomanip>
#include <iterator>
#include <cstdio>
using std::cout;
using std::cin;
using std::setw;
using std::endl;
using std::flush;
using std::list;
using std::string;
using std::vector;
using std::map;
using std::next;
using std::prev;
using std::move;

static std::mutex mutex;

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

Path::Path() : GateVector(), isFind{false, false}
{}

Path::Path(Path &path) : GateVector(path), isFind{false, false}
{}

void Network::output_format(size_t pid, Path &path) {
    outputFile << "\nPath  {  " << ++truePathCounter << "  }\n";
    outputFile << "\n    A True Path List\n    {\n"
        << "    ---------------------------------------------------------------------------\n"
        << "    Pin" <<"    "<< "type" <<"                                "<< "Incr" <<"        "<< "Path delay\n"
        << "    ---------------------------------------------------------------------------\n";
    int constrain = timing;
    auto it = path.begin();
    auto temp = it;
    int slack_time = 0;
    string buf;
    for (; it != path.end(); ++it){
        buf = "    ";
        if ((*it)->type == INPUT || (*it)->type == OUTPUT){
            outputFile << std::left << setw(44);
            buf += (*it)->name + " " + ((*it)->type == INPUT ? "(in)" : "(out)");
            outputFile << buf << setw(11) << "0" << (*it)->arrival_time[pid] - ((*it)->type == OUTPUT) << " ";
            if((*it)->value[pid])
                outputFile << "r\n";
            else
                outputFile << "f\n";
        } else {
            outputFile << std::left << setw(44);
            buf += (*it)->name;
            if(*temp == (*it)->fan_in.front())
                buf+="/A (";
            else
                buf+="/B (";
            switch((*it)->type)
            {
                case NOT:
                    buf+="NOT";
                    break;
                case NOR:
                    buf+="NOR";
                    break;
                case NAND:
                    buf+="NAND";
                    break;
                default:
                    break;
            }
            if ((*it)->type == NOT)
                buf+="1)";
            else if ((*it)->type == NOR || (*it)->type == NAND)
                buf+="2)";
            outputFile << buf << setw(11) << "0" << (*temp)->arrival_time[pid]  << " ";
            if((*temp)->value[pid])
                outputFile << "r\n";
            else
                outputFile << "f\n";

            outputFile << std::left << setw(44);
            buf = "    " + (*it)->name;
            buf+="/Y (";

            switch((*it)->type)
            {
                case NOT:
                    buf+="NOT";
                    break;
                case NOR:
                    buf+="NOR";
                    break;
                case NAND:
                    buf+="NAND";
                    break;
                default:
                    break;
            }
            if ((*it)->type == NOT)
                buf+="1)";
            else if ((*it)->type == NOR || (*it)->type == NAND)
                buf+="2)";
            outputFile << buf << setw(11) << "1" << (*it)->arrival_time[pid]  << " ";
            if((*it)->value[pid])
                outputFile << "r\n";
            else
                outputFile << "f\n";
            slack_time = (*it)->arrival_time[pid];
        }
        temp = it;
    }
    outputFile << "    --------------------------------------------------------------------------\n"
        << setw(30) << "    Data Required Time"  << constrain << endl;
    outputFile << setw(30)  << "    Data Arrival Time" ;
    outputFile << slack_time << "\n" <<
        "    --------------------------------------------------------------------------\n";
    outputFile << setw(30) << "    Slack"   << constrain - slack_time << "\n    }\n\n"
        << "    Input Vector\n    {\n";
    for (auto it = start.fan_out.begin(); it != start.fan_out.end(); ++it)
    {
        outputFile << "        " << (*it)->name << " = ";
        if(*it == *(path.begin()))
        {
            if((*it)->value[pid])
                outputFile << "r\n";
            else
                outputFile << "f\n";
        }
        else
            outputFile << (~(*it)->value[pid] ? (*it)->value[pid] : 0) << "\n";
    }
    outputFile << "    }\n";
    outputFile << flush;
}

void Network::startFindTruePath() {
    size_t PISize = start.fan_out.size();
    findAllPath();
    topologySort();
    outputFile << "Header  {  A True Path Set  }" << endl << endl;
    outputFile << "Benchmark  {  " << moduleName << "  }" << endl;
    if (PISize <= 20) {
        parallelExhaustiveMethod();
    } else {
        if (PISize >= 60)
            branchLimit = PISize * 5;
        evalFLTime();
        cout << "start PISeq" << endl;
        genAllPISequence();
        parallelBranchAndBound();
    }
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

Network::Network(unsigned int timing, unsigned int slack,
                 std::istream& in, std::ostream& out)
    : inputFile(in), outputFile(out), timing(timing), slack(slack), start("start"), end("end"),
      truePathCounter(0), pathCounter(0), minimun(timing - slack), branchLimit(ULLONG_MAX)
{
    for (size_t i = 0; i < ThreadNumber; ++i) {
        branchCounter[i] = 0;
    }
    srand(time(NULL));
    paths.reserve(5000);
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
    Gate *nowGate;
    vector<char *> gate_tokens;
    map<string, string> port;
    gate_tokens.reserve(9);
    while (gate_declare = getExpression(), strcmp("endmodule", gate_declare)) {
        getTokens(gate_tokens, gate_declare);
        auto lp = gate_tokens.begin();
        nowGate = new Gate;
        nowGate->name = string(lp[1]);
        gatePool[nowGate->name] = nowGate;
        port[lp[2]] = lp[3];
        port[lp[4]] = lp[5];
        Gate *input1, *input2, *output;
        if ((input1 = findGateByName(port["A"].c_str()))) {
            gateWiring(input1, nowGate);
        }
        else {
            input1 = new Gate;
            input1->name = port["A"];
            gateWiring(input1, nowGate);
            wirePool[input1->name] = input1;
        }

        if (!strcmp("NOT1", lp[0])) {
            nowGate->type = NOT;
            if ((input2 = findGateByName(port["Y"].c_str()))) {
                gateWiring(nowGate, input2);
            }
            else {
                input2 = new Gate;
                input2->name = port["Y"];
                gateWiring(nowGate, input2);
                wirePool[input2->name] = input2;
            }
        }
        else {
            port[lp[6]] = lp[7];
            nowGate->type = (!strcmp("NOR2", lp[0]) ? NOR : NAND);
            if ((input2 = findGateByName(port["B"].c_str()))) {
                gateWiring(input2, nowGate);
            }
            else {
                input2 = new Gate;
                input2->name = port["B"];
                gateWiring(input2, nowGate);
                wirePool[input2->name] = input2;
            }
            if ((output = findGateByName(port["Y"].c_str()))) {
                gateWiring(nowGate, output);
            }
            else {
                output = new Gate;
                output->name = port["Y"];
                gateWiring(nowGate, output);
                wirePool[output->name] = output;
            }
        }
    }

    // rewiring
    for ( auto& eachWire : wirePool ){
        eachWire.second->fan_in.front()->fan_out = eachWire.second->fan_out;
        for ( Gate* wireEachFanout : eachWire.second->fan_out ){
            for (GateList::iterator wire_out_in_it = wireEachFanout->fan_in.begin();
                    wire_out_in_it != wireEachFanout->fan_in.end(); ++wire_out_in_it){
                if (eachWire.second == (*wire_out_in_it)){
                    (*wire_out_in_it) = eachWire.second->fan_in.front();
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

void Network::resetAllValueAndTime(size_t pid) {
    for (auto &it : gatePool)
        it.second->value[pid] = it.second->arrival_time[pid] = -1;
}

GateSet Network::findAssociatePI(Gate* in) {
    GateSet PISet;
    GateList Queue;
    Queue.push_back(in);
    while( Queue.size() ){
        for ( Gate* gate : Queue.front()->fan_in )
            if (!gate->hasTrav && gate != &start) {
                Queue.push_back(gate);
            }
        if ( Queue.front()->type == INPUT )
            PISet.insert(Queue.front());
        Queue.front()->hasTrav = true;
        Queue.pop_front();
    }
    return PISet;
}

void Network::findAllPath() {
    Path path;
    path.reserve(1000);
    for (Gate* inputport : start.fan_out) {
        path.push_back(inputport);
        while (path.size()) {
            if (path.back()->fan_out_it == path.back()->fan_out.end()) {
                if (path.back()->type == OUTPUT) {
                    if (path.size() - 2ul > minimun) {
                        IOMap[path.back()].insert(path.front());
                        paths.push_back(new Path(path));
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
    for (auto map_it = IOMap.begin(); map_it != IOMap.end(); ++map_it) {
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
    for (Path* path : paths)
        printContainer(*path), cout << endl;
}

// Generate a sequence for evaluate network value without START GATE
void Network::topologySort() {
    GateList stack;
    stack.push_back(&start);
    start.hasTrav = true;
    while (stack.size()) {
        if (stack.back()->fan_out_it == stack.back()->fan_out.end()) {
            evalSequence.push_front(stack.back());
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

void Network::random2Shrink(size_t pid){
    randomInput(pid);
    evalNetwork(pid);
    checkAllPathNowIsTruePath(pid);
}

void Network::exhaustiveMethod() {
    for (auto PI : start.fan_out)
        PI->value[0] = 0;
    int overflow = 0;
    while (overflow != 2) {
        evalNetwork(0);
        checkAllPathNowIsTruePath(0);
        bool flag1 = start.fan_out.back()->value[0];
        for (auto PI : start.fan_out) {
            PI->value[0] = !PI->value[0];
            if (PI->value[0])
                break;
        }
    if (flag1 != start.fan_out.back()->value[0])
        ++overflow;
    }
}

bool Network::nextPIPattern(size_t pid,
                            GateList::iterator first,
                            GateList::iterator last, int *overflow)
{
    auto back = (*prev(last));
    bool flag = back->value[pid];
    for (;first != last; ++first) {
        (*first)->value[pid] = !(*first)->value[pid];
        if ((*first)->value[pid])
        break;
    }
    if (flag != back->value[pid])
        ++(*overflow);
    return *overflow != 2;
}

void Network::checkAllPathNowIsTruePath(size_t pid) {
    for (Path* path : paths) {
        short type = path->front()->value[pid];
        if (path->isFind[type])
            continue;
        if (isTruePath(pid, *path)==1) {
            bool Print = false;
            mutex.lock();
            Print = !path->isFind[type];
            path->isFind[type] = true;
            if (Print) {
                output_format(pid, *path);
            }
            mutex.unlock();
        }
    }
}

int Network::isTruePath(size_t pid, Path &path) {
    int ret;
    Gate* me = path.front();
    for (Gate* curGate : path) {
        Gate* you;
        if (curGate->type == NAND || curGate->type == NOR) {
            if (me == curGate->fan_in.front()){
                you = curGate->fan_in.back();
            } else {
                you = curGate->fan_in.front();
                me = curGate->fan_in.back();
            }
            if ((ret = subIsTruePath(pid, curGate, me, you)) != 1)
                return ret;
        }
        me = curGate;
    }
    return true;
}

// define both (you & me)'s arrival time == -1, means it's false path
int Network::subIsTruePath(size_t pid, Gate* curGate, Gate* me, Gate* you) {
    int isTruePath = 1;
    if (you->value[pid] == curGate->ctrlValue() &&
        me->value[pid] != -1 && me->value[pid] != curGate->ctrlValue()) {
        return 0;
    }
    if (curGate->arrival_time[pid] == -1){
       isTruePath = -1;
    }
    else if (curGate->type == NAND) {
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
    else if (curGate->type == NOR){
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
    Gate* me = path.front();
    for (auto gate : gatePool) {
        gate.second->hasTrav = false;
    }
    path.front()->hasTrav = true;
    path.PISequence.push_back(path.front());
    int count = -1;
    // put critical gate to trySeq
    for (auto it = path.begin(); it != path.end(); ++it, ++count) {
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
            if (you->first_in > count || you->last_in < count) {
                GateSet set = findAssociatePI(*it);

                if (you->first_in > count) {
                    if ( me->criticalValue == -1 ){
                        me->criticalValue = (*it)->ctrlValue();
                        backwardImplication( path , me );
                        path.criticList.push_back({me, (*it)->ctrlValue()});
                        path.isFind[count % 2 ? me->criticalValue : !me->criticalValue]
                        = true;
                    }
                    else if (me->criticalValue != (*it)->ctrlValue()) {
                        path.isFind[0] = path.isFind[1] = true;
                        break;
                    }
                } else {
                    if ( you->criticalValue == -1 ){
                        you->criticalValue = !(*it)->ctrlValue();
                        backwardImplication( path , you );
                        path.criticList.push_back({you, !(*it)->ctrlValue()});
                    }
                    else if (you->criticalValue != !(*it)->ctrlValue()) {
                        path.isFind[0] = path.isFind[1] = true;
                        break;
                    }
                }
                for (auto gate : set) {
                    path.PISequence.push_back(gate);
                }
            }
        }
        me = *it;
    }
    // reset criticalValue
    for (auto gate : path.criticList) {
        gate.first->criticalValue = -1;
    }
    //add AccosiateSeq
    GateSet acSet = findAssociatePI(path.back());
    for (auto gate : acSet) {
        path.PISequence.push_back(gate);
    }
}

void Network::genAllPISequence() {
    for (Path* path : paths){
        genPISequence(*path);
    }
    std::sort(paths.begin(), paths.end(), [](const Path *a, const Path *b)
    { return a->PISequence.size() < b->PISequence.size(); });
}

// test to print all the gate value
void Network::test2PrintGateValue(size_t pid) {
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
Gate* Network::isReady(size_t pid, Gate* out) {
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
void Network::evalNetwork(size_t pid) {
    for (Gate* gate : evalSequence) {
        gate->eval(pid);
        gate->arrival_time[pid] = gate->trueinput(pid)->arrival_time[pid] + 1;
    }
}

// random test pattern
void Network::randomInput(size_t pid) {
    for (Gate* inputport : start.fan_out) {
        inputport->value[pid] = rand() % 2;
    }
}

void Network::parallelRandomMethod() {
    std::thread threads[ThreadNumber];
    for (size_t i = 0; i < ThreadNumber; ++i) {
        threads[i] = std::thread(&Network::findPatternTruePath, this, i);
    }
    for (size_t i = 0; i < ThreadNumber; ++i) {
        threads[i].join();
    }
}

void Network::parallelBranchAndBound() {
    std::thread threads[ThreadNumber];
    for (size_t i = 0; i < ThreadNumber; ++i) {
        threads[i] = std::thread(&Network::branchAndBoundThreading, this, i);
    }
    for (size_t i = 0; i < ThreadNumber; ++i) {
        threads[i].join();
    }
}

void Network::parallelExhaustiveMethod() {
    std::thread threads[ThreadNumber];
    int overflow[ThreadNumber];
    for (size_t i = 0; i < ThreadNumber; ++i)
        overflow[i] = 0;
    for (Gate* PI : start.fan_out) {
        for (size_t i = 0; i < ThreadNumber; ++i)
            PI->value[i] = 0;
    }
    int offset = log(ThreadNumber);
    for (size_t i = 0; i < ThreadNumber; ++i) {
        for (int j = 0; j < i; j++) {
            nextPIPattern(i, start.fan_out.begin(),
            next(start.fan_out.begin(), offset), &overflow[i]);
        }
        threads[i] = std::thread(&Network::ExhaustiveMethodThreading, this, i);
    }
    for (size_t i = 0; i < ThreadNumber; ++i) {
        threads[i].join();
    }
}

void Network::ExhaustiveMethodThreading(size_t pid) {
    int offset = log(ThreadNumber);
    auto sep = next(start.fan_out.begin(), offset);
    bool next;
    int overflow = 0;
    do {
        evalNetwork(pid);
        checkAllPathNowIsTruePath(pid);
        next = nextPIPattern(pid, sep, start.fan_out.end(), &overflow);
    } while (next);
}

void Network::branchAndBoundThreading(size_t pid) {
    for (size_t i = pid; i < paths.size(); i += ThreadNumber) {
        branchAndBoundOnePath(pid, *paths[i]);
    }
}

void Network::findPatternTruePath(size_t pid) {
    for(int i = 0; i < 100000; ++i) {
        if (pathCounter >= paths.size() * 2)
            break;
        random2Shrink(pid);
    }
}

void Network::branchAndBoundOnePath(size_t pid, Path &path) {
    mutex.lock();
    cout << ++pathCounter << endl;
    mutex.unlock();
    ModifyList modifyList;
    path.PISequence.front()->arrival_time[pid] = 0;
    if (!path.isFind[0]) {
        path.PISequence.front()->value[pid] = 0;
        for (Gate *nowPIFanOut : path.PISequence.front()->fan_out) {
            forwardSimulation(pid, nowPIFanOut, modifyList);
        }
        branchAndBound(pid, path, ++path.PISequence.begin());
        resetLastStatusWithModifyList(pid, modifyList);
        modifyList.clear();
        branchCounter[pid] = 0;
    }
    if (!path.isFind[1]) {
        path.PISequence.front()->value[pid] = 1;
        for (Gate *nowPIFanOut : path.PISequence.front()->fan_out) {
            forwardSimulation(pid, nowPIFanOut, modifyList);
        }
        branchAndBound(pid, path, ++path.PISequence.begin());
        resetLastStatusWithModifyList(pid, modifyList);
        branchCounter[pid] = 0;
    }
    path.PISequence.front()->value[pid] = -1;
    path.PISequence.front()->arrival_time[pid] = -1;
}

bool Network::checkInverseValue(size_t pid, Path &path) {
    auto it = path.begin();
    int tt = 0;
    short ness = !(*it)->value[pid];
    for (std::advance(it, 1); tt != path.size() - 2; ++it, ++tt, ness = !ness) {
        if (~(*it)->value[pid] && ness != (*it)->value[pid])
            return false;
    }
    return true;
}

Network::~Network() {
    // release all Path
    for (Path* path : paths)
        delete path;

    // release all Gate
    for (auto &it : gatePool)
        delete it.second;
}

void Network::forwardSimulation(size_t pid , Gate* current, ModifyList &modifyList) {
    if (current->value[pid] != -1 &&
        current->arrival_time[pid] != -1)
        return;
    modifyList.emplace_front(
        std::make_tuple(current, current->arrival_time[pid], current->value[pid]));
    if ( current->type == NOT || current->type == OUTPUT ){
        current->value[pid] = (current->type == NOT)?
                        !current->fan_in.front()->value[pid] :
                        current->fan_in.front()->value[pid];
        int av = current->fan_in.front()->arrival_time[pid];
        if (~av)
            current->arrival_time[pid] = av + 1;
        for ( auto cur_fan_out : current->fan_out )
            forwardSimulation(pid, cur_fan_out, modifyList);
    } else if ( current->type == NOR || current->type == NAND ){
        int ctrlValue = ( current->type == NOR );
        // if has one ctrling val
        if ( current->fan_in.front()->value[pid] == ctrlValue ||
            current->fan_in.back()->value[pid] == ctrlValue ){
            current->value[pid] = !ctrlValue;
            int cav = current->ctrlinput(pid)->arrival_time[pid];
            if (~cav)
                current->arrival_time[pid] = cav + 1;
            for ( auto cur_fan_out : current->fan_out )
                forwardSimulation(pid, cur_fan_out, modifyList);
        }
        else if ( current->fan_in.front()->value[pid] == -1 ||
                current->fan_in.back()->value[pid] == -1 ) {
            modifyList.pop_front();
            return;
        }
        else {
            // both fanin has val but both not ctrling
            current->value[pid] = ctrlValue;
            int i1 = current->fan_in.front()->arrival_time[pid];
            int i2 = current->fan_in.back()->arrival_time[pid];
            if ( i1 != -1 && i2 != -1)
                current->arrival_time[pid] = std::max(i1, i2) + 1;

            for (auto cur_fan_out : current->fan_out)
                forwardSimulation(pid, cur_fan_out, modifyList);
        }
    }
}

// if find trupath => 1; bound => 0; default => -1
int Network::branchAndBound(size_t pid, Path &path, GateList::iterator pos) {
    ++branchCounter[pid];
    ModifyList modifyList;
    int type = path.front()->value[pid];
    int bound = -1;
    if (path.isFind[type])
        return 1;
    if (branchCounter[pid] > branchLimit)
        return 1;
    if (pos == path.PISequence.end()) {
        if (!path.isFind[type] && isTruePath(pid, path) == 1) {
            path.isFind[type] = true;
            mutex.lock();
            output_format(pid, path);
            mutex.unlock();
            checkAllPathNowIsTruePath(pid);
            return 1;
        }
        return -1;
    }
    if (criticalFalse(pid, path.criticList))
        return -1;
    if (!checkInverseValue(pid, path))
        return -1;
    if ((bound = isTruePath(pid, path)) != -1)
        return -1;

    (*pos)->arrival_time[pid] = 0;
    (*pos)->value[pid] = 0;
    if(bound == -1) {
        for (Gate *nowPIFanOut : (*pos)->fan_out) {
            forwardSimulation(pid, nowPIFanOut, modifyList);
        }
        bound = branchAndBound(pid, path, next(pos));
        resetLastStatusWithModifyList(pid, modifyList);
        modifyList.clear();
    }
    (*pos)->value[pid] = 1;
    if (bound == -1) {
        for (Gate *nowPIFanOut : (*pos)->fan_out) {
            forwardSimulation(pid, nowPIFanOut, modifyList);
        }
        bound = branchAndBound(pid, path, next(pos));
        resetLastStatusWithModifyList(pid, modifyList);
    }
    (*pos)->value[pid] = -1;
    (*pos)->arrival_time[pid] = -1;
    return bound;
}

bool Network::criticalFalse(size_t pid, CriticalList &criticList) {
    for (auto criGate : criticList) {
        if (~criGate.first->value[pid] &&
            criGate.first->value[pid] != criGate.second )
                return true;
    }
    return false;
}

void Network::resetLastStatusWithModifyList(size_t pid, ModifyList &modifyList) {
    using std::get;
    for (auto &curGate : modifyList) {
        get<0>(curGate)->arrival_time[pid] = get<1>(curGate);
        get<0>(curGate)->value[pid] = get<2>(curGate);
    }
}

bool Network::backwardIsConflict( Path& path , Gate* direction , Gate* cur ){
    if ( cur->type == NOT ){
        if ( direction->criticalValue != -1 &&
             direction->criticalValue != !cur->criticalValue ){
            path.isFind[0] = path.isFind[1] = true;
            return true;
        }
    }
    else if ( cur->type == NAND || cur->type == NOR ){
        int ctrlValue = ( cur->type == NOR );
        if ( cur->criticalValue == ctrlValue ){
            if ( direction->criticalValue != -1 &&
                 direction->criticalValue != !ctrlValue ){
                path.isFind[0] = path.isFind[1] = true;
                return true;
            }
        }
    }
    return false;
}

bool Network::forwardIsConflict( Path& path , Gate* direction ) {
    if ( direction->type == NOT ){
        if ( direction->criticalValue != -1 && direction->fan_in.front()->criticalValue == direction->criticalValue ){
            path.isFind[0] = path.isFind[1] = true;
            return true;
        }
    }

    int ctrlValue = ( direction->type == NOR );
    if ( direction->fan_in.front()->criticalValue == ctrlValue ||
            direction->fan_in.back()->criticalValue == ctrlValue ) {
        if ( direction->criticalValue != -1 && direction->criticalValue != !ctrlValue ){
            path.isFind[0] = path.isFind[1] = true;
            return true;
        }
    }
    if ( direction->fan_in.front()->criticalValue == !ctrlValue &&
            direction->fan_in.back()->criticalValue == !ctrlValue &&
            direction->fan_in.front()->criticalValue != -1 &&
            direction->fan_in.back()->criticalValue != -1 ) {
        if ( direction->criticalValue != -1 && direction->criticalValue != ctrlValue ){
            path.isFind[0] = path.isFind[1] = true;
            return true;
        }
    }
    return false;
}

void Network::backwardImplication(Path &path, Gate *cur) {
    if ( path.isFind[0] && path.isFind[1] )
        return;
    if ( cur->type == NOT ){
        if ( backwardIsConflict( path , cur->fan_in.front() , cur ) )
            return;
        else {
            if ( cur->fan_in.front()->criticalValue == -1 ){
                cur->fan_in.front()->criticalValue = !cur->criticalValue;
                path.criticList.push_back({cur->fan_in.front(), !cur->criticalValue});
                backwardImplication( path , cur->fan_in.front() );
            }
            // forwardImplication( path, cur );
        }
    }
    else if ( cur->type == NAND || cur->type == NOR ){
        if ( backwardIsConflict(path , cur->fan_in.front() , cur ) ||
             backwardIsConflict( path , cur->fan_in.back() , cur ))
            return;
        else{
            if ( cur->type == NOR && cur->criticalValue == 1 ){
                if ( cur->fan_in.front()->criticalValue == -1 ){
                    cur->fan_in.front()->criticalValue = 0;
                    path.criticList.push_back({cur->fan_in.front(),cur->fan_in.front()->criticalValue});
                    backwardImplication( path , cur->fan_in.front() );
                }

                if ( cur->fan_in.back()->criticalValue == -1 ){
                    cur->fan_in.back()->criticalValue = 0;
                    path.criticList.push_back({cur->fan_in.back(),cur->fan_in.back()->criticalValue});
                    backwardImplication( path , cur->fan_in.back() );
                }
            }
            else if ( cur->type == NOR && cur->criticalValue == 0 ){
                if ( cur->fan_in.front()->criticalValue == -1 && cur->fan_in.back()->criticalValue == 0 ){
                    cur->fan_in.front()->criticalValue = 1;
                    path.criticList.push_back({cur->fan_in.front(),cur->fan_in.front()->criticalValue});
                    backwardImplication( path , cur->fan_in.front() );
                }
                else if( cur->fan_in.front()->criticalValue ==0 && cur->fan_in.back()->criticalValue == -1 ){
                    cur->fan_in.back()->criticalValue = 1;
                    path.criticList.push_back({cur->fan_in.back(),cur->fan_in.back()->criticalValue});
                    backwardImplication( path , cur->fan_in.back() );
                }
            }

            if ( cur->type == NAND && cur->criticalValue == 0 ){
                if ( cur->fan_in.front()->criticalValue == -1 ){
                    cur->fan_in.front()->criticalValue = 1;
                    path.criticList.push_back({cur->fan_in.front(),cur->fan_in.front()->criticalValue});
                    backwardImplication( path , cur->fan_in.front() );
                }

                if ( cur->fan_in.back()->criticalValue == -1 ){
                    cur->fan_in.back()->criticalValue = 1;
                    path.criticList.push_back({cur->fan_in.back(),cur->fan_in.back()->criticalValue});
                    backwardImplication( path , cur->fan_in.back() );
                }
            }
            else if ( cur->type == NAND && cur->criticalValue == 1 ){
                if ( cur->fan_in.front()->criticalValue == -1 && cur->fan_in.back()->criticalValue == 1 ){
                    cur->fan_in.front()->criticalValue = 0;
                    path.criticList.push_back({cur->fan_in.front(),cur->fan_in.front()->criticalValue});
                    backwardImplication( path , cur->fan_in.front() );
                }
                else if( cur->fan_in.front()->criticalValue == 1 && cur->fan_in.back()->criticalValue == -1 ){
                    cur->fan_in.back()->criticalValue = 0;
                    path.criticList.push_back({cur->fan_in.back(),cur->fan_in.back()->criticalValue});
                    backwardImplication( path , cur->fan_in.back() );
                }
            }
        }
        // forwardImplication( path, cur );
    }
    else if ( cur->type == INPUT ){
        ;
        //  forwardImplication( path, cur );
    }
}

void Network::forwardImplication(Path &path, Gate *cur){
    if ( path.isFind[0] && path.isFind[1] )
        return;
    for ( auto fan_out : cur->fan_out ){
        if ( fan_out->type == NOT ){
            if ( forwardIsConflict( path , fan_out ) )
                return;
            else{
                if ( fan_out->criticalValue == -1 ){
                    fan_out->criticalValue = !cur->criticalValue;
                    path.criticList.push_back({fan_out,fan_out->criticalValue});
                    forwardImplication( path , fan_out );
                }
            }
        }
        else if ( fan_out->type == NAND || fan_out->type == NOR ){
            if ( forwardIsConflict( path, fan_out ) )
                return;
            else{
                if ( fan_out->type == NOR && fan_out->criticalValue == -1 ){
                    if ( fan_out->fan_in.front()->criticalValue == 0 &&
                         fan_out->fan_in.back()->criticalValue == 0 ){
                        fan_out->criticalValue = 1;
                        path.criticList.push_back({fan_out, fan_out->criticalValue});
                        forwardImplication( path , fan_out );
                    }
                    else if ( fan_out->fan_in.front()->criticalValue == 1 ||
                              fan_out->fan_in.back()->criticalValue == 1 ){
                        fan_out->criticalValue = 0;
                        path.criticList.push_back({fan_out, fan_out->criticalValue});
                        forwardImplication( path , fan_out );
                    }
                }
                if ( fan_out->type == NAND && fan_out->criticalValue == -1 ){
                    if ( fan_out->fan_in.front()->criticalValue == 1 &&
                         fan_out->fan_in.back()->criticalValue == 1 ){
                        fan_out->criticalValue = 0;
                        path.criticList.push_back({fan_out, fan_out->criticalValue});
                        forwardImplication( path , fan_out );
                    }
                    else if ( fan_out->fan_in.front()->criticalValue == 0 ||
                              fan_out->fan_in.back()->criticalValue == 0 ){
                        fan_out->criticalValue = 1;
                        path.criticList.push_back({fan_out, fan_out->criticalValue});
                        forwardImplication( path , fan_out );
                    }
                }
            }
        }
        if (fan_out->criticalValue != -1)
            backwardImplication( path , fan_out );
    }
}
