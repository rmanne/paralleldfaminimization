#pragma once
#include "Config.h"

#include "Seq.h"
#include "Array.h"
#include "Set.h"

template<typename T>
class Matrix {
private:
    Nat m;
    T*  d;

public:
    Matrix(Nat n, Nat m) : m(m), d(new T[n * m]()) {
    }

    ~Matrix() {
        delete[] d;
    }

    T* operator[](Nat i) const {
        return d + m * i;
    }
};

typedef Matrix<Nat> Transitions;

class DFA {
public:
#ifdef FINETUNE
    mutable long times[FINETUNE];
    mutable long states[FINETUNE];
    mutable int timeSamples = 0;
#endif

    Nat Q;
    Nat Σ;

    Transitions δ;

    Nat q₀;
    Seq F;

private:
    Matrix<Set> backEdges;
public:

    DFA(Nat Q, Nat Σ);
    DFA() : DFA(0, 0) { }

    DFA* prepare(); // returns pointer to self

    bool operator==(DFA* M);    // O(QΣlog(Q)) <- probably

    DFA* quotient(Seq** P, Nat const * const ≡, Nat Q′) const;   // O(QΣ) | O(Σ)
    DFA* minimize() const;

    void reachableHelper(Nat*, Nat*, Nat) const;
    DFA* reachable() const;

    bool accept(Array* s) const;

    void print() {
        printf("%d states on %d symbols\n", Q, Σ);
        printf("%d -->", q₀);
        F.map([](Nat q) { printf(" %d", q); });
        printf("\n");

        for (Nat q = 0; q < Q; q++)
            for (Nat σ = 0; σ < Σ; σ++)
                printf("δ(%d, %d) = %d\n", q, σ, δ[q][σ]);
    }

};
