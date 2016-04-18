#include "DFA.h"

#include "NFA.h"
#include "Specials.h"

static inline void testPowerset(NFA* nfa, DFA* trueMinimal) {
    DFA* dfa = nfa->powersetAutomata()->prepare();

    DFA* minimal = dfa->minimize();
    DFA* reachable = dfa->reachable()->prepare();
    DFA* minimal2 = reachable->minimize();
    ASSERT(minimal2->operator==(trueMinimal));
    delete minimal2;
    delete reachable;
    delete minimal;
    delete dfa;
}

bool test3() {
    const char B = 2;
    Nat timingArray[B][4] = { 0 };

    NFA* nfas[B];
    DFA* minimals[B];

    // "b*(abbb|abab|aaab)a*" BEGIN
    nfas[0] = specialNFA1();
    minimals[0] = new DFA(7, 2);
    minimals[0]->q₀ = 0;
    minimals[0]->F.insert(2);
    minimals[0]->δ[0][0] = 3;
    minimals[0]->δ[0][1] = 0;
    minimals[0]->δ[1][0] = 4;
    minimals[0]->δ[1][1] = 4;
    minimals[0]->δ[2][0] = 2;
    minimals[0]->δ[2][1] = 6;
    minimals[0]->δ[3][0] = 5;
    minimals[0]->δ[3][1] = 1;
    minimals[0]->δ[4][0] = 6;
    minimals[0]->δ[4][1] = 2;
    minimals[0]->δ[5][0] = 4;
    minimals[0]->δ[5][1] = 6;
    minimals[0]->δ[6][0] = 6;
    minimals[0]->δ[6][1] = 6;
    // "b*(abbb|abab|aaab)a*" END

    // "(pikachu|eevee)" BEGIN
    nfas[1] = specialNFA2();
    minimals[1] = new DFA(13, 26);
    minimals[1]->q₀ = 1;
    minimals[1]->F.insert(12);
    minimals[1]->δ[1]['p' - 'a'] = 2;
    minimals[1]->δ[2]['i' - 'a'] = 3;
    minimals[1]->δ[3]['k' - 'a'] = 4;
    minimals[1]->δ[4]['a' - 'a'] = 5;
    minimals[1]->δ[5]['c' - 'a'] = 6;
    minimals[1]->δ[6]['h' - 'a'] = 7;
    minimals[1]->δ[7]['u' - 'a'] = 12;
    minimals[1]->δ[1]['e' - 'a'] = 8;
    minimals[1]->δ[8]['e' - 'a'] = 9;
    minimals[1]->δ[9]['v' - 'a'] = 10;
    minimals[1]->δ[10]['e' - 'a'] = 11;
    minimals[1]->δ[11]['e' - 'a'] = 12;
    // "(pikachu|eevee)" END

    for (char b = 1; b < B; b++)
        testPowerset(nfas[b], minimals[b]);

    for (char b = 0; b < B; b++) {
        delete nfas[b];
        delete minimals[b];
    }
    return true;
}
