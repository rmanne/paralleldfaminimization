/* @file Set.h
 *
 * This class is just a wrapper around std::set
 */
#pragma once
#include "Config.h"

#include <set>

class Tree {
private:
    std::set<Nat> s;

public:
    Nat empty() {
        return s.size() == 0;
    }

    Nat first() {
        Nat n = *s.begin();
        s.erase(n);
        return n;
    }

    bool contains(Nat n) {
        return s.find(n) != s.end();
    }

    void insert(Nat n) {
        #pragma omp critical
        s.insert(n);
    }
};
