#include "parser.h"
#include "cpu.h"
#include "memory.h"
#include <bitset>

namespace hst {
    Memory Mem;
    Register Reg;
    Bus Bus_;
    ReservationStation RS_;
    LoadStoreBuffer LSB_;
    ReorderBuffer RoB_;
    int Predictor::sum = 0;
    int Predictor::success = 0;
}
hst::cabbage_cpu T;

int main() {
    T.work();
    
    return 0;
}