#include "Seq.h"

#include <cstdlib>
#include <algorithm> // for std::copy

#include <cstring> // for memset
#include <immintrin.h>
#include <omp.h>

static inline void* vcalloc(Nat size) {
    void* p = valloc(size);
    memset(p, 0, size);
    return p;
}

Seq::Seq(const Seq& s) : τ(s.τ), Q(s.Q), c(s.c) {
    if (τ == BitVector) {
        bitVector = (long*)valloc(round(Q, PACKED_SIZE) * sizeof(long));
        std::copy(s.bitVector, s.bitVector + round(Q, PACKED_SIZE), bitVector);
    } else {
        elemArray = (Nat*)malloc(c * sizeof(Nat));
        std::copy(s.elemArray, s.elemArray + c, elemArray);
    }
}

Seq::Seq(Nat Q) : τ(BitVector), Q(Q), c(0) {
    bitVector = (long*)vcalloc(round(Q, PACKED_SIZE) * sizeof(long));
}

Seq::Seq(Nat Q, Nat c) : Q(Q), c(0) {
    if (Q * TYPE_THRESHOLD < c)
        τ = BitVector, bitVector = (long*)vcalloc(round(Q, PACKED_SIZE) * sizeof(long));
    else
        τ = ElemArray, elemArray = (Nat*)malloc(c * sizeof(Nat));
}

Seq::~Seq() {
    if (τ == BitVector)
        free(bitVector);
    else
        free(elemArray);
}

bool Seq::horizontalStrategy() const {
    return τ == ElemArray; // && c <= EASY_BLOCK;
}
void Seq::recomputeSize() {
    ASSERT(τ == BitVector);
    Nat c = 0;
    #pragma omp parallel for schedule(static, CACHELINE_SIZE / PACKED_SIZE) reduction(+:c)
    for (Nat q = 0; q < round(Q, PACKED_SIZE); q++)
        c += _popcnt64(bitVector[q]);
    this->c = c;
}
bool Seq::operator[](Nat q) const {
    ASSERT(τ == BitVector);
    return bitVector[q / PACKED_SIZE] & ((long)1 << (q % PACKED_SIZE));
}
void Seq::atomicInsert(Nat q) {
    ASSERT(τ == BitVector);
    __sync_fetch_and_or(&bitVector[q / PACKED_SIZE], (long)1 << (q % PACKED_SIZE));
}
bool Seq::insert(Nat q) {
    ASSERT(τ == BitVector);
    if (bitVector[q / PACKED_SIZE] & ((long)1 << (q % PACKED_SIZE)))
        return false;
    bitVector[q / PACKED_SIZE] |= (long)1 << (q % PACKED_SIZE);
    c++;
    return true;
}
void Seq::zero() {
    ASSERT(τ == BitVector);
    //__m256i z = _mm256_set1_epi64x(0);
    //#pragma omp parallel for schedule(static)
    //for (Nat i = 0; i < round(Q, PACKED_SIZE) / 4; i++)
    //    _mm256_stream_si256((__m256i*)&bitVector[4 * i], z);

    //const Nat r = round(Q, PACKED_SIZE);
    //for (Nat i = 0; i < r % 4; i++)
    //    bitVector[r / 4 + i] = 0;
    memset(bitVector, 0, round(Q, PACKED_SIZE) * sizeof(long));
}

Seq* Seq::complement() const {
    ASSERT(τ == BitVector);
    Seq* s = new Seq(BitVector, Q, c);
    #pragma omp parallel for schedule(static, CACHELINE_SIZE / PACKED_SIZE) if(round(Q, PACKED_SIZE) > CACHELINE_SIZE * omp_get_max_threads())
    for (Nat q = 0; q < round(Q, PACKED_SIZE); q++)
        s->bitVector[q] = ~bitVector[q];

    if (Q != (Q / PACKED_SIZE) * PACKED_SIZE) {
        long mask = (long)1 << (Q - (Q / PACKED_SIZE) * PACKED_SIZE - 1);
        mask |= mask >> 1;
        mask |= mask >> 2;
        mask |= mask >> 4;
        mask |= mask >> 8;
        mask |= mask >> 16;
        mask |= mask >> 32;
        s->bitVector[Q / PACKED_SIZE] &= mask;
    }

    s->c = Q - c;
    return s;
}

