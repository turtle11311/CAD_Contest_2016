#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
using namespace std;

// int TopoCount = 0;
struct Gate {

    int id;
    string name;
    int func_type; // ip = 1, op , not , nand , nor , wire;
    int type;      // ip = 1, op, internal, wire
    vector<Gate *> fanout_Ary;
    Gate *fanin1;
    Gate *fanin2;
    int value;
    int topo;
};

struct Network {

    vector<Gate *> Gate_Ary;
    vector<Gate *> PO_Ary;
    vector<Gate *> PI_Ary;
};

void GetInputData(Network &network);
// int gatetype , Network network ,  vector<char*> data
void SetNotGate(int, Network &, vector<char *> &);
// int index , data's index, means the fanin/out gate name ,  Network network ,
// vector<char*> data
void SetGateIp(int, Network &, vector<char *> &);
void SetGateOp(int, Network &, vector<char *> &);
void SetNorNandGate(int, Network &, vector<char *> &);
// void Dfs(Gate*, Network , stack<Gate*> , vector<vector<Gate*>>& );
void Dfs(Gate *, stack<Gate *> &, vector<stack<Gate *> > &);
void IterativeDfs(Network, vector<vector<Gate *> > &);
void CheckTruePath(bool *, vector<stack<Gate *> >);
string int2str(int);
void Print(Network);
// void subPrint( Gate* );
void PrintPath(vector<stack<Gate *> >);
// convert a char to integer
int A2I(char *);
// c = a ^ b -> Power( a , b ) return c
int Power(int, int);

int main() {

    Network network;
    GetInputData(network);

    // vector<stack<Gate*>> Paths;
    // stack<Gate*> PathStack;
    // for (int i = 0; i < network.PO_Ary.size(); i++)
    //	Dfs(network.PO_Ary[i], PathStack, Paths);
    vector<vector<Gate *> > Paths;
    IterativeDfs(network, Paths);

    // PrintPath(Paths);
    return 0;
}

void GetInputData(Network &network) {

    ifstream inFile;
    vector<char *> data;
    char test[256];
    string temp;
    inFile.open("test.txt", ios::in | ios::binary);

    while (!inFile.eof()) {
        while (inFile.getline(test, sizeof(test), '\r')) {
            char *cstr;
            temp = test;
            char *context = NULL;
            char delims[] = " (),;\t\0";
            cstr = strtok(test, delims);
            int num = 0;
            while (cstr != NULL) {
                char *buf = new char[temp.size() + 1];
                for (int i = 0; i < strlen(cstr); i++) {
                    if (isdigit(cstr[i]) | isalpha(cstr[i]) | cstr[i] == ':' |
                        cstr[i] == '[' | cstr[i] == ']') {
                        buf[num] = cstr[i];
                        num++;
                    }
                }
                buf[num] = '\0';
                num = 0;
                if (strlen(buf) != 0)
                    data.push_back(buf);
                cstr = strtok(NULL, delims);
            }
            if (data.size() != 0) {
                if (strcmp(data[0], "output") == 0) {

                    char dbuffer[10];
                    strcpy(dbuffer, data[1]);

                    char *sBuffer = strtok(dbuffer, "[]:");
                    int OpSize = A2I(sBuffer) + 1;

                    for (int i = 0; i < OpSize; i++) {

                        Gate *gbuffer = new Gate();
                        gbuffer->id = i + 1;
                        gbuffer->name =
                            string(data[2]) + "[" + int2str(i) + "]";
                        gbuffer->func_type = 2;
                        gbuffer->type = 2;
                        network.PO_Ary.push_back(gbuffer);
                    }
                }

                else if (strcmp(data[0], "input") == 0) {

                    char dbuffer[10];
                    strcpy(dbuffer, data[1]);

                    char *sBuffer = strtok(dbuffer, "[]:");

                    if (strcmp(data[2], "A") == 0) {

                        int IpASize = A2I(sBuffer) + 1;
                        for (int i = 0; i < IpASize; i++) {

                            Gate *gbuffer = new Gate();
                            gbuffer->id = network.PI_Ary.size();
                            gbuffer->name =
                                string(data[2]) + "[" + int2str(i) + "]";
                            gbuffer->func_type = 1;
                            gbuffer->type = 1;
                            network.PI_Ary.push_back(gbuffer);
                        }
                    }

                    else if (strcmp(data[2], "B") == 0) {

                        int IpBSize = A2I(sBuffer) + 1;
                        for (int i = 0; i < IpBSize; i++) {

                            Gate *gbuffer = new Gate();
                            gbuffer->id = network.PI_Ary.size();
                            gbuffer->name =
                                string(data[2]) + "[" + int2str(i) + "]";
                            gbuffer->func_type = 1;
                            gbuffer->type = 1;
                            network.PI_Ary.push_back(gbuffer);
                        }
                    }
                }

                if (strcmp(data[0], "wire") == 0) {

                    for (int i = 0; i < data.size() - 1; i++) {

                        Gate *gbuffer = new Gate();
                        gbuffer->id = network.Gate_Ary.size();
                        gbuffer->name = string(data[i + 1]);
                        gbuffer->func_type = 6;
                        gbuffer->type = 4;
                        network.Gate_Ary.push_back(gbuffer);
                    }
                }

                // set NOT Gate
                if (strcmp(data[0], "NOT1") == 0)
                    SetNotGate(3, network, data);

                // set NOR Gate
                else if (strcmp(data[0], "NOR2") == 0)
                    SetNorNandGate(4, network, data);

                // set Nand Gate
                else if (strcmp(data[0], "NAND2") == 0)
                    SetNorNandGate(5, network, data);
            }
            data.resize(0);
        }
    }
}

