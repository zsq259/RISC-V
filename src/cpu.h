#ifndef RISC_V_CPU_H
#define RISC_V_CPU_H

#include "parser.h"
#include "memory.h"
#include <memory>
#include <bitset>

namespace hst {

inline signed int sext(unsigned int x, int n) { 
    return x >> (n - 1) ? x | (0xffffffff >> n << n) : x;
}

class Register {
public:
    static unsigned int x[32];
    static unsigned int pc;
};

class ALU {
private:
    Register reg;
    Memory m;
public:
    int work(std::unique_ptr<instruction> o) {
        //o->print();
        // if (1) {
        //     puts("begin");
        //     printf("rs1=%d rs2=%d rd=%d\n",reg.x[o->get_rs1()], reg.x[o->get_rs2()], reg.x[o->get_rd()]);
            
        //     unsigned bb = sext(o->get_imm(), 12);
        //     std::bitset<32>mm(bb);
        //     std::cout <<"mm=" <<mm << " bb=" << bb <<'\n';
        // }
        int flag =  1;
        switch (o->op) {
            case 0: { reg.x[o->get_rd()] = (signed int)(o->get_imm() << 12); } break;
            case 1: { reg.x[o->get_rd()] = reg.pc + (signed int)(o->get_imm() << 12); } break;
            case 2: { flag = 0; reg.x[o->get_rd()] = reg.pc + 4; reg.pc += sext(o->get_imm(), 21); } break;
            case 3: { flag = 0; unsigned t = reg.pc + 4; reg.pc = (reg.x[o->get_rs1()] + sext(o->get_imm(), 12)) & ~1; reg.x[o->get_rd()] = t; } break;
            case 4: { if(reg.x[o->get_rs1()] == reg.x[o->get_rs2()]) flag = 0, reg.pc += sext(o->get_imm(), 13); } break;
            case 5: { if(reg.x[o->get_rs1()] != reg.x[o->get_rs2()]) flag = 0, reg.pc += sext(o->get_imm(), 13); } break;
            case 6: { if((signed)reg.x[o->get_rs1()] < (signed)reg.x[o->get_rs2()]) flag = 0, reg.pc += sext(o->get_imm(), 13); } break;
            case 7: { if((signed)reg.x[o->get_rs1()] >= (signed)reg.x[o->get_rs2()]) flag = 0, reg.pc += sext(o->get_imm(), 13); } break;
            case 8: { if(reg.x[o->get_rs1()] < reg.x[o->get_rs2()]) flag = 0, reg.pc += sext(o->get_imm(), 13); } break;
            case 9: { if(reg.x[o->get_rs1()] >= reg.x[o->get_rs2()]) flag = 0, reg.pc += sext(o->get_imm(), 13); } break;
            case 10: { reg.x[o->get_rd()] = sext(m.load(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), 1), 8); } break;
            case 11: { reg.x[o->get_rd()] = sext(m.load(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), 2), 16); } break;
            case 12: { reg.x[o->get_rd()] = m.load(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), 4); } break;
            case 13: { reg.x[o->get_rd()] = m.load(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), 1); } break;
            case 14: { reg.x[o->get_rd()] = m.load(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), 2); } break;
            case 15: { m.store(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), reg.x[o->get_rs2()], 1); } break;
            case 16: { m.store(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), reg.x[o->get_rs2()], 2); } break;
            case 17: { m.store(reg.x[o->get_rs1()] + sext(o->get_imm(), 12), reg.x[o->get_rs2()], 4); } break;
            case 18: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] + sext(o->get_imm(), 12); } break;
            case 19: { reg.x[o->get_rd()] = ((signed)reg.x[o->get_rs1()] < (signed)sext(o->get_imm(), 12)); } break;
            case 20: { reg.x[o->get_rd()] = ((unsigned)reg.x[o->get_rs1()] < (unsigned)sext(o->get_imm(), 12)); } break;
            case 21: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] ^ sext(o->get_imm(), 12); } break;
            case 22: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] | sext(o->get_imm(), 12); } break;
            case 23: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] & sext(o->get_imm(), 12); } break;
            case 24: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] << o->get_imm(); } break;
            case 25: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] >> o->get_imm(); } break;
            case 26: { reg.x[o->get_rd()] = sext(reg.x[o->get_rs1()] >> o->get_imm(), 32 - o->get_imm()); } break;
            case 27: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] + reg.x[o->get_rs2()]; } break;
            case 28: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] - reg.x[o->get_rs2()]; } break;
            case 29: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] << reg.x[o->get_rs2()]; } break;
            case 30: { reg.x[o->get_rd()] = ((signed)reg.x[o->get_rs1()] < (signed)reg.x[o->get_rs2()]); } break;
            case 31: { reg.x[o->get_rd()] = ((unsigned)reg.x[o->get_rs1()] < (unsigned)reg.x[o->get_rs2()]); } break;
            case 32: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] ^ reg.x[o->get_rs2()]; } break;
            case 33: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] >> (reg.x[o->get_rs2()] & 0x1f); } break;
            case 34: { reg.x[o->get_rd()] = sext(reg.x[o->get_rs1()] >> (reg.x[o->get_rs2()] & 0x1f), 32 - (reg.x[o->get_rs2()] & 0x1f)); } break;
            case 35: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] | reg.x[o->get_rs2()]; } break;
            case 36: { reg.x[o->get_rd()] = reg.x[o->get_rs1()] & reg.x[o->get_rs2()]; } break;
        }
        if(reg.x[0]) reg.x[0] = 0;
        // if (1) {
        //     puts("end");
        //     printf("rs1=%d rs2=%d rd=%d\n",reg.x[o->get_rs1()], reg.x[o->get_rs2()], reg.x[o->get_rd()]);
        // }
        return flag;
    }
};

class cabbage_cpu {
private:
    ALU a;
    decoder p;
    Memory m;
    Register reg;
public:
    void work() {
        m.init();
        while (1) {
            unsigned ins = m.get(Register::pc); 
            if (ins == 0x0ff00513) break;
            if (a.work(p.get_instruction(ins))) Register::pc += 4;;
            if (reg.x[0]) break;
        }
        cout << std::dec << (((unsigned int)reg.x[10]) & 255u) <<'\n';
    }
};
}
#endif
