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
using std::map;
using std::list;
using std::string;
using std::getline;
using std::cin;
using std::cout;
using std::endl;

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
    Gate() : type(NONE), value(-1), arrival_time(-1) {}
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
    list<GateList> paths;
    Network() {
        start.name = "start";
        end.name = "end";
	start.arrival_time = 0;
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
            newGate->fan_out.push_back(&end);
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

    void Dfs() {
        GateList path;
        path.push_back(&start);
        while (path.size()) {
            if (path.back()->fan_out_it == path.back()->fan_out.end()) {
                if (path.back()->name == "end") {
                    paths.push_back(path);
                    // printContainer(path);
                    // cout << endl << endl;
                }
                path.back()->fan_out_it = path.back()->fan_out.begin();
                path.pop_back();
            } else {
                	path.push_back(*(path.back()->fan_out_it++));
            }
        }
    }

    void random2Shrink(){
	    // random test pattern
	    /**************************************************************************************/
	    srand(time(0));
	    for ( GateList::iterator it = start.fan_out.begin() ; it != start.fan_out.end() ; ++it )
		(*it)->value = rand() % 2;
	    /**************************************************************************************/
		
	    // dfs with the status Q
	    /**************************************************************************************/
	    list<GateList> status;
	    GateList path;
	    path.push_back(&start);
	    status.push_back(path);
	    while( status.size() ){
		    path.clear();
		    path = status.front();
//		    cout << "cur: \n";
//		    for ( list<GateList>::iterator it = status.begin() ; it != status.end() ; ++it ){
//		    	printContainer(*it);
//		    	cout << endl;
//		    }
//		    cout << endl;
		    
		    bool unready = false;
		    for ( GateList::iterator it = path.back()->fan_out.begin() ; it != path.back()->fan_out.end() ; ++it ){
			    if ( (*it)->arrival_time == -1 ){
//				if ( (*it)->name == "w"  )
//					cout << "123\n";
				    int which = isReady(*it);
			    	if ( which == 1 )
					(*it)->arrival_time = (*it)->fan_in.front()->arrival_time + 1;
			    	else if ( which == 2 || which == 0 )
					(*it)->arrival_time = (*it)->fan_in.back()->arrival_time + 1;
			    	else {		
					unready = true;
					continue;
			    	}
			    }
			    	path.push_back( *it );	    
		    	    	status.push_back( path );
			    	path.pop_back();
			    
		    }
		    //if ( unready )
		//	    status.push_back( status.front() );
		    status.pop_front();
	    }
	    /**************************************************************************************/ 
    }
    // 1 = one fanin or fanin1 is first , 2 = fanin2 is first , 0 = arrive at the same time , -1 = unready
    int isReady( Gate* out ){
		

	   if ( out->fan_in.size() == 1 )
		   return 1;
	   else if ( out->fan_in.size() == 2 ){
	   	GateList::iterator it1 = out->fan_in.begin();
		GateList::iterator it2 = ++it1;
//				if ( out->name == "w" ){
//					cout << out->fan_in.size() << endl;
//					printContainer(out->fan_in);
//					cout << endl;
//					//				
////					cout << (*it1)->name << " " << (*it1)->arrival_time << endl;
////					cout << (*it2)->name << " " << (*it2)->arrival_time << endl;
//				
//				}
		if ( (*it1)->arrival_time != -1 && (*it2)->arrival_time != -1 ){
			if ( (*it1)->arrival_time > (*it2)->arrival_time ) 
				return 1;
			else if ( (*it1)->arrival_time == (*it2)->arrival_time )
				return 0;
			else 
				return 2;
		} 
	   }
	   return -1;
    }

    ~Network() {
        for (GateMap::iterator it = gatePool.begin(); it != gatePool.end();
             ++it) {
            delete it->second;
        }
    }
};
#endif
