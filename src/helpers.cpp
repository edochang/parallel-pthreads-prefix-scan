#include "helpers.h"

prefix_sum_args_t* alloc_args(int n_threads) {
  return (prefix_sum_args_t*) malloc(n_threads * sizeof(prefix_sum_args_t));
}

int next_power_of_two(int x) {
    // Don't believe this function produces correct results.  Modified to produce the correct results.
    int pow = 1;
    int counter = 0;
    while (pow < x) {
        pow *= 2;
        counter++;
    }
    return pow;
}

void fill_args(prefix_sum_args_t *args,
               int n_threads,
               int n_vals,
               int *inputs,
               int *outputs,
               bool spin,
               int (*op)(int, int, int),
               int n_loops,
               pthread_barrier_t *pthreadBarrier,  // To pass global barrier instance to threads
               spin_barrier *spin_barrier
               ) {
    // Updated to pass global / shared pthread barrier to threads
    for (int i = 0; i < n_threads; ++i) {
        args[i] = {
            inputs,
            outputs,
            spin,
            n_vals,
            n_threads,
            i,
            op,
            n_loops,
            pthreadBarrier,  // To pass global barrier instance to threads
            spin_barrier
        };
    }
}