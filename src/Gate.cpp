#include "Gate.h"
#include <functional>

Gate::Gate(std::string &&name, GateType type)
    : name(std::move(name)), type(type),
      hasTrav(false) , first_in(0) , last_in(0) {
    for (size_t i = 0; i < ThreadNumber; ++i)
        value[i] = arrival_time[i] = -1;
}

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


Gate* Gate::ctrlinput(int pid) {
    Gate *ret = NULL;
    Gate *A = fan_in.front();
    Gate *B = fan_in.back();
    if ((A->arrival_time[pid] == -1) ||
        (A->value[pid] == ctrlValue() && B->arrival_time[pid] != -1 && B->value[pid] != ctrlValue()) ||
        (A->value[pid] == ctrlValue() && A->arrival_time[pid] < B->arrival_time[pid]))
        ret = A;
    else
        ret = B;
    return ret;
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
