#include <stdio.h>

#include "Config.h"

#include <omp.h>

bool test1();
bool test2();
bool test3();

void benchmark1();
void benchmark2();
void benchmark3();

void unitbenchmarks();

#include "DFA.h"
#include "Specials.h"
#include <cstring>

int main() {
#ifdef FINETUNE
    //DFA* dfa = specialDFA1(100000)->prepare();
    //DFA* dfa2 = specialDFA1(100000)->prepare();
    //DFA* dfa = specialDFA2(2000)->prepare();
    //DFA* dfa2 = specialDFA2(2000)->prepare();
    DFA* dfa = specialDFA3(100000)->prepare();
    DFA* dfa2 = specialDFA3(100000)->prepare();
    long states[FINETUNE]; long times[FINETUNE]; int timeSamples;
    long states2[FINETUNE]; long times2[FINETUNE]; int timeSamples2;
    omp_set_num_threads(1);
    #pragma omp parallel
    {
        Nat x = 1 << 31;
        while (--x);
    }
    delete dfa->minimize();
    memcpy(states, dfa->states, dfa->timeSamples * sizeof(long));
    memcpy(times, dfa->times, dfa->timeSamples * sizeof(long));
    timeSamples = dfa->timeSamples;
    printf("\n\n\nBREAK\n\n\n");
    omp_set_num_threads(2);
    #pragma omp parallel
    {
        Nat x = 1 << 31;
        while (--x);
    }
    delete dfa2->minimize();
    memcpy(states2, dfa2->states, dfa2->timeSamples * sizeof(long));
    memcpy(times2, dfa2->times, dfa2->timeSamples * sizeof(long));
    timeSamples2 = dfa2->timeSamples;
    delete dfa2;
    delete dfa;

    ASSERT(timeSamples == timeSamples2);
    printf("Overall: %10ld vs %10ld\n", times[timeSamples - 1] - times[0],
           times2[timeSamples - 1] - times2[0]);
    for (int i = timeSamples - 1; i > 0; i--)
        times[i] -= times[i - 1], times2[i] -= times2[i - 1];
#define histSize 15
    long hist[histSize] = { 0 };
    long hist2[histSize] = { 0 };
    for (int i = 1; i < timeSamples; i++)
        hist[states[i - 1]] += times[i],
        hist2[states2[i - 1]] += times2[i];
    for (int i = 0; i < histSize; i++)
        printf("    %2d: %10ld vs %10ld\n", i, hist[i], hist2[i]);
    //for (int i = 1; i < timeSamples; i++)
    //    printf("%2d: %10ld vs %10ld\n", i, times[i] - times[i - 1], times2[i] - times2[i - 1]);
#else
    omp_set_num_threads(2);

    ASSERT(test1());
    ASSERT(test2());
    ASSERT(test3());
    printf("All correctness tests passed successfully!\n\n\n");

    //unitbenchmarks();
    benchmark1();
    benchmark2();
    benchmark3();
#endif
}
