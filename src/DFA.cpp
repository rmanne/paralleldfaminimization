#include "DFA.h"

#include <cstdio>
#include <algorithm>
#include <cstring> // memset

#include "Array.h"
#include "Tree.h"
#include "Benchmark.h"

#define DEBUG(x)
//#define DEBUG(x) printf x

DFA::DFA(Nat Q, Nat Σ) : Q(Q), Σ(Σ), δ(Q, Σ), F(Q), backEdges(Σ, Q) {
}

DFA* DFA::prepare() { // don't count this to time, skews it too much
    //#pragma omp parallel for schedule(static)
    for (Nat q = 0; q < Q; q++)
        for (Nat σ = 0; σ < Σ; σ++) // TODO: make more efficient, pack it
            backEdges[σ][δ[q][σ]].insert(q);
    return this;
}

bool DFA::operator==(DFA* M) {
    if (Q != M->Q || Σ != M->Σ)
        return false;

    Nat* ≡ = new Nat[Q];
    for (Nat q = 0; q < Q; q++)
        ≡[q] = -1;
    ≡[q₀] = M->q₀;

    Array* v  = new Array(); v->insert(q₀);
    Array* v′ = new Array();
    while (v->size() != 0) {
        for (Nat i = 0; i < v->size(); i++) {
            Nat q = v->operator[](i);

            for (Nat σ = 0; σ < Σ; σ++) {
                bool changed = __sync_bool_compare_and_swap(&≡[δ[q][σ]], -1, M->δ[≡[q]][σ]);
                if (≡[δ[q][σ]] != M->δ[≡[q]][σ]) {
                    delete[] ≡;
                    return false;
                }
                if (changed)
                    v′->insert(δ[q][σ]);
            }
        }

        delete v;
        v  = v′;
        v′ = new Array();
    }
    delete v;
    delete v′;

    for (Nat q = 0; q < Q; q++) {
        if (F[q] != M->F[≡[q]]) {
            delete[] ≡;
            return false;
        }
    }
    delete[] ≡;
    return true;
}


DFA* DFA::quotient(Seq** P, Nat const * const ≡, Nat Q′) const {
    DFA* M = new DFA(Q′, Σ);

#ifdef FINETUNE
    states[timeSamples] = 12; times[timeSamples++] = currentTime();
#endif
    F.map([≡](Nat q) -> Nat { return ≡[q]; }, M->F);    // O(Q) | O(1)
    M->q₀ = ≡[q₀];

#ifdef FINETUNE
    states[timeSamples] = 13; times[timeSamples++] = currentTime();
#endif
    #pragma omp parallel for schedule(static)
    for (Nat q = 0; q < Q′; q++) {                       // O(QΣ) | O(Σ)
        Nat p = P[q]->getMember();
        Nat* Mδq = M->δ[q];
        Nat* δp = δ[p];
        for (Nat σ = 0; σ < Σ; σ++)
            Mδq[σ] = ≡[δp[σ]];
    }

#ifdef FINETUNE
    states[timeSamples] = 14; times[timeSamples++] = currentTime();
#endif
    return M;
}