void SetNotGate(int GateType, Network &network, vector<char *> &data) {

    Gate *buffer = new Gate;
    (*buffer).id = network.Gate_Ary.size();
    (*buffer).name = string(data[1]);
    (*buffer).func_type = GateType;
    network.Gate_Ary.push_back(buffer);

    // set in1
    SetGateIp(3, network, data);

    // set out1
    SetGateOp(5, network, data);
}

string int2str(int i) {

    stringstream ss;
    ss << i;
    return ss.str();
}

void SetGateIp(int Index, Network &network, vector<char *> &data) {

    // set ip
    for (int i = 0; i < network.Gate_Ary.size(); i++) {
        if (strcmp(data[Index], network.Gate_Ary[i]->name.c_str()) == 0) {

            if (network.Gate_Ary.back()->fanin1 == NULL) {
                network.Gate_Ary.back()->fanin1 = network.Gate_Ary[i];
                network.Gate_Ary[i]->fanout_Ary.push_back(
                    network.Gate_Ary.back());
            } else {
                network.Gate_Ary.back()->fanin2 = network.Gate_Ary[i];
                network.Gate_Ary[i]->fanout_Ary.push_back(
                    network.Gate_Ary.back());
            }
            return;
        }
    }

    for (int i = 0; i < network.PI_Ary.size(); i++) {
        if (strcmp(data[Index], network.PI_Ary[i]->name.c_str()) == 0) {
            if (!network.Gate_Ary.back()->fanin1) {
                network.Gate_Ary.back()->fanin1 = network.PI_Ary[i];
                network.PI_Ary[i]->fanout_Ary.push_back(
                    network.Gate_Ary.back());
            } else {
                network.Gate_Ary.back()->fanin2 = network.PI_Ary[i];
                network.PI_Ary[i]->fanout_Ary.push_back(
                    network.Gate_Ary.back());
            }
            return;
        }
    }
}

void SetGateOp(int Index, Network &network, vector<char *> &data) {

    // set op , op gatetype is node or wire
    for (int i = 0; i < network.Gate_Ary.size(); i++) {
        if (strcmp(data[Index], network.Gate_Ary[i]->name.c_str()) == 0) {

            if (network.Gate_Ary[i]->fanin1 == NULL) {
                network.Gate_Ary[i]->fanin1 = network.Gate_Ary.back();
                network.Gate_Ary.back()->fanout_Ary.push_back(
                    network.Gate_Ary[i]);
            }

            else {
                network.Gate_Ary[i]->fanin2 = network.Gate_Ary.back();
                network.Gate_Ary.back()->fanout_Ary.push_back(
                    network.Gate_Ary[i]);
            }
            return;
        }
    }
    // set op, op gatetype is PO_Ary
    for (int i = 0; i < network.PO_Ary.size(); i++) {
        if (strcmp(data[Index], network.PO_Ary[i]->name.c_str()) == 0) {
            network.PO_Ary[i]->fanin1 = network.Gate_Ary.back();
            network.Gate_Ary.back()->fanout_Ary.push_back(network.PO_Ary[i]);
            return;
        }
    }
}

void SetNorNandGate(int GateType, Network &network, vector<char *> &data) {

    Gate *buffer = new Gate;
    (*buffer).id = network.Gate_Ary.size();
    (*buffer).name = string(data[1]);
    (*buffer).func_type = GateType;
    network.Gate_Ary.push_back(buffer);

    // set in1
    SetGateIp(3, network, data);

    // set in2
    SetGateIp(5, network, data);

    // set out1
    SetGateOp(7, network, data);
}

