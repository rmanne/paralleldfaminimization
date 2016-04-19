/* @file Set.h
 *
 * This class is provides a dynamic-size array.
 */
#pragma once
#include "Config.h"

class Set {
private:
    Nat  c;
    Nat  s;

public:
    Pair<Nat, long>* d; // offsets:bitvector
    // if d.first[i] == q / PACKED_SIZE then (q ∈ S iff d.second[i] & (q % PACKED_SIZE))

    Set();
    ~Set();

    Nat size() const;

    Pair<Nat, long>* begin() const;
    Pair<Nat, long>* end() const;

    void insert(Nat n);
};
