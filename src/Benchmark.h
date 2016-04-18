#pragma once

#include <ctime>
#include <sys/time.h>
#include <algorithm>

#include <omp.h>

#define BENCHMARK_THREADS 4
#define BENCHMARK_ITERATIONS 5 // take median over iterations
static int progressMax;
static int progressCurrent;
static inline void progressInit(int benchmarks) {
    progressMax = BENCHMARK_THREADS * BENCHMARK_ITERATIONS * benchmarks;
    printf("[>");
    for (int i = 0; i < progressMax; i++)
        printf(" ");
    printf("]\r");
    fflush(stdout);
}
static inline void progressIncrement() {
    progressCurrent++;
    printf("[");
    for (int i = 0; i < progressCurrent; i++)
        printf("=");
    printf(">");
    for (int i = progressCurrent; i < progressMax; i++)
        printf(" ");
    printf("]\r");
    fflush(stdout);
}
static inline void progressFinalize() {
    for (int i = 0; i < progressMax + 3; i++)
        printf(" ");
    printf("\r");
}

static inline long currentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);

    return 1000000 * time.tv_sec + time.tv_usec;
    //return 1000 * clock() / CLOCKS_PER_SEC;
}

static inline Nat average(Nat array[BENCHMARK_ITERATIONS]) {
    // take the median (should be resistant to outliers)
    std::sort(array, array + BENCHMARK_ITERATIONS);
    return array[(BENCHMARK_ITERATIONS + 1) / 2];

    // don't take mean
    //Nat s = 0;
    //for (Nat i = 0; i < BENCHMARK_ITERATIONS; i++)
    //    s += array[i];
    //return s;
}

static inline void printStats(const Nat BENCHMARK_COUNT, Nat (*timingArray)[BENCHMARK_THREADS]) {
    printf("Threads:     ");
    for (Nat t = 0; t < 4 && t < BENCHMARK_THREADS; t++) // until 8
        printf("  %d  ", 1 << t);
    for (Nat t = 4; t < 7 && t < BENCHMARK_THREADS; t++) // until 64
        printf(" %d  ", 1 << t);
    for (Nat t = 7; t < BENCHMARK_THREADS; t++) // until 512
        printf("%d  ", 1 << t);
    printf("\n");
    for (Nat b = 0; b < BENCHMARK_COUNT; b++) {
        printf("Speedups t%d:", b);
        for (Nat i = 0; i < BENCHMARK_THREADS; i++) {
            double speedup = timingArray[b][0] / (double)timingArray[b][i];
            if (speedup < 10)
                printf(" %.2f", speedup);
            else if (speedup < 100)
                printf(" %.1f", speedup);
            else
                printf(" %d", (int)speedup);
        }
        printf("\n");
    }
    printf("\n");
}

static inline Nat timeMinimization(DFA* dfa) {
    long begin = currentTime();
    DFA* minimal = dfa->minimize();
    Nat time = currentTime() - begin;
    delete minimal;

    return time;
}

template<Nat B>
static inline void runBenchmarks(DFA* dfas[B]) {
    Nat timingArray[B][BENCHMARK_THREADS];

    progressInit(B);
    for (Nat b = 0; b < B; b++) {
        for (Nat i = 0; i < BENCHMARK_THREADS; i++) {
            omp_set_num_threads(1 << i);

            Nat times[BENCHMARK_ITERATIONS];
            for (Nat c = 0; c < BENCHMARK_ITERATIONS; c++)
                times[c] = timeMinimization(dfas[b]),
                progressIncrement();

            timingArray[b][i] = average(times);
        }
    }
    progressFinalize();

    printStats(B, timingArray);
}
