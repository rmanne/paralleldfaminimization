#include "DFA.h"
#include "Specials.h"

#include "Benchmark.h" // just for timing
#include <cstdio>

bool test2() {
    bool test = true;
    for (Nat i = 3; i < 1000; i += 300) {
        DFA* dfa = specialDFA2(i); // circular dfa with 1 accept state (aka, can't be minimized)
        dfa->prepare();

        long begin = currentTime();
        DFA* minimal = dfa->minimize();
        printf("Ticks for %d : %ld\n", i, currentTime() - begin);
        minimal->prepare();
        DFA* minimal2 = minimal->minimize();

        test &= minimal->Q == i && minimal2->Q == i;

        delete dfa;
        delete minimal;
        delete minimal2;
    }
    return test;
}
