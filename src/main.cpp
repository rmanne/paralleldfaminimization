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

int main() {
    //omp_set_num_threads(1);
    //ASSERT(test2());
    //omp_set_num_threads(2);
    //ASSERT(test2());

    omp_set_num_threads(2);

    ASSERT(test1());
    ASSERT(test2());
    ASSERT(test3());
    printf("All correctness tests passed successfully!\n\n\n");

    //unitbenchmarks();
    benchmark1();
    benchmark2();
    benchmark3();
}
