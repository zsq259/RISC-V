#ifndef RISC_V_MEMORY_H
#define RISC_V_MEMORY_H
#include <cstdio>
#include <iostream>

namespace hst{
class Memory{
private:
    static unsigned char mem[20000005];
public:
    void init() {
        bool count = false;
        char c;
        unsigned int place = 0, x = 0;
        while ((c = getchar()) != EOF) {
            if (c >= 'a' && c <= 'z') c -= 32;
            if (c == '@') {
                std::cin >> std::hex >> place;
            }
            else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
                if (count) { 
                    x = x * 16 + (c > '9'? 10 + c - 'A' : c - '0');
                    Memory::mem[place++] = x; x = 0;
                    count ^= 1;
                }
                else x = c > '9'? 10 + c - 'A' : c - '0', count ^= 1;
            }
        }
    }
    unsigned int get(int place) {
        unsigned int ins = 0;
        for (int i = 0; i < 4; ++i) ins |= (Memory::mem[i + place] << (i * 8));
        return ins;
    }
    unsigned int load(int place, int n) {
        unsigned int x = 0;
        for (int i = 0; i < n; ++i) x |= mem[i + place] << (i * 8);
        return x;
    }
    void store(int place, unsigned int x, int n) {
        for (int i = 0; i < n; ++i) mem[i + place] = x & 0xff, x >>= 8;
    }
};

}
#endif