#ifndef RISC_V_PARSER_H
#define RISC_V_PARSER_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <memory>
#include <bitset>
using std::cout;
using std::cin;
using std::unique_ptr;
using std::string;
using std::make_unique;

namespace hst{
static string funcs[38] = {"lui", "auipc", "jal", "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu", "lb", "lh", "lw", "lbu", "lhu", "sb", "sh", "sw", "addi", "slti", "sltiu", "xori", "ori", "andi", "slli", "srli", "srai", "add", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and" };

inline unsigned int get_num(unsigned int ins, int l, int r) {
    ins >>= l;
    return ins ^ ((ins >> (r - l + 1)) << (r - l + 1));
}

struct instruction {
    unsigned int op : 6;
    virtual unsigned int get_rd() = 0;
    virtual unsigned int get_rs1() = 0;
    virtual unsigned int get_rs2() = 0;
    virtual unsigned int get_imm() = 0;
    void print() { std::cout << std::dec << funcs[op] << ' ' <<get_rs1() << ' ' << get_rs2() << ' ' << get_rd() << ' ' << get_imm() <<'\n'; }
};
struct R_type: public instruction {
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int rd : 5;
    unsigned int get_rd() override { return rd; }
    unsigned int get_rs1() override { return rs1; }
    unsigned int get_rs2() override { return rs2; }
    unsigned int get_imm() override { return 0; }
    R_type(unsigned int ins) {
        unsigned int funct3 = (ins >> 12) & 0x7;
        rs1 = (ins >> 15) & 0x1f;
        rs2 = (ins >> 20) & 0x1f;
        rd = (ins >> 7) & 0x1f;
        if (funct3 == 0) {
            if ((ins >> 30) ^ 1) op = 27;
            else op = 28;
        }
        else if (funct3 == 1) op = 29;
        else if (funct3 == 2) op = 30;
        else if (funct3 == 3) op = 31;
        else if (funct3 == 4) op = 32;
        else if (funct3 == 5) {
            if ((ins >> 30) ^ 1) op = 33;
            else op = 34;
        }
        else if (funct3 == 6) op = 35;
        else if (funct3 == 7) op = 36;
    }
};
struct I_type: public instruction {
    unsigned int imm :12;
    unsigned int rs1 : 5;
    unsigned int rd : 5;
    unsigned int get_rd() override { return rd; }
    unsigned int get_rs1() override { return rs1; }
    unsigned int get_rs2() override { return 0; }
    unsigned int get_imm() override { return imm; }
    I_type(unsigned int ins) {
        unsigned int opcode = ins & 0x7f;
        unsigned int funct3 = (ins >> 12) & 0x7;
        rd = (ins >> 7) & 0x1f;
        rs1 = (ins >> 15) & 0x1f;
        imm = (ins >> 20) & 0xfff;
        if (opcode == 0x67) { op = 3; }
        else if (opcode == 0x03) {
            if (funct3 == 0) op = 10;
            else if (funct3 == 1) op = 11;
            else if (funct3 == 2) op = 12;
            else if (funct3 == 4) op = 13;
            else if (funct3 == 5) op = 14;
        }
        else if (opcode == 0x13) {
            if (funct3 == 0) op = 18;
            else if (funct3 == 2) op = 19;
            else if (funct3 == 3) op = 20;
            else if (funct3 == 4) op = 21;
            else if (funct3 == 6) op = 22;
            else if (funct3 == 7) op = 23;
            else if (funct3 == 1) op = 24;
            else if (funct3 == 5) {
                if ((ins >> 30) ^ 1) op = 25;
                else op = 26, imm ^= (1 << 10);
            }
        }
    }
};
struct S_type: public instruction {
    unsigned int imm : 12;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int get_rd() override { return 0; }
    unsigned int get_rs1() override { return rs1; }
    unsigned int get_rs2() override { return rs2; }
    unsigned int get_imm() override { return imm; }
    S_type(unsigned int ins) {
        unsigned int funct3 = (ins >> 12) & 0x7;
        rs1 = (ins >> 15) & 0x1f;
        rs2 = (ins >> 20) & 0x1f;
        imm = (ins >> 7) & 0x1f;
        imm |= get_num(ins, 25, 31) << 5;
        if (funct3 == 0) op = 15;
        else if (funct3 == 1) op = 16;
        else if (funct3 == 2) op = 17;
    }
};
struct B_type: public instruction {
    unsigned int imm : 13;
    unsigned int rs2 : 5;
    unsigned int rs1 : 5;
    unsigned int get_rd() override { return 0; }
    unsigned int get_rs1() override { return rs1; }
    unsigned int get_rs2() override { return rs2; }
    unsigned int get_imm() override { return imm; }
    B_type(unsigned int ins) {
        unsigned int funct3 = (ins >> 12) & 0x7;
        if (funct3 == 0) { op = 4; }
        else if (funct3 == 1) { op = 5; }
        else if (funct3 == 4) { op = 6; }
        else if (funct3 == 5) { op = 7; }
        else if (funct3 == 6) { op = 8; }
        else if (funct3 == 7) { op = 9; }
        rs1 = (ins >> 15) & 0x1f;
        rs2 = (ins >> 20) & 0x1f;
        imm |= ((ins >> 8) & 0xf) << 1;
        imm |= get_num(ins, 25, 30) << 5;
        imm |= ((ins >> 7) & 1) << 11;
        imm |= ((ins >> 31) &1) << 12;
    }
};
struct U_type: public instruction{
    unsigned int imm : 20;
    unsigned int rd : 5;
    unsigned int get_rd() override { return rd; }
    unsigned int get_rs1() override { return 0; }
    unsigned int get_rs2() override { return 0; }
    unsigned int get_imm() override { return imm; }
    U_type(unsigned int ins) {
        if ((ins & 0x7f) == 0x37) op = 0;
        else op = 1;
        rd = (ins >> 7) & 0x1f;
        imm = (ins >> 12) & 0xfffff;
    }
};
struct J_type: public instruction {
    unsigned int imm : 21;
    unsigned int rd : 5;
    unsigned int get_rd() override { return rd; }
    unsigned int get_rs1() override { return 0; }
    unsigned int get_rs2() override { return 0; }
    unsigned int get_imm() override { return imm; }
    J_type(unsigned int ins) {
        op = 2;
        rd = (ins >> 7) & 0x1f;
        imm |= ((ins >> 21) & 0x3ff) << 1;
        imm |= ((ins >> 20) & 1) << 11;
        imm |= ((ins >> 12) & 0xff) << 12;
        imm |= ((ins >> 31) & 1) << 20;
    }
};
class decoder {
public:
    unique_ptr<instruction> get_instruction(unsigned int ins) {
        unique_ptr<instruction> o;
        // cout << std::hex << ins <<'\n';
        // std::bitset<32> mm(ins);
        // cout << mm << '\n';
        uint8_t opcode = ins & 0x7f;
        if (opcode == 0x37 ||opcode == 0x17) {
            o = make_unique<U_type>(ins);
        }
        else if (opcode == 0x6f) {
            o = make_unique<J_type>(ins);
        }
        else if (opcode == 0x63) {
            o = make_unique<B_type>(ins);
        }
        else if (opcode == 0x67 || opcode == 0x03 || opcode == 0x13) {
            o = make_unique<I_type>(ins);
        }
        else if (opcode == 0x23) {
            o = make_unique<S_type>(ins);
        }
        else if (opcode == 0x33) {
            o = make_unique<R_type>(ins);
        }
        return o;
    }
};
}
#endif
