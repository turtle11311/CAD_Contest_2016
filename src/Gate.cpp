#include"Gate.h"

Gate::Gate(std::string name, GateType type)
    : name(name), type(type), value(-1), arrival_time(0), hasTrav(false),
    true_fan_in(NULL) {}


    void Gate::eval(){
        if (type == NOT)
            value = !fan_in.front()->value;
        else if (type == NAND)
            value = (!fan_in.front()->value) || (!fan_in.back()->value);
        else if (type == NOR)
            value = (!fan_in.front()->value) && (!fan_in.back()->value);
        else if (type == OUTPUT)
            value = fan_in.front()->value;
    }
Gate* Gate:: trueinput() {
    Gate *ret = NULL;
    Gate *A = fan_in.front();
    Gate *B = fan_in.back();
    switch (type) {
        case NAND:
            if ((!A->value && A->arrival_time <= B->arrival_time) ||
                (A->value && B->value && A->arrival_time >= B->arrival_time) ||
                (!A->value && B->value))
                ret = A;
            else
                ret = B;
            break;
        case NOR:
            if ((A->value && A->arrival_time <= B->arrival_time) ||
                (!A->value && !B->value && A->arrival_time >= B->arrival_time) ||
                (A->value && !B->value))
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
