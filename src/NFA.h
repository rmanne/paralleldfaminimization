// for additional benchmarking
#pragma once

#include "Vector.h"
#include <cmath>

class NFA {
public:
    Nat Q;
    Nat Σ;
    Matrix<Vector> δ;
    Vector I;
    Vector F;

    NFA(Nat Q, Nat Σ) : Q(Q), Σ(Σ), δ(Q, Σ), I(), F() {
    }

    NFA() : NFA(0, 0) {
    }

    DFA* powersetAutomata() const {
        DFA* M = new DFA(pow(2, Q), Σ);
        M->q₀ = 0;
        for (Nat i : I)
            M->q₀ |= 1 << i;
        for (Nat f : F)
            for (Nat q = 0; q < M->Q; q++)
                if (q & (1 << f))
                    M->F.insert(q);
        M->F.recomputeSize();

        for (Nat q = 0; q < M->Q; q++)
            for (Nat b = 0; b < Q; b++)
                if (q & (1 << b))
                    for (Nat σ = 0; σ < Σ; σ++)
                        for (Nat b′ : δ[b][σ])
                            M->δ[q][σ] |= 1 << b′;

        return M;
    }
};
