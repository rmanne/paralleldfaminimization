#include "Array.h"

#include <cstdlib>

Array::Array() : c(0), s(0), d(NULL) {
}

Array::~Array() {
    free(d);
}

Nat Array::size() const {
    return s;
}

Nat* Array::begin() const {
    return d;
}

Nat* Array::end() const {
    return d + s;
}

Nat Array::operator[](Nat i) const {
    return d[i];
}

void Array::insert(Nat n) {
    Nat i = s++;
    if (i >= c) { // resize
        c = c * 2 + 1;
        d = (Nat*)realloc((void*)d, c * sizeof(Nat));
    }
    d[i] = n;
}