DFA* DFA::minimize() const {
#ifdef FINETUNE
    states[timeSamples] = 0; times[timeSamples++] = currentTime();
#endif
    Seq* F = new Seq(this->F);
    Seq* I = F->complement();

#ifdef FINETUNE
    states[timeSamples] = 1; times[timeSamples++] = currentTime();
#endif
    Seq* P[Q];  // minimal DFA will not be larger
    Nat Q′ = 2; // number of partitions
    P[0] = I;
    P[1] = F;

#ifdef FINETUNE
    states[timeSamples] = 2; times[timeSamples++] = currentTime();
#endif
    Tree S; // Split List
    S.insert(F->size() <= I->size() ? 1 : 0);
    //Seq* U = new Seq(Q);
    //void* rawMemory = operator new[](Σ * sizeof(Seq));
    //Seq* UΣ = static_cast<Seq*>(rawMemory);
    //for (Nat σ = 0; σ < Σ; σ++)
    //    new (&UΣ[σ])Seq(Q);
#ifdef FINETUNE
    states[timeSamples] = 3; times[timeSamples++] = currentTime();
#endif
    Seq** UΣ = new Seq*[Σ];
    for (Nat σ = 0; σ < Σ; σ++)
        UΣ[σ] = new Seq(Q);

#ifdef FINETUNE
    states[timeSamples] = 4; times[timeSamples++] = currentTime();
#endif
    while (!S.empty()) {
        Nat i = S.first();

        for (Nat σ = 0; σ < Σ; σ++) {
#ifdef FINETUNE
            states[timeSamples] = 5; times[timeSamples++] = currentTime();
#endif
            Seq* U = UΣ[σ];
            U->zero();
            U->c = 0;
#ifdef FINETUNE
            states[timeSamples] = 6; times[timeSamples++] = currentTime();
#endif
            Set* backEdgesσ = backEdges[σ];
            P[i]->map([&U, backEdgesσ](Nat q) mutable {
                if (backEdgesσ[q].size() != 0) {
                    U->c = 1;
                    for (Pair<Nat, long> p : backEdgesσ[q])
                        __sync_fetch_and_or(&U->bitVector[p.first], p.second);
                }
            });
            if (U->c == 0)
                continue;

#ifdef FINETUNE
            states[timeSamples] = 7; times[timeSamples++] = currentTime();
#endif
            Seq* B⁻ = P[i]->partition(U);

            if (P[i]->size() > 0 && B⁻->size() > 0) {
                Nat k = Q′++;
                P[k] = B⁻;
                P[k]->state.lastSplittingσ = σ;

                if (σ != Σ - 1)
                    S.insert(k);
                else
                    if (P[k]->size() < P[i]->size())
                        S.insert(k);
                    else
                        S.insert(i);
            } else if (B⁻->size() > 0) {
                delete P[i];
                P[i] = B⁻;
            } else {
                delete B⁻;
            }
        }

        Nat PQ′ = 0;
        while (PQ′ != Q′) {
            const Nat lower = PQ′;
            const Nat upper = Q′;
            PQ′ = Q′;
#ifdef FINETUNE
            states[timeSamples] = 8; times[timeSamples++] = currentTime();
#endif
            #pragma omp parallel for schedule(static, 16) //if(upper - lower > omp_get_max_threads())
            for (Nat j = lower; j < upper; j++) {
                if (i == j)
                    continue;
                for (Nat σ = 0; σ < Σ; σ++) {
                    Seq* U = UΣ[σ];
                    if (P[j]->state.lastSplittingσ == σ || U->c == 0) {
                        P[j]->state.lastSplittingσ = -1;
                        continue;
                    }

                    Nat initialSize = P[j]->size();
                    Seq* B⁻ = P[j]->partition(U);
                    ASSERT(B⁻->size() + P[j]->size() == initialSize);

                    if (P[j]->size() > 0 && B⁻->size() > 0) {
                        Nat k = __sync_fetch_and_add(&Q′, 1);
                        P[k] = B⁻;
                        P[k]->state.lastSplittingσ = σ;

                        if (S.contains(j))
                            S.insert(k);
                        else
                            if (P[j]->size() <= B⁻->size())
                                S.insert(j);
                            else
                                S.insert(k);
                    } else if (B⁻->size() > 0) {
                        delete P[j];
                        P[j] = B⁻;
                    } else {
                        delete B⁻;
                    }
                }
            }
        }
#ifdef FINETUNE
        states[timeSamples] = 9; times[timeSamples++] = currentTime();
#endif
    }
    //for (Nat σ = Σ - 1; σ >= 0; σ--)
    //    delete UΣ[σ];
    delete[] UΣ;
    //for (Nat σ = Σ - 1; σ >= 0; σ--)
    //    UΣ[σ].~Seq();
    //operator delete[](rawMemory);

#ifdef FINETUNE
    states[timeSamples] = 10; times[timeSamples++] = currentTime();
#endif
    Nat* ≡ = new Nat[Q];
    Nat* horizontalStrategy = new Nat[Q′];
    Nat horizontalCount = 0;
    for (Nat q = 0; q < Q′; q++) {
        if (P[q]->horizontalStrategy()) {
            horizontalStrategy[horizontalCount++] = q;
            continue;
        }
        P[q]->map([&≡, q](Nat p) mutable { ≡[p] = q; });
    }
    #pragma omp parallel for schedule(static)
    for (Nat j = 0; j < horizontalCount; j++) {
        Nat q = horizontalStrategy[j];
        P[q]->map([&≡, q](Nat p) mutable { ≡[p] = q; });
    }
    delete[] horizontalStrategy;

#ifdef FINETUNE
    states[timeSamples] = 11; times[timeSamples++] = currentTime();
#endif
    DFA* M = quotient(P, ≡, Q′);
    delete[] ≡;
    #pragma omp parallel for schedule(static)
    for (Nat q = 0; q < Q′; q++)
        delete P[q];
    return M;
}

void DFA::reachableHelper(Nat* ≡, Nat* Q′, Nat q) const {
    if (≡[q] & 0x80000000) {
        ≡[q] = *Q′;
        (*Q′)++;

        for (Nat σ = 0; σ < Σ; σ++)
            reachableHelper(≡, Q′, δ[q][σ]);
    }
}
DFA* DFA::reachable() const {
    Nat* ≡ = new Nat[Q];
    for (Nat q = 0; q < Q; q++)
        ≡[q] = -1;
    Nat Q′ = 0;
    reachableHelper(≡, &Q′, q₀);

    DFA* M = new DFA(Q′, Σ);
    M->q₀ = ≡[q₀];
    for (Nat q = 0; q < Q; q++)
        if (F[q] && !(≡[q] & 0x80000000))
            M->F.insert(≡[q]);

    #pragma omp parallel for schedule(static)
    for (Nat p = 0; p < Q; p++) {
        if (≡[p] & 0x80000000)
            continue;
        Nat q = ≡[p];
        for (Nat σ = 0; σ < Σ; σ++)
            M->δ[q][σ] = ≡[δ[p][σ]];
    }

    bool test = !(M->q₀ & 0x80000000);
    #pragma omp parallel for schedule(static) reduction(&:test)
    for (Nat q = 0; q < Q′; q++)
        for (Nat σ = 0; σ < Σ; σ++)
            test &= !(M->δ[q][σ] & 0x80000000);

    ASSERT(test);

    return M;
}

bool DFA::accept(Array* input) const {
    Nat q = q₀;
    for (Nat σ : *input)
        q = δ[q][σ];
    return F[q];
}