Seq* Seq::sequentialPartition(Seq* p) { // O(c) | O(c)
    ASSERT(τ == ElemArray);
    Seq* s = new Seq(Q, c);

    Nat whl = 0;
    Nat whr = 0;

    for (Nat i = 0; i < c; i++)
        if (p->operator[](elemArray[i]))
            elemArray[whl++] = elemArray[i];
        else
            s->elemArray[whr++] = elemArray[i];

    ASSERT(c == whl + whr);
    c = whl;
    s->c = whr;
    return s;
}
Seq* Seq::partition(Seq* p) { // O(c) | O(log(c))
    if (τ == BitVector && Q * TYPE_THRESHOLD < c) {
        Seq* s = new Seq(BitVector, Q, c); // allocate but don't copy

        Nat c = 0;
        #pragma omp parallel for schedule(static, CACHELINE_SIZE / PACKED_SIZE) reduction(+:c)
        for (Nat i = 0; i < round(Q, PACKED_SIZE); i++) {
            long v  = bitVector[i];
            long v′ = v;
            long p′ = p->bitVector[i];
            v &= p′; // keep q if q ∈ p
            v′ ^= v; // remove q if q ∈ v

            bitVector[i] = v;
            s->bitVector[i] = v′;
            c += _popcnt64(v);
        }

        this->c = c;
        s->c -= c;
        return s;
    } else if (τ == BitVector) { // convert down
        long* bitVector = this->bitVector;
        τ = ElemArray;
        elemArray = (Nat*)malloc(c * sizeof(Nat));
        Seq* s = new Seq(Q, c);
        ASSERT(s->τ == ElemArray);
        c = 0;

        #pragma omp parallel for schedule(static, CACHELINE_SIZE / PACKED_SIZE)
        for (Nat i = 0; i < round(Q, PACKED_SIZE); i++) { // O(c) | O(1)
            long v = bitVector[i];
            long v′ = v;
            long p′ = p->bitVector[i];
            v &= p′; // keep q if q ∈ p
            v′ ^= v; // remove q if q ∈ v

            Nat c′ = __sync_fetch_and_add(&c, _popcnt64(v));
            Nat sc′ = __sync_fetch_and_add(&s->c, _popcnt64(v′));

            for (Nat j = 0; j < PACKED_SIZE; j++)
                if (v & ((long)1 << j))
                    elemArray[c′++] = PACKED_SIZE * i + j;
                else if (v′ & ((long)1 << j))
                    s->elemArray[sc′++] = PACKED_SIZE * i + j;
        }

        free(bitVector);
        return s;
    } else if (c <= HARD_BLOCK) { // ElemArray
        return sequentialPartition(p);
    } else { // ElemArray
        Seq* s = new Seq(Q, c);
        Nat* elemArray = this->elemArray;
        Nat  c = this->c;
        this->c = 0;
        this->elemArray = (Nat*)malloc(c * sizeof(Nat));

        #pragma omp parallel for schedule(static, CACHELINE_SIZE / sizeof(Nat))
        for (Nat i = 0; i < c; i++)
            if (p->operator[](elemArray[i]))
                this->elemArray[__sync_fetch_and_add(&this->c, 1)] = elemArray[i];
            else
                s->elemArray[__sync_fetch_and_add(&s->c, 1)] = elemArray[i];

        ASSERT(c == this->c + s->c);
        free(elemArray);
        return s;
    }
}

void Seq::map(Function<Nat(Nat)> f, Seq& s) const { // O(Q) | O(1)
    #pragma omp parallel for schedule(static, CACHELINE_SIZE / PACKED_SIZE) if(round(Q, PACKED_SIZE) > CACHELINE_SIZE * omp_get_max_threads())
    for (Nat i = 0; i < round(Q, PACKED_SIZE); i++) { // O(c) | O(1)
        long v = bitVector[i];
        for (Nat j = 0; j < PACKED_SIZE; j++)
            if (v & ((long)1 << j))
                s.atomicInsert(f(PACKED_SIZE * i + j));
    }
    s.recomputeSize();
}

void Seq::map(Function<void(Nat)> f) const { // O(c) | O(1)
    if (τ == BitVector) {
        //#pragma omp parallel for schedule(dynamic, 64)
        //#pragma omp parallel for schedule(static, 16)
        #pragma omp parallel for schedule(static)
        for (Nat i = 0; i < round(Q, PACKED_SIZE); i++) { // O(c) | O(1)
            long v = bitVector[i];
            for (Nat j = 0; j < PACKED_SIZE; j++)
                if (v & ((long)1 << j))
                    f(PACKED_SIZE * i + j);
        }
    } else {
        //#pragma omp parallel for schedule(dynamic, 64)
        //#pragma omp parallel for schedule(static, 16)
        #pragma omp parallel for schedule(static)
        for (Nat i = 0; i < c; i++)
            f(elemArray[i]);
    }
}

//void Seq::filter(Function<bool(Nat)> f) {
//    #pragma omp parallel for schedule(static)
//    for (Nat i = 0; i < round(Q, PACKED_SIZE); i++) {
//        long v = 0;
//        for (Nat j = 0; j < PACKED_SIZE; j++)
//            if (f(PACKED_SIZE * i + j))
//                v |= 1 << j;
//        bitVector[i] = v;
//    }
//}
//
//bool Seq::any(Seq* f) const { // returns if intersection is non-null
//    if (τ == BitVector) {
//        for (Nat i = 0; i < round(Q, PACKED_SIZE); i++)
//            if (bitVector[i] & p->bitVector[i])
//                return true;
//        return false;
//    } else {
//        for (Nat i = 0; i < c; i++)
//            if (p->operator[](elemArray[i]))
//                return true;
//        return false;
//    }
//}
//
//bool Seq::contains(Nat q) const {
//    if (τ == BitVector) {
//        return this->operator[](q);
//    for (Nat i = 0; i < c; i++)
//        if (elemArray[i] == q)
//            return true;
//    return false;
//}

Nat Seq::getMember() const { // returns arbitrary number
    ASSERT(c > 0);
    if (τ == ElemArray)
        return elemArray[0];
    for (Nat i = 0; i < round(Q, PACKED_SIZE); i++)
        if (bitVector[i])
            return PACKED_SIZE * i + __builtin_ctzl(bitVector[i]);
    ASSERT(false);
    return -1; // impossible
}
