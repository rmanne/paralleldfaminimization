#include "DFA.h"
#include "Specials.h"
#include "Benchmark.h"

#include <cstdio>

#include <omp.h>

void benchmark2() {
    printf("Benchmark Pattern: (pseudo-)Randomly Generated DFA\n");

    const Nat B = 3;
    DFA* dfas[B] = {
        randomDFA(2000, 20, 2, 1)->prepare(),
        randomDFA(300000, 1, 270000, 2)->prepare(),
        randomDFA(100, 200, 5, 3)->prepare(),
    };
    runBenchmarks<B>(dfas);
    for (Nat b = 0; b < B; b++)
        delete dfas[b];
}
