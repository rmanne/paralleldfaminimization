// for additional benchmarking
#pragma once

#include "DFA.h"
#include "NFA.h"

static inline DFA* specialDFA1(Nat Q) {
    DFA* dfa = new DFA(Q, 2);
    for (Nat q = 0; q < Q - 1; q++)
        dfa->δ[q][0] = Q - 2,
        dfa->δ[q][1] = Q - 1;
    dfa->δ[Q - 1][0] = Q - 1;
    dfa->δ[Q - 1][1] = Q - 1;

    dfa->q₀ = 0;
    dfa->F.insert(Q - 1);
    return dfa;
}
static inline DFA* specialDFA2(Nat Q) {
    DFA* dfa = new DFA(Q, 1);
    for (Nat q = 0; q < Q - 1; q++)
        dfa->δ[q][0] = q + 1;
    dfa->δ[Q - 1][0] = 0;

    dfa->q₀ = 0;
    dfa->F.insert(Q - 1);
    return dfa;
}
static inline DFA* specialDFA3(Nat Q) {
    DFA* dfa = new DFA(Q, 2);
    for (Nat q = 0; q < Q - 1; q++)
        dfa->δ[q][0] = q == 0 ? 0 : q - 1,
        dfa->δ[q][1] = Q - 1;
    dfa->δ[Q - 1][0] = 0;
    dfa->δ[Q - 1][1] = Q - 1;

    dfa->q₀ = 0;
    dfa->F.insert(Q - 1);
    return dfa;
}

static inline NFA* specialNFA1() { // "b*(abbb|abab|aaab)a*"
    NFA* nfa = new NFA(13, 2);
    nfa->I.insert(0);
    nfa->δ[0][1].insert(0); // b*

    nfa->δ[0][0].insert(1); // b*a
    nfa->δ[1][1].insert(2); // b*ab
    nfa->δ[2][1].insert(3); // b*abb
    nfa->δ[3][1].insert(4); // b*abbb
    nfa->δ[4][0].insert(4); // b*abbba*

    nfa->δ[0][0].insert(5); // b*a
    nfa->δ[5][1].insert(6); // b*ab
    nfa->δ[6][0].insert(7); // b*aba
    nfa->δ[7][1].insert(8); // b*abab
    nfa->δ[8][0].insert(8); // b*ababa*

    nfa->δ[0][0].insert(9); // b*a
    nfa->δ[9][0].insert(10); // b*aa
    nfa->δ[10][0].insert(11); // b*aaa
    nfa->δ[11][1].insert(12); // b*aaab
    nfa->δ[12][0].insert(12); // b*aaaba*

    nfa->F.insert(4);
    nfa->F.insert(8);
    nfa->F.insert(12);
    return nfa;
}
static inline NFA* specialNFA2() { // "(pikachu|eevee)"
    NFA* nfa = new NFA(12, 26);
    nfa->I.insert(0);
    nfa->F.insert(11);
    
    nfa->δ[0]['p' - 'a'].insert(1);
    nfa->δ[1]['i' - 'a'].insert(2);
    nfa->δ[2]['k' - 'a'].insert(3);
    nfa->δ[3]['a' - 'a'].insert(4);
    nfa->δ[4]['c' - 'a'].insert(5);
    nfa->δ[5]['h' - 'a'].insert(6);
    nfa->δ[6]['u' - 'a'].insert(11);
    
    nfa->δ[0]['e' - 'a'].insert(7);
    nfa->δ[7]['e' - 'a'].insert(8);
    nfa->δ[8]['v' - 'a'].insert(9);
    nfa->δ[9]['e' - 'a'].insert(10);
    nfa->δ[10]['e' - 'a'].insert(11);

    return nfa;
}


#define RANDOM(seed) (rand_r(&seed) * (double)rand_r(&seed) * rand_r(&seed) \
                     / (RAND_MAX * (double)RAND_MAX * RAND_MAX))
static inline DFA* randomDFA(Nat Q, Nat Σ, Nat F, Nat seed) {
    DFA* dfa = new DFA(Q, Σ);
    for (Nat q = 0; q < Q; q++)
        for (Nat σ = 0; σ < Σ; σ++)
            dfa->δ[q][σ] = Q * RANDOM(seed);

    dfa->q₀ = Q * RANDOM(seed);
    while (F > 0) {
        Nat x = Q * RANDOM(seed);
        if (dfa->F.insert(x))
            F--;
    }
    Nat c = dfa->F.c;
    dfa->F.recomputeSize();
    ASSERT(c == dfa->F.c);

    return dfa;
}
