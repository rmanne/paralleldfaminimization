/* @file Seq.h
 *
 * This class supports the following operations efficiently:
 *      size        O(1)
 *      partition   O(n)
 *      map         O(n)
 */
#pragma once
#include "Config.h"

class Seq {
private:
    enum Type {
        BitVector,
        ElemArray
    } τ;

    Seq(Type τ, Nat Q, Nat c) : τ(τ), Q(Q), c(c) {
        if (τ == BitVector)
            bitVector = (long*)valloc(round(Q, PACKED_SIZE) * sizeof(long));
        else
            elemArray = (Nat*)malloc(c * sizeof(Nat));
    }

public:
    Nat Q; // number of states (aka the maximum value)

    Nat c;
    Nat size() const {
        return c;
    }

    union {
        long* bitVector;
        Nat*  elemArray;
    };

    Seq(const Seq& s, Type τ); // copy with possible conversion (unsupported)
    Seq(const Seq& s);         // copy

    Seq(Nat Q);         // BitVector
    Seq(Nat Q, Nat c);  // Will automatically pick a good representation.

    ~Seq();

    void recomputeSize();
    bool operator[](Nat q) const; // for the translated bitVector indexing

    bool insert(Nat q);                             // O(1)
    void atomicInsert(Nat q);                       // O(1)
    Seq* complement() const;                        // O(Q)
    void zero();                                    // O(Q)

    Nat getMember() const;
    bool contains(Nat q) const;
    bool any(Seq* f) const;
    void filter(Function<bool(Nat)> f);

    bool horizontalStrategy() const;

    Seq* sequentialPartition(Seq* p);               // O(c) | O(c)
    Seq* partition(Seq* p);                         // O(c) | O(log(c))
    void map(Function<Nat(Nat)> f, Seq& s) const;   // O(Q) | O(1)
    void sequentialMap(Function<void(Nat)> f) const;// O(c) | O(c)
    void map(Function<void(Nat)> f) const;          // O(c) | O(1)

    void print() {
        printf("%s:", τ == BitVector ? "BitVector" : "ElemArray");
        map([](Nat q) { printf(" %d", q); });
        printf("\n");
    }

};
