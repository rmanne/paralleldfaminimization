#include "DFA.h"

#include "Specials.h"

#include "Benchmark.h" // just for timing
#include <cstdio>

bool test1() {
    DFA* knownMinimal = new DFA(2, 2);
    knownMinimal->δ[0][0] = 0;
    knownMinimal->δ[0][1] = 1;
    knownMinimal->δ[1][0] = 1;
    knownMinimal->δ[1][1] = 1;
    knownMinimal->q₀ = 0;
    knownMinimal->F.insert(1);

    bool test = true;
    for (Nat i = 3; i < 10000; i += 1000) {
        DFA* dfa = specialDFA1(i);
        dfa->prepare();

        long begin = currentTime();
        DFA* minimal = dfa->minimize();
        printf("Ticks for %d : %ld\n", i, currentTime() - begin);
        minimal->prepare();
        DFA* minimal2 = minimal->minimize();

        ASSERT(knownMinimal->operator==(minimal) && knownMinimal->operator==(minimal2));
        //test &= knownMinimal->operator==(minimal) && knownMinimal->operator==(minimal2);

        delete dfa;
        delete minimal;
        delete minimal2;
    }
    delete knownMinimal;
    return test;
}
