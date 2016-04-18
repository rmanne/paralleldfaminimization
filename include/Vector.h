/* @file Vector.h
 *
 * This class is provides a dynamic-size array.
 */
#pragma once
#include "Config.h"

class Vector {
private:
    Nat  c;
    Nat  s;
    Nat* d;

public:
    Vector();
    ~Vector();

    Nat size() const;

    Nat* begin() const;
    Nat* end() const;

    Nat operator[](Nat i) const;

    // concurrent
    void insert(Nat n);
};
