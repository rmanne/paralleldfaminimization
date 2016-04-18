#include "DFA.h"
#include "Specials.h"
#include "Benchmark.h"

#include <cstdio>

#include <omp.h>

void benchmark1() {
    printf("Benchmark Pattern: Structured DFA\n");

    const Nat B = 3;
    DFA* dfas[B] = {
        specialDFA1(1000000)->prepare(),
        specialDFA2(2000)->prepare(),
        specialDFA3(1000000)->prepare(),
    };
    runBenchmarks<B>(dfas);
    for (Nat b = 0; b < B; b++)
        delete dfas[b];
}
