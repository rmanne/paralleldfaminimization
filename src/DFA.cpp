#include "DFA.h"

#include <cstdio>
#include <algorithm>
#include <cstring> // memset

#include "Vector.h"
#include "Tree.h"
#include "Benchmark.h"

#define DEBUG(x)
//#define DEBUG(x) printf x

DFA::DFA(Nat Q, Nat Σ) : Q(Q), Σ(Σ), δ(Q, Σ), F(Q), backEdges(Q, Σ) {
}

DFA* DFA::prepare() { // don't count this to time, skews it too much
    //#pragma omp parallel for schedule(static)
    for (Nat q = 0; q < Q; q++)
        for (Nat σ = 0; σ < Σ; σ++)
            backEdges[δ[q][σ]][σ].insert(q);
    return this;
}

static inline int factorial(int i) {
    return i == 0 ? 1 : i * factorial(i - 1);
}
bool DFA::operator==(DFA* M) {
    if (Q != M->Q || Σ != M->Σ)
        return false;

    Nat* ≡ = new Nat[Q];
    for (Nat q = 0; q < Q; q++)
        ≡[q] = -1;
    ≡[q₀] = M->q₀;

    Vector* v  = new Vector(); v->insert(q₀);
    Vector* v′ = new Vector();
    while (v->size() != 0) {
        bool stillgood = true;
        for (Nat i = 0; i < v->size(); i++) {
            Nat q = v->operator[](i);

            for (Nat σ = 0; σ < Σ; σ++) {
                bool changed = __sync_bool_compare_and_swap(&≡[δ[q][σ]], -1, M->δ[≡[q]][σ]);
                if (≡[δ[q][σ]] != M->δ[≡[q]][σ])
                    stillgood = false;
                if (changed)
                    v′->insert(δ[q][σ]);
            }
        }
        if (!stillgood) {
            delete[] ≡;
            return false;
        }

        delete v;
        v  = v′;
        v′ = new Vector();
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


DFA* DFA::quotient(Nat const * const ≡, Nat Q′) const {
    DFA* M = new DFA(Q′, Σ);

    long begin = currentTime();
    F.map([≡](Nat q) -> Nat { return ≡[q]; }, M->F);    // O(Q) | O(1)
    M->q₀ = ≡[q₀];
    DEBUG(("10: %d\n", currentTime() - begin));

    begin = currentTime();
    bool* c = (bool*)calloc(Q, sizeof(bool));
    #pragma omp parallel for schedule(static)
    for (Nat p = 0; p < Q; p++) {                       // O(QΣ) | O(Σ)
        Nat q = ≡[p];
        //if (!__sync_fetch_and_or(&c[q], 1))
        if (!c[q]) {
            c[q]++;
            for (Nat σ = 0; σ < Σ; σ++)
                M->δ[q][σ] = ≡[δ[p][σ]];
        }
    }
    free(c);
    DEBUG(("11: %d\n", currentTime() - begin));

    return M;
}

DFA* DFA::minimize() const {
    long begin = currentTime();
    Seq* F = new Seq(this->F);
    Seq* I = F->complement();

    Seq* P[Q];  // minimal DFA will not be larger
    Nat Q′ = 2; // number of partitions
    P[0] = I;
    P[1] = F;
    DEBUG(("02: %d\n", currentTime() - begin));

    Tree S; // Split List
    S.insert(F->size() <= I->size() ? 1 : 0);
    char* U = (char*)calloc(round(Q), 1);
    while (!S.empty()) {
        Nat i = S.first();

        for (Nat σ = 0; σ < Σ; σ++) {
            begin = currentTime();
            bool Uempty = true;
            P[i]->map([&U, &Uempty, σ, this](Nat q) mutable {
                if (backEdges[q][σ].size() != 0)
                    Uempty = false;
                for (Nat q′ : backEdges[q][σ])
                    __sync_fetch_and_or(&U[q′ / 8], 1 << (q′ % 8));
            });
            DEBUG(("03: %d\n", currentTime() - begin));
            if (Uempty)
                continue;
            Function<bool(Nat)> p = [U](Nat q) -> bool { return U[q / 8] & (1 << (q % 8)); };

            // split up strategy: do harder ones "vertically"
            // this means splitting up partitions into a "parallel" and
            // non-parallel partitions, and this will help because it should
            // decrease work imbalance, especially when a single partition
            // has most of the nodes (happens in cycle graphs)
            begin = currentTime();
            const int Pₛ = Q′;
            Nat* horizontalStrategy = new Nat[Pₛ];
            Nat horizontalCount = 0;
            for (Nat j = 0; j < Pₛ; j++) {
                if (P[j]->τ == Seq::ElemArray && P[j]->c <= HARD_BLOCK) {
                    horizontalStrategy[horizontalCount++] = j;
                    continue;
                }

                Seq* B⁻ = P[j]->partition(p);

                if (P[j]->size() > 0 && B⁻->size() > 0) {
                    Nat k = Q′++;
                    P[k] = B⁻;

                    if (S.contains(j))
                        S.insert(k);
                    else if (i == j)
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
            DEBUG(("04: %d\n", currentTime() - begin));

            begin = currentTime();
            #pragma omp parallel for schedule(static)
            for (Nat j′ = 0; j′ < horizontalCount; j′++) {
                Nat j = horizontalStrategy[j′];
                // B is P[j], after the partition, P[j] becomes B⁺
                Seq* B⁻ = P[j]->sequentialPartition(p);

                if (P[j]->size() > 0 && B⁻->size() > 0) {
                    Nat k = __sync_fetch_and_add(&Q′, 1);
                    P[k] = B⁻;

                    if (S.contains(j))
                        S.insert(k);
                    else if (i == j)
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
            DEBUG(("05: %d\n", currentTime() - begin));

            begin = currentTime();
            delete[] horizontalStrategy;
            memset(U, 0, round(Q));
            DEBUG(("06: %d\n", currentTime() - begin));
        }
    }
    free(U);

    begin = currentTime();
    Nat* ≡ = new Nat[Q];
    Nat* horizontalStrategy = new Nat[Q′];
    Nat horizontalCount = 0;
    for (Nat q = 0; q < Q′; q++) {
        if (P[q]->τ == Seq::ElemArray && P[q]->c <= HARD_BLOCK) {
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
    DEBUG(("08: %d\n", currentTime() - begin));
    begin = currentTime();
    #pragma omp parallel for schedule(static)
    for (Nat q = 0; q < Q′; q++)
        delete P[q];
    DEBUG(("09: %d\n", currentTime() - begin));

    DFA* M = quotient(≡, Q′);
    delete[] ≡;
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

bool DFA::accept(Vector* input) const {
    Nat q = q₀;
    for (Nat σ : *input)
        q = δ[q][σ];
    return F[q];
}
