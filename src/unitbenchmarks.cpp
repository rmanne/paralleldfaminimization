#include "DFA.h"
#include "Benchmark.h"

#include "Vector.h"

#include <cstdio>

#include <omp.h>

Nat partitionBitVectorBenchmark(Nat Q, Nat seed) {
    bool* U = (bool*)calloc(Q, sizeof(bool));

    for (Nat q = 0; q < Q; q++)
        U[q] = rand_r(&seed) >= RAND_MAX / 2;

    Seq* s = new Seq(Q);
    for (Nat q = 0; q < Q; q++)
        s->bitVector[q] = rand_r(&seed) >= RAND_MAX / 2;
    for (Nat q = 0; q < Q; q++)
        if (s->bitVector[q])
            s->c++;

    long begin = currentTime();
    Seq* s′ = s->partition([U](Nat u) -> bool { return U[u]; });
    Nat time = currentTime() - begin;

    delete s′;
    delete s;
    free(U);
    return time;
}

Nat partitionConversionBenchmark(Nat Q, Nat seed) {
    bool* U = (bool*)calloc(Q, sizeof(bool));

    for (Nat q = 0; q < Q; q++)
        U[q] = rand_r(&seed) >= RAND_MAX / 2;

    Seq* s = new Seq(Q);
    for (Nat q = 0; q < Q; q++)
        s->bitVector[q] = rand_r(&seed) >= RAND_MAX * (float)19 / 20;
    for (Nat q = 0; q < Q; q++)
        if (s->bitVector[q])
            s->c++;

    long begin = currentTime();
    Seq* s′ = s->partition([U](Nat u) -> bool { return U[u]; });
    Nat time = currentTime() - begin;

    delete s′;
    delete s;
    free(U);
    return time;
}

Nat partitionElemArrayBenchmark(Nat l, Nat seed) {
    bool* U = (bool*)calloc(10 * l, sizeof(bool));

    for (Nat q = 0; q < 10 * l; q++)
        U[q] = rand_r(&seed) >= RAND_MAX / 2;

    Seq* s = new Seq(10 * l, l);
    ASSERT(s->τ == Seq::ElemArray);
    for (Nat i = 0; i < l; i++)
        s->elemArray[i] = 10 * l * (rand_r(&seed) / (float)RAND_MAX);
    s->c = l;

    long begin = currentTime();
    Seq* s′ = s->partition([U](Nat u) -> bool { return U[u]; });
    Nat time = currentTime() - begin;

    delete s′;
    delete s;
    free(U);
    return time;
}


#define RANDOM(seed) (rand_r(&seed) * (double)rand_r(&seed) * rand_r(&seed) \
                     / (RAND_MAX * (double)RAND_MAX * RAND_MAX))
static inline DFA* randomDFA(Nat Q, Nat Σ, Nat F, Nat seed) {
    DFA* dfa = new DFA(Q, Σ);
    for (Nat q = 0; q < Q; q++)
        for (Nat σ = 0; σ < Σ; σ++)
            dfa->δ[q][σ] = Q * RANDOM(seed);

    dfa->q₀ = Q * RANDOM(seed);
    dfa->F.c = F;
    while (F > 0) {
        Nat x = Q * RANDOM(seed);
        if (!dfa->F.bitVector[x]) {
            dfa->F.bitVector[x] = true;
            F--;
        }
    }

    return dfa;
}
Nat quotientBenchmark(Nat Q, Nat Σ, Nat F, Nat seed) {
    DFA* dfa = randomDFA(Q, Σ, F, seed);

    Nat Q′ = Q * (rand_r(&seed) / (float)RAND_MAX);
    //bool* used = new bool[Q′];
    Nat* ≡ = new Nat[Q];
    for (Nat q = 0; q < Q; q++)
        ≡[q] = Q′ * (rand_r(&seed) / (float)RAND_MAX);

    long begin = currentTime();
    DFA* minimal = dfa->quotient(≡, Q′);
    Nat time = currentTime() - begin;
    delete minimal;
    delete dfa;
    delete[] ≡;
    return time;
}

Nat complementBenchmark(Nat Q, Nat seed) {
    Seq* s = new Seq(Q);
    for (Nat q = 0; q < Q; q++)
        s->bitVector[q] = rand_r(&seed) >= RAND_MAX / 2;
    for (Nat q = 0; q < Q; q++)
        if (s->bitVector[q])
            s->c++;

    long begin = currentTime();
    Seq* s′ = s->complement();
    Nat time = currentTime() - begin;

    delete s′;
    delete s;
    return time;
}

Nat mutableMapBenchmark(Nat Q, Nat seed) {
    Seq* s = new Seq(Q);
    for (Nat q = 0; q < Q; q++)
        s->bitVector[q] = rand_r(&seed) >= RAND_MAX / 2;
    for (Nat q = 0; q < Q; q++)
        if (s->bitVector[q])
            s->c++;

    long begin = currentTime();
    bool* U = (bool*)calloc(Q, sizeof(bool));
    bool Uempty = true;
    s->map([&U, &Uempty, &Q](Nat p) mutable {
        Nat seed = 0;
        if (rand_r(&seed) <= 5000)
            Uempty = false;
        for (Nat q = 0; q < 100 * rand_r(&seed) / (float)RAND_MAX; q++)
            U[(int)(Q * rand_r(&seed) / (float)RAND_MAX)] = true;
    });
    Nat time = currentTime() - begin;

    free(U);
    delete s;
    return time;
}

void unitbenchmarks() {
    const Nat B = 6;
    Nat timingArray[B][BENCHMARK_THREADS];
    Function<Nat()> runners[B] = {
        []() { return partitionBitVectorBenchmark(1000000, 81); },
        []() { return partitionConversionBenchmark(1000000, 12); },
        []() { return partitionElemArrayBenchmark(1000000, 248); },
        []() { return quotientBenchmark(100000, 10, 3335, 248); },
        []() { return complementBenchmark(1000000, 85); },
        []() { return mutableMapBenchmark(1000000, 85); },
    };

    progressInit(B);
    for (Nat b = 0; b < B; b++) {
        for (Nat i = 0; i < BENCHMARK_THREADS; i++) {
            omp_set_num_threads(1 << i);

            Function<Nat()> t = runners[b];
            Nat times[BENCHMARK_ITERATIONS];
            for (Nat c = 0; c < BENCHMARK_ITERATIONS; c++)
                times[c] = t(),
                progressIncrement();

            timingArray[b][i] = average(times);
        }
    }
    progressFinalize();

    printf("Benchmark Pattern: Unit Benchmarks\n");
    printStats(B, timingArray);
}
