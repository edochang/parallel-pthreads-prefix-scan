#include <iostream>
#include <argparse.h>
#include <threads.h>
#include <io.h>
#include <chrono>
#include <cstring>
#include "operators.h"
#include "helpers.h"
#include "prefix_sum.h"

using namespace std;

// Additional headers / namespaces

// global variables
pthread_barrier_t g_pthreadBarrier_t;  // comment out to use a class encapsulated instance of this.

int main(int argc, char **argv)
{
    // Parse args
    struct options_t opts;
    get_opts(argc, argv, &opts);

    bool sequential = false;
    if (opts.n_threads == 0) {
        opts.n_threads = 1;
        sequential = true;
    }

    // Setup threads
    pthread_t *threads = sequential ? NULL : alloc_threads(opts.n_threads);;

    // Setup pthread Barriers with default (NULL) attributes to wait for n_threads
    pthread_barrier_init(&g_pthreadBarrier_t, NULL, opts.n_threads);

    cout << "main: Read input file: " << opts.in_file << endl;  // debug statement

    // Setup args & read input data
    prefix_sum_args_t *ps_args = alloc_args(opts.n_threads);
    int n_vals;
    int *input_vals, *output_vals;
    read_file(&opts, &n_vals, &input_vals, &output_vals);

    cout << "main: File has n_vals: " << n_vals << endl;  // debug statement

    //"op" is the operator you have to use, but you can use "add" to test
    int (*scan_operator)(int, int, int);
    scan_operator = op;
    //scan_operator = add;

    //Moodified to pass barriers 
    fill_args(ps_args, opts.n_threads, n_vals, input_vals, output_vals,
        opts.spin, scan_operator, opts.n_loops, &g_pthreadBarrier_t);

    // May need to do variable setup here to improve time.

    // Start timer
    auto start = std::chrono::high_resolution_clock::now();

    if (sequential)  {
        cout << "main: Using Sequential Prefix Sum: " << sequential << endl;  // debug statement
        cout << "main: Number of Values: " << n_vals << endl;

        //sequential prefix scan
        output_vals[0] = input_vals[0];
        for (int i = 1; i < n_vals; ++i) {
            //y_i = y_{i-1}  <op>  x_i
            output_vals[i] = scan_operator(output_vals[i-1], input_vals[i], ps_args->n_loops);
        }
    }
    else {
        cout << "main: Starting threads..." << endl;  // debug statement

        start_threads(threads, opts.n_threads, ps_args, compute_prefix_sum);

        // Wait for threads to finish
        join_threads(threads, opts.n_threads);

        cout << "main: Threads joined..." << endl;  // debug statement
    }

    //End timer and print out elapsed
    auto end = std::chrono::high_resolution_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time: " << diff.count() << std::endl;

    // Write output data
    write_file(&opts, &(ps_args[0]));

    cout << "main: Wrote output file" << endl;  // debug statement

    // Free other buffers
    free(threads);
    free(ps_args);
}
