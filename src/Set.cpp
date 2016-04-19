#include "Set.h"

#include <cstdlib>

Set::Set() : c(0), s(0), d(NULL) {
}

Set::~Set() {
    free(d);
}

Nat Set::size() const {
    return s;
}

Pair<Nat, long>* Set::begin() const {
    return d;
}

Pair<Nat, long>* Set::end() const {
    return d + s;
}

void Set::insert(Nat n) {
    if (s == 0 || d[s - 1].first != n / PACKED_SIZE) {
        if (s != 0)
            ASSERT(n / PACKED_SIZE > d[s - 1].first);
        if (s >= c) { // resize
            c = c * 2 + 1;
            d = (Pair<Nat, long>*)realloc((void*)d, c * sizeof(Pair<Nat, long>));
        }
        d[s++] = Pair<Nat, long>(n / PACKED_SIZE, (long)1 << (n % PACKED_SIZE));
        return;
    }

    d[s - 1].second |= (long)1 << (n % PACKED_SIZE);
}
