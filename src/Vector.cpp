#include "Vector.h"

#include <cstdlib>

Vector::Vector() : c(0), s(0), d(NULL) {
}

Vector::~Vector() {
    free(d);
}

Nat Vector::size() const {
    return s;
}

Nat* Vector::begin() const {
    return d;
}

Nat* Vector::end() const {
    return d + s;
}

Nat Vector::operator[](Nat i) const {
    return d[i];
}

void Vector::insert(Nat n) {
    Nat i = s++;
    if (i >= c) { // resize
        c = c * 2 + 1;
        d = (Nat*)realloc((void*)d, c * sizeof(Nat));
    }
    d[i] = n;
}
