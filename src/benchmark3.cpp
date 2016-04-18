#include "DFA.h"
#include "Specials.h"
#include "Benchmark.h"

#include <cstdio>

void benchmark3() {
    printf("Benchmark Pattern: The Powerset Automata From Structured NFA\n");

    const Nat B = 2;
    NFA* nfas[B] = { specialNFA1(), specialNFA2() };
    DFA* dfas[B];
    for (Nat b = 0; b < B; b++)
        dfas[b] = nfas[b]->powersetAutomata()->prepare();
    runBenchmarks<B>(dfas);
    for (Nat b = 0; b < B; b++)
        delete dfas[b];
}
