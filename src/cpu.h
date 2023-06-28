#ifndef RISC_V_CPU_H
#define RISC_V_CPU_H

#include "parser.h"
#include "memory.h"
#include <memory>
#include <bitset>
#include <deque>
#include <queue>
using std::deque;
using std::queue;

namespace hst {

inline signed int sext(unsigned int x, int n) {
    return x >> (n - 1) ? x | (0xffffffff >> n << n) : x ^ (x >> n << n);
}

inline bool is_R(int op) { return op >= 27 && op <= 36; }
inline bool is_I(int op) { return op == 3 || (op >= 10 && op <= 14) || (op >= 18 && op <= 26); }
inline bool is_S(int op) { return op >= 15 && op <= 17; }
inline bool is_B(int op) { return op >= 4 && op <= 9; }
inline bool is_U(int op) { return op == 0 || op == 1; }
inline bool is_J(int op) { return op == 2; }

class Register {
public:
    unsigned int x[2][32];
    unsigned int pc[2];
    int q[2][32];
    void update(int clk) {
        pc[!clk] = pc[clk];
        for (int i = 1; i < 32; ++i) x[!clk][i] = x[clk][i], q[!clk][i] = q[clk][i];
        x[!clk][0] = 0;
    }
    void clear(int clk) {
        for (int i = 0; i < 32; ++i) q[clk][i] = -1;
    }
    void print(int clk) {
        for (int i = 0; i < 32; ++i) std::cerr << x[clk][i] << ' ';
        std::cerr << pc[clk] << '\n';
    }
};

extern Memory Mem;
extern Register Reg;

class ALU {
private:
    Memory *m = &Mem;
public:
    int run_U(int op, unsigned imm) {
        switch (op) {
            case 0: { return (signed int)(imm << 12); } break;
            case 1: { return imm; } break;
        } 
        throw;
    }
    int run_I(int op, unsigned rs1, unsigned imm) {
        switch (op) {
            case 10: { return sext(m->load(rs1 + sext(imm, 12), 1), 8); } break;
            case 11: { return sext(m->load(rs1 + sext(imm, 12), 2), 16); } break;
            case 12: { return m->load(rs1 + sext(imm, 12), 4); } break;
            case 13: { return m->load(rs1 + sext(imm, 12), 1); } break;
            case 14: { return m->load(rs1 + sext(imm, 12), 2); } break;
            case 18: { return rs1 + sext(imm, 12); } break;
            case 19: { return ((signed)rs1 < (signed)sext(imm, 12)); } break;
            case 20: { return ((unsigned)rs1 < (unsigned)sext(imm, 12)); } break;
            case 21: { return rs1 ^ sext(imm, 12); } break;
            case 22: { return rs1 | sext(imm, 12); } break;
            case 23: { return rs1 & sext(imm, 12); } break;
            case 24: { return rs1 << imm; } break;
            case 25: { return rs1 >> imm; } break;
            case 26: { return sext(rs1 >> imm, 32 - imm); } break;
        }
        throw;
    }
    int run_B(int op, unsigned rs1, unsigned rs2) {
        switch (op) {
            case 4: return rs1 == rs2;
            case 5: return rs1 != rs2;
            case 6: return (signed)rs1 < (signed)rs2;
            case 7: return (signed)rs1 >= (signed)rs2;
            case 8: return (unsigned)rs1 < (unsigned)rs2;
            case 9: return (unsigned)rs1 >= (unsigned)rs2;
        }
        throw;
    }
    int run_S(int op, unsigned rs1, unsigned imm) {
        return rs1 +sext(imm, 12);
    }
    int run_R(int op, unsigned rs1, unsigned rs2) {
        switch (op) {
            case 27: { return rs1 + rs2; } break;
            case 28: { return rs1 - rs2; } break;
            case 29: { return rs1 << rs2; } break;
            case 30: { return ((signed)rs1 < (signed)rs2); } break;
            case 31: { return ((unsigned)rs1 < (unsigned)rs2); } break;
            case 32: { return rs1 ^ rs2; } break;
            case 33: { return rs1 >> (rs2 & 0x1f); } break;
            case 34: { return sext(rs1 >> (rs2 & 0x1f), 32 - (rs2 & 0x1f)); } break;
            case 35: { return rs1 | rs2; } break;
            case 36: { return rs1 & rs2; } break;
        }
        throw;
    }
};

class Predictor {
public:
    bool result() { return false; }
};

class Bus {
private:
    bool flag[2];
public:
    void update(int clk) { flag[!clk] = flag[clk]; }
    void clear(int  clk) { flag[clk] = false; }
    void set(int clk) { flag[clk] = true; }
    bool get_flag(int clk) { return flag[clk]; }
};

extern Bus Bus_;

enum status{kcommit, kissue, kexcute, kwrite};

struct RSdata {
    int busy, op, vj, vk, qj, qk, A, dest;
};

class RSbase {
private:
    const static int maxSize = 32;
    RSdata v[2][maxSize];
    int c[2][maxSize];
public:
    friend class ReservationStation;
    friend class LoadStoreBuffer;
    friend class decoder;
    friend class ReorderBuffer;
};

class ReservationStation : public RSbase {
private:
    int size[2];
public:
    friend class ReorderBuffer;
    void update(int clk) { size[!clk] = size[clk]; for (int i = 0; i < maxSize; ++i) v[!clk][i] = v[clk][i], c[!clk][i] = c[clk][i]; }
    void add(int clk) { ++size[clk]; }
    bool full(int clk) { return size[clk] == maxSize; }
    void clear(int clk) {
        size[clk] = 0;
        for (int i = 0; i < maxSize; ++i) c[clk][i] = 0;
    }
    void bus(int id, int value, int clk) {
        for (int i = 0; i < maxSize; ++i) {
            if (!c[i]) continue;
            if (v[!clk][i].qj == id) v[clk][i].vj = value, v[clk][i].qj = -1;
            if (v[!clk][i].qk == id) v[clk][i].vk = value, v[clk][i].qk = -1;
            // if (v[!clk][i].qk == id && v[clk][i].op == 5) cout << "vvvvv= " << v[clk][i].vk << '\n';
        }
    }
};

class LoadStoreBuffer : public RSbase{
private:
    int size[2];
public:
    friend class ReorderBuffer;
    void update(int clk) { size[!clk] = size[clk]; for (int i = 0; i < maxSize; ++i) v[!clk][i] = v[clk][i], c[!clk][i] = c[clk][i]; }
    void add(int clk) { ++size[clk]; }
    bool full(int clk) { return size[clk] == maxSize; }
    void clear(int clk) {
        size[clk] = 0;
        for (int i = 0; i < maxSize; ++i) c[clk][i] = 0;
    }
    void bus(int id, int value, int clk) {
        for (int i = 0; i < maxSize; ++i) {
            if (!c[i]) continue;
            if (v[!clk][i].qj == id) v[clk][i].vj = value, v[clk][i].qj = -1;
            if (v[!clk][i].qk == id) v[clk][i].vk = value, v[clk][i].qk = -1;
        }
    }
};

extern ReservationStation RS_;
extern LoadStoreBuffer LSB_;

struct RoBdata {
    int id, busy,  dest, value, op;
    RoBdata(int id_, int b_, int v_, int o_): id(id_), busy(b_), value(v_), op(o_), dest(0) {}
};    

class ReorderBuffer {
private:
    int cnt[2] = {}, size[2] = {}, block[2] = {}, head[2] = {};
    const static int maxSize = 32;
    deque<RoBdata> que[2];
    ReservationStation *RS = &RS_;
    LoadStoreBuffer *LSB = &LSB_;
    Register *reg = &Reg;
    Memory *m = &Mem;
    ALU A;
public:
    friend class decoder;
    friend class cabbage_cpu;
    bool full(int clk) { return size[clk] == maxSize; }
    void update(int clk) { 
        cnt[!clk] = cnt[clk];
        size[!clk] = size[clk];
        block[!clk] = block[clk];
        head[!clk] = head[clk];
        que[!clk] = que[clk];
    }
    void clear(int clk) {
        while (!que[clk].empty()) que[clk].pop_back();
        cnt[clk] = size[clk] = block[clk] = head[clk] = 0;
    }
    void LSB_excute(int clk) {
        RSdata *a = nullptr;
        RoBdata *b = nullptr;
        for (int i = 0; i < maxSize; ++i) {
            if (!LSB->c[!clk][i] || LSB->v[!clk][i].qj != -1 || LSB->v[!clk][i].qk != -1) continue;
            
            a = &LSB->v[clk][i];
            int flag = 0, h = a->dest;
            b = &que[clk][h];
            if (is_I(b->op)) {
                for (int i = head[!clk]; i < h; ++i) if (is_S(que[!clk][i].op)) { flag = 1; break; }
            }
            else {
                for (int i = head[!clk]; i < h; ++i) if (is_S(que[!clk][i].op) || (que[!clk][i].op >= 10 && que[!clk][i].op <= 14)) { flag = 1; break; }
            }
            if (flag) continue;

            if (LSB->c[clk][i] < 3) { ++LSB->c[clk][i]; continue; }

            b->busy = 0;
            //b->status = kexcute;
            if (is_S(b->op)) {
                b->dest = a->vj + sext(a->A, 12);
                b->value = a->vk;
                // cout << "op= " << b->id << ' ' << a->vk << '\n';
            }
            else {
                b->value = A.run_I(a->op, a->vj, a->A);
            }
            LSB->c[clk][i] = 0; --LSB->size[clk];
            LSB->bus(a->dest, b->value, clk);
            RS->bus(a->dest, b->value, clk);
        }
    }
    void RS_excute(int clk) {
        RSdata *a = nullptr;
        RoBdata *b = nullptr;
        for (int i = 0; i < maxSize; ++i) {
            //printf("i=%d %d %d\n", i, RS->v[!clk][i].qj, RS->v[!clk][i].qk);
            if (!RS->c[!clk][i] || RS->v[!clk][i].qj != -1 || RS->v[!clk][i].qk != -1) continue;
            a = &RS->v[clk][i];
            b = &que[clk][a->dest];
            b->busy = 0;
            //b->status = kexcute;
            // if (b->id == 9) std::cerr << "vj================== " << RS->v[!clk][i].vj << '\n';
            if (is_R(a->op)) {
                if (b->dest) b->value = A.run_R(a->op, a->vj, a->vk);    
            }
            else if (is_U(a->op)) {
                if (b->dest) b->value = A.run_U(a->op, a->A);
            }
            else if (is_I(a->op)) {
                if (a->op == 3) { reg->pc[clk] = (a->vj + sext(a->A, 12)) & ~1; }
                else { if (b->dest) b->value = A.run_I(a->op, a->vj, a->A); }
            }
            else if (is_B(a->op)) {
                b->value ^= A.run_B(a->op, a->vj, a->vk);
            }
            RS->c[clk][i] = 0; --RS->size[clk];
            LSB->bus(a->dest, b->value, clk);
            RS->bus(a->dest, b->value, clk);
        }
    }
    void commit(int clk) { //clk: next time;
        //cout << "head=" << std::dec << head[!clk] << ' ' << size[!clk] << ' ' << que[!clk][head[!clk]].busy << ' ' << que[clk][head[clk]].op <<'\n';
        if (!size[!clk] || que[!clk][head[!clk]].busy) return ;
        // cout << "head=" << std::dec << head[!clk] << ' ' << size[!clk] << ' ' << que[!clk][head[!clk]].busy << ' ' << que[clk][head[clk]].op <<'\n';
        RoBdata *v = &que[clk][head[clk]]; ++head[clk];
        //que[clk].pop_front(); 
        --size[clk];
        v->busy = 0;
        if (v->op == 3) block[clk] = 0;
        if (is_B(v->op)) {
            if (v->value) {
                // std::cerr << "wrong!!!!!!!!!!!! " << v->op << ' ' << v->id << '\n';
                reg->pc[clk] = v->dest;
                clear(clk);
                RS->clear(clk);
                LSB->clear(clk);
                reg->clear(clk);
                Bus *b = &Bus_; b->set(clk);
            }
        }
        else if (is_S(v->op)) {
            if (v->op == 15) m->store(v->dest, v->value, 1);
            else if (v->op == 16) m->store(v->dest, v->value, 2);
            else if (v->op == 17) m->store(v->dest, v->value, 4);
        }
        else { 
            if (v->dest) reg->x[clk][v->dest] = v->value; 
            // if (reg->q[!clk][v->dest] != reg->q[clk][v->dest]) std::cerr << "q=== " <<  reg->q[!clk][v->dest] << ' ' <<  reg->q[clk][v->dest] << '\n';
            if (reg->q[clk][v->dest] == v->id) reg->q[clk][v->dest] = -1;
            RS->bus(v->id, v->value, clk);
            LSB->bus(v->id, v->value, clk);
        }
    }
};

extern ReorderBuffer RoB_;

class decoder {
private:
    ReorderBuffer *RoB = &RoB_;
    ReservationStation *RS = &RS_;
    LoadStoreBuffer *LSB = &LSB_;
    Register *reg = &Reg;
public:
    shared_ptr<instruction> decode(unsigned int ins) {
        shared_ptr<instruction> o;
        uint8_t opcode = ins & 0x7f;
        if (opcode == 0x37 ||opcode == 0x17) { o = make_shared<U_type>(ins); }
        else if (opcode == 0x6f) { o = make_shared<J_type>(ins); }
        else if (opcode == 0x63) { o = make_shared<B_type>(ins); }
        else if (opcode == 0x67 || opcode == 0x03 || opcode == 0x13) { o = make_shared<I_type>(ins); }
        else if (opcode == 0x23) { o = make_shared<S_type>(ins); }
        else if (opcode == 0x33) { o = make_shared<R_type>(ins); }
        return o;
    }
    bool issue(shared_ptr<instruction> o, int pc, int clk) { //clk: next time
        if (RoB->full(!clk)) return false;
        int op = (o->op >= 10 && o->op <= 17);
        if (op && LSB->full(!clk)) return false;
        if (!op && RS->full(!clk)) return false;
        ++RoB->size[clk];
        RoB->que[clk].push_back(RoBdata(RoB->cnt[clk], 1, 0, o->op));
        RSdata *v = nullptr;
        if (op) { for (int i = 0; i < LSB->maxSize; ++i) if (!LSB->c[!clk][i]) { v = &LSB->v[clk][i]; LSB->c[clk][i] = 1; LSB->add(clk); break; } }
        else { for (int i = 0; i < RS->maxSize; ++i) if (!RS->c[!clk][i]) { v = &RS->v[clk][i]; RS->c[clk][i] = 1; RS->add(clk); break; } }
        v->busy = 1; v->dest = RoB->cnt[clk];
        v->A = o->get_imm(); v->op = o->op;
        int rs1 = o->get_rs1(), rs2 = o->get_rs2(), rd = o->get_rd();
        if (o->is_J()) { RoB->que[clk][RoB->cnt[clk]].value = pc + 4; }
        else if (o->op == 3) { RoB->que[clk][RoB->cnt[clk]].value = pc + 4; RoB->block[clk] = 1; }
        else if (o->op == 1) { v->A += pc; }
        v->qj = v->qk = -1;
        if (!(o->is_U() || o->is_J())) {
            // if (RoB->cnt[clk] == 9) std::cerr << "reg================= " << reg->q[!clk][rs1] << '\n';
            if (reg->q[!clk][rs1] != -1) {
                int h = reg->q[!clk][rs1];
                if (!RoB->que[!clk][h].busy) { v->vj = RoB->que[!clk][h].value; v->qj = -1; }
                else v->qj = h;
            }
            else { v->vj = reg->x[!clk][rs1]; v->qj = -1; }
            if (!rs1) { v->vj = 0; v->qj = -1; }
        }
        // if (RoB->cnt[clk] == 9) std::cerr << "reg================= " << v->qj << '\n';
        if (o->is_B() || o->is_S() || o->is_R()) {
            if (reg->q[!clk][rs2] != -1) {
                int h = reg->q[!clk][rs2];
                if (!RoB->que[!clk][h].busy) { v->vk = RoB->que[!clk][h].value; v->qk = -1; }
                else v->qk = h;
            }
            else { v->vk = reg->x[!clk][rs2]; v->qk = -1; }
            if (!rs2) { v->vk = 0; v->qk = -1; }
        }
        
        if (o->is_B()) { RoB->que[clk][RoB->cnt[clk]].value = pc & 1; RoB->que[clk][RoB->cnt[clk]].dest = pc & ~1; }
        
        if (!(o->is_B() || o->is_S())) { reg->q[clk][rd] = RoB->cnt[clk]; RoB->que[clk][RoB->cnt[clk]].dest = rd; }
        // if (RoB->cnt[clk] == 21) std::cerr << "vk================== " << v->vk << ' ' << v->qk << '\n';
        ++RoB->cnt[clk];
        
        return true;
    }
};


class cabbage_cpu {
private:
    ALU a;
    decoder d;
    Memory *m = &Mem;
    Register *reg = &Reg;
    ReorderBuffer *RoB = &RoB_;
    ReservationStation *RS = &RS_;
    LoadStoreBuffer *LSB = &LSB_;
    Predictor p;
    Bus *b = &Bus_;
    int clock = 0;
    bool changeFlag[2];
    unsigned ins[2], change[2];
public:
    void clear(int clk) { ins[clk] = change[clk] = changeFlag[clk] = 0; }
    void update(int clk) { ins[!clk] = ins[clk]; change[!clk] = change[clk]; changeFlag[!clk] = changeFlag[clk]; }
    void fetch(int clk) { ins[clk] = m->fetch(reg->pc[!clk]); reg->pc[clk] = reg->pc[!clk] + 4; }
    bool issue(shared_ptr<instruction> o, int clk, bool res) {
        if (!o->is_B()) return d.issue(o, reg->pc[!clk] - 4, clk);
        else return d.issue(o, (reg->pc[!clk] - 4 + (res? 4 : sext(o->get_imm(), 13))) | res, clk);
    }
    bool decode(int clk) {
        if (changeFlag[!clk]) ins[!clk] = change[!clk];
        changeFlag[clk] = 0;
        if (ins[!clk] == 0x0ff00513) return true;
        if (!ins[!clk]) return false;
        
        shared_ptr<instruction> o = d.decode(ins[!clk]);
        //std::cout << "ins=  " << std::hex << ins[!clk] << '\n';

        bool res = false;
        if (o->is_B()) res = p.result();

        if(!issue(o, clk, res)) { changeFlag[clk] = true; change[clk] = ins[!clk]; return false; }

        //o->print();

        if (o->is_J()) { reg->pc[clk] = reg->pc[!clk] - 4 + sext(o->get_imm(), 21); change[clk] = 0; changeFlag[clk] = 1; }
        else if (o->is_B()) { reg->pc[clk] = reg->pc[!clk] - 4  + (res? sext(o->get_imm(), 13) : 4); change[clk] = 0; changeFlag[clk] = 1; }
        else if (o->op == 3) { change[clk] = 0; changeFlag[clk] = 1; }
        // cout << "addadasd-=---------=" << std::hex << sext(o->get_imm(), 21) << '\n';
        // cout << "next pc = " << std::hex << reg->pc[clk] << '\n';
        // puts("ok");

        return false;
    }
    void work() {
        m->init();
        reg->clear(0); reg->clear(1);
        int clk = clock & 1;
        while (true) {
            // std::cerr << "-----------------------clock= " << clock << ' ' << clk << '\n';
            //if (clock > 50) break;
            if (!RoB->block[!clk]) {
                if (!changeFlag[!clk]) fetch(clk);
                else ins[clk] = 0;
                if (decode(clk)) break;
                //cout << std::hex << ins[clk] << ' ' << reg->pc[clk] << ' ' << changeFlag[clk] << '\n';
            }
            RoB->RS_excute(clk);
            RoB->LSB_excute(clk);
            RoB->commit(clk);
            if (b->get_flag(clk)) clear(clk), b->clear(clk);

            // reg->print(clk);

            update(clk);
            reg->update(clk);
            RoB->update(clk);
            RS->update(clk);
            LSB->update(clk);
            b->update(clk);
            ++clock; clk ^= 1;
            
        }
        cout << std::dec << (((unsigned int)reg->x[clock & 1][10]) & 255u) << '\n';
    }
};
}
#endif
