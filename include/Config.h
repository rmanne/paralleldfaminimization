/* @file Config.h
 *
 * All of the global defines
 */
#pragma once

//#define FINETUNE 100000

#include <cassert>
#define ASSERT assert

//#include <utility>
//#define Pair std::pair
template<typename A, typename B>
struct Pair {
    A first;
    B second;

    Pair(A first, B second) : first(first), second(second) { }
};

#include <functional>
#define Function std::function

typedef unsigned int Nat;

#define EASY_BLOCK 512 // blocksize when the computation is easy and uniform
#define HARD_BLOCK 64  // blocksize when the computation is hard and uniform

// if size of this sequence is more than 10% of the world,
// use a BitVector representation, and otherwise, use ElemArray
// 0 -> always BitVector, 1 -> always ElemArray
//#define TYPE_THRESHOLD 0.10
#define TYPE_THRESHOLD 0.001
// the constant associated with using elemarrays is at least 256, meaning
// we need to use a TYPE_THRESHOLD less than (1 / 256)

// for now, just use longs, we can think about vectorizing later
#include <unistd.h>
#define PACKED_SIZE (8 * sizeof(long))
#define CACHELINE_SIZE (8 * sysconf(_SC_LEVEL1_DCACHE_LINESIZE))
static inline Nat round(Nat count, Nat size = 8) {
    return (count + size - 1) / size;
}
