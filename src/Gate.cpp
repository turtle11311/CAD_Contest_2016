#include"Gate.h"

Gate::Gate(std::string name, GateType type)
    : name(name), type(type), value({-1,-1,-1,-1}), arrival_time({0,0,0,0}),
      hasTrav(false) {}

    void Gate::eval(int pid){
        if (type == NOT)
            value[pid] = !fan_in.front()->value[pid];
        else if (type == NAND)
            value[pid] = (!fan_in.front()->value[pid]) || (!fan_in.back()->value[pid]);
        else if (type == NOR)
            value[pid] = (!fan_in.front()->value[pid]) && (!fan_in.back()->value[pid]);
        else if (type == OUTPUT)
            value[pid] = fan_in.front()->value[pid];
    }
Gate* Gate::trueinput(int pid) {
    Gate *ret = NULL;
    Gate *A = fan_in.front();
    Gate *B = fan_in.back();
    switch (type) {
        case NAND:
            if ((!A->value[pid] && A->arrival_time[pid] <= B->arrival_time[pid]) ||
                (A->value[pid] && B->value[pid] && A->arrival_time[pid] >= B->arrival_time[pid]) ||
                (!A->value[pid] && B->value[pid]))
                ret = A;
            else
                ret = B;
            break;
        case NOR:
            if ((A->value[pid] && A->arrival_time[pid] <= B->arrival_time[pid]) ||
                (!A->value[pid] && !B->value[pid] && A->arrival_time[pid] >= B->arrival_time[pid]) ||
                (A->value[pid] && !B->value[pid]))
                ret = A;
            else
                ret = B;
            break;
        default:
            ret = A;
            break;
    }
    return ret;
}