// void Dfs( Gate* ptr, Network network , stack<Gate*> PathStack,
// vector<vector<Gate*>>& Paths ){
//
//	stack<Gate*> stack;
//	stack.push(ptr);
//	while (!stack.empty()){
//
//		for ( Gate* subptr = ptr ; subptr->fanin1 != NULL ; subptr =
//subptr->fanin1 ){
//
//
//
//
//		}
//	}
//
//
//}

void CheckTruePath(bool *, vector<stack<Gate *> > Paths) {}

void Print(Network network) {

    cout << "input:\n";
    for (int i = 0; i < network.PI_Ary.size(); i++) {

        cout << "id = " << network.PI_Ary[i]->id << endl;
        cout << "name = " << network.PI_Ary[i]->name << endl;
        if (network.PI_Ary[i]->fanin1 != NULL) {
            cout << "fanin1 = " << network.PI_Ary[i]->fanin1->name << endl;
            if (network.PI_Ary[i]->fanin2)
                cout << "fanin2 = " << network.PI_Ary[i]->fanin2->name << endl;
        }
        cout << "fanout = ";
        for (int j = 0; j < network.PI_Ary[i]->fanout_Ary.size(); j++) {
            cout << network.PI_Ary[i]->fanout_Ary[j]->name << ", ";
        }
        cout << endl;
    }

    getchar();
    cout << "output:\n";
    for (int i = 0; i < network.PO_Ary.size(); i++) {

        cout << "id = " << network.PO_Ary[i]->id << endl;
        cout << "name = " << network.PO_Ary[i]->name << endl;
        if (network.PO_Ary[i]->fanin1 != NULL) {
            cout << "fanin1 = " << network.PO_Ary[i]->fanin1->name << endl;
            if (network.PO_Ary[i]->fanin2)
                cout << "fanin2 = " << network.PO_Ary[i]->fanin2->name << endl;
        }
        cout << "fanout = ";
        for (int j = 0; j < network.PO_Ary[i]->fanout_Ary.size(); j++) {
            cout << network.PO_Ary[i]->fanout_Ary[j]->name << ", ";
        }
        cout << endl;
    }

    getchar();
    cout << "Gate:\n";
    for (int i = 0; i < network.Gate_Ary.size(); i++) {

        cout << "id = " << network.Gate_Ary[i]->id << endl;
        cout << "name = " << network.Gate_Ary[i]->name << endl;
        if (network.Gate_Ary[i]->fanin1 != NULL) {
            cout << "fanin1 = " << network.Gate_Ary[i]->fanin1->name << endl;
            if (network.Gate_Ary[i]->fanin2)
                cout << "fanin2 = " << network.Gate_Ary[i]->fanin2->name
                     << endl;
        }
        cout << "fanout = ";
        for (int j = 0; j < network.Gate_Ary[i]->fanout_Ary.size(); j++) {
            cout << network.Gate_Ary[i]->fanout_Ary[j]->name << ", ";
        }
        cout << endl;
    }
    getchar();
}

// void subPrint( Gate* ptr ){
//
//	cout << "I am ";
//	cout << (*ptr).name << endl;
//}

void PrintPath(vector<stack<Gate *> > Paths) {

    for (int i = 0; i < Paths.size(); i++) {
        while (!Paths[i].empty()) {
            cout << Paths[i].top()->name;
            if (Paths[i].size() != 1)
                cout << " -> ";
            Paths[i].pop();
        }
        cout << endl << endl;
    }
}

void Dfs(Gate *ptr, stack<Gate *> &PathStack, vector<stack<Gate *> > &Paths) {

    if (ptr) {
        // subPrint( ptr );
        PathStack.push(ptr);
        if (ptr->func_type == 1) {
            Paths.push_back(PathStack);
            PathStack.pop();
            return;
        }

        Dfs(ptr->fanin1, PathStack, Paths);
        if (ptr->func_type != 3 && ptr->func_type != 2 && ptr->func_type != 6)
            Dfs(ptr->fanin2, PathStack, Paths);
    }
    PathStack.pop();
}

void IterativeDfs(Network network, vector<vector<Gate *> > &Paths) {

    vector<Gate *> stack;
    for (int i = 0; i < network.PO_Ary.size(); i++) {
    }
}

int A2I(char *a) {

    int Integer = 0;
    int size = strlen(a);
    for (int i = 0; i < size; i++) {
        Integer += (a[i] - 48) * Power(10, size - 1 - i);
    }
    return Integer;
}

int Power(int a, int exponent) {
    int temp = 1;
    for (int i = 0; i < exponent; i++) {
        temp *= a;
    }
    return temp;
}
