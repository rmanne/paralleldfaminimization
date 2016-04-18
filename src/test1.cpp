#include "DFA.h"

#include "Specials.h"

#include <ctime>
#include <cstdio>

bool test1() {
    DFA* knownMinimal = new DFA(2, 2);
    knownMinimal->δ[0][0] = 0;
    knownMinimal->δ[0][1] = 1;
    knownMinimal->δ[1][0] = 1;
    knownMinimal->δ[1][1] = 1;
    knownMinimal->q₀ = 0;
    knownMinimal->F.c = 1;
    knownMinimal->F.insert(1);

    bool test = true;
    for (Nat i = 3; i < 10000; i += 1000) {
        DFA* dfa = specialDFA1(i);
        dfa->prepare();

        clock_t begin = clock();
        DFA* minimal = dfa->minimize();
        printf("Ticks for %d : %d\n", i, clock() - begin);
        minimal->prepare();
        DFA* minimal2 = minimal->minimize();

        test &= knownMinimal->operator==(minimal) && knownMinimal->operator==(minimal2);

        delete dfa;
        delete minimal;
        delete minimal2;
    }
    delete knownMinimal;
    return test;
}
