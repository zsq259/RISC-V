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
}
hst::cabbage_cpu T;

int main() {
    T.work();
    
    return 0;
}