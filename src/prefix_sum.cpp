#include "prefix_sum.h"
#include "helpers.h"

// Additional headers / namespaces
#include <math.h>

using namespace std;

void* compute_prefix_sum(void *a)
{
    prefix_sum_args_t *args = (prefix_sum_args_t *)a;

    /************************
     * Your code here...    *
     * or wherever you like *
     ************************/
    // Student Note:  This function will implement the balanced binary tree algorithm provided by the "Prefix Sum and Their Applications" - Guy E Blelloch (1987)" publication.

    // Create local variables from the thread args.
    int *inputArray = args->input_vals;
    int *outputArray = args->output_vals;
    int nValues = args->n_vals;
    int nThreads = args->n_threads;
    int nTid = args->t_id;
    int (*scan_op)(int, int, int) = args->op;
    int nLoops = args->n_loops;
    pthread_barrier_t *pthreadBarrier = args->pthreadBarrier;

    cout << "pfx: Tid[" << nTid << "] Hello World" << endl;  // debug statement

    // Calculate the padding for non-power of 2 elements, to use the balanced binary tree algorithm.
    int nPowerPad = ceil(log2(nValues));
    int nPaddedValues = pow(2,nPowerPad);
    //int nPad = nPaddedValues - nValues;
    // debug block
    /*
    cout << "pfx: Tid[" << nTid << "] nPowerPad is: " << nPowerPad << endl;
    cout << "pfx: Tid[" << nTid << "] nPaddedValues is: " << nPaddedValues << endl;
    cout << "pfx: Tid[" << nTid << "] nPad is: " << nPad << endl;
    */

    // Set thread scope
    // Note: use nPaddedValues vs. nValues
    int nTidRange = ceil(nPaddedValues/nThreads);
    int nTidScopeIndexStart = nTid * nTidRange;
    int nTidScopeIndexEnd = nTidScopeIndexStart + nTidRange;

    cout << "pfx: Tid[" << nTid << "] Starting +-reduce operations with scope[" << nTidScopeIndexStart << ", " << nTidScopeIndexEnd << "]" << endl; // debug statement


    /*  + - Reduce Operation (a.k.a. Upsweep Alogrithm) for Exclusive Read, Exlusive Write (EREW) PRAM
        for d from 0 to (lg n)−1 
            in parallel for i from 0 to n−1 by 2^(d+1)
                a[i+2^(d+1)−1] ← a[i+2^d−1] + a[i+2^(d+1)−1]
    */

    // Note: use nPaddedValues vs. nValues
    int stride = 0;
    for (int d = 0; d <= log2(nPaddedValues)-1; d++) {
        //cout << "pfx: Tid[" << nTid << "] Reduce operations level: " << d << " of " << log2(nPaddedValues)-1 << endl;  // debug statement
        stride = pow(2,d+1);
        for (int i = 0; nPaddedValues-1; i+=stride) {
            int index2 = i + stride - 1;
            int index1 = i + pow(2,d) - 1;
            if ((nTidScopeIndexStart < index2) && (index2 < nTidScopeIndexEnd)) {
                if (d == 0) {
                    if (index2 >= nValues){
                        outputArray[index1] = 0;  // Pad with 0
                        outputArray[index2] = scan_op(inputArray[index1], 0, nLoops);  // Pad with 0
                    } else {
                        // Added index 1 value to output array.
                        outputArray[index1] = inputArray[index1];
                        outputArray[index2] = scan_op(inputArray[index1], inputArray[index2], nLoops);
                    }
                } else {
                    outputArray[index2] = scan_op(outputArray[index1], outputArray[index2], nLoops);
                }
            } 
            if (index2 >= nTidScopeIndexEnd) {
                break;
            }
        }
        pthread_barrier_wait(pthreadBarrier);
    }

    cout << "pfx: Tid[" << nTid << "] Starting +-prescan operations with scope[" << nTidScopeIndexStart << ", " << nTidScopeIndexEnd << "]" << endl;  // debug statement

    /*  + - Prescan Operation (a.k.a. Downsweep Alogrithm) for Exclusive Read, Exlusive Write (EREW) PRAM
        % Algorithm is for inclusive prescan.
        for d from (lg n)−1 downto 1 
            in parallel for i from 2^(d-1) to n−1 by 2^(d) 
                a[i+2^d+2^(d-1)−1] ← a[i+2^d−1] + a[i+2^d+2^(d-1)−1]
    */

    // Note: use nPaddedValues vs. nValues
    if (nTid != nThreads - 1) {
        nTidScopeIndexEnd++;
    }
    stride = 0;
    for (int d = log2(nPaddedValues)-1; d >= 1; d--) {
        //cout << "pfx: Tid[" << nTid << "] Prescan operations level: " << d << " of " << log2(nPaddedValues)-1 << endl;  // debug statement
        stride = pow(2,d);
        int nIPad = pow(2,d-1);
        for (int i = nIPad; i < nPaddedValues-1; i+=stride) {
            int index2 = i + stride - 1;
            
            if (index2 >= nPaddedValues) {
                //cout << "pfx: Tid[" << nTid << "]: Step (" << d << ") - skip index 2 (" << index2 << ") with nPaddedValues: " << nPaddedValues << endl;  // debug statement

                continue;  // skip current iteration and increment i; condition will stop the loop.
            } else {
                int index1 = i + stride - nIPad - 1;
                if ((nTidScopeIndexStart < index2) && (index2 < nTidScopeIndexEnd)) {
                    //cout << "pfx: Tid[" << nTid << "]: Step (" << d << ") Prescan with pair (" << index1 << ", " << outputArray[index1] << ") and (" << index2 << ", " << outputArray[index2] << endl;  // debug statement
                    
                    outputArray[index2] = scan_op(outputArray[index1], outputArray[index2], nLoops);
                    
                    //cout << "pfx: Tid[" << nTid << "]: Step (" << d << ") Prescan - index("<< index2 << ") with value: " << outputArray[index2] << endl;  // debug statement
                } 
                if (index2 >= nTidScopeIndexEnd) {
                    break;
                }
            }
        }
        pthread_barrier_wait(pthreadBarrier);
    }

    /*  + - Prescan Operation (a.k.a. Downsweep Alogrithm) for Exclusive Read, Exlusive Write (EREW) PRAM
        % Algorithm is for exclusive prescan.
        a[n−1] ← 0                                      %Settheidentity 
        for d from (lg n)−1 downto 0 
            in parallel for i from 0 to n−1 by 2^(d+1) 
                t ← a[i+2^d−1]                          %Saveintemporary 
                a[i+2^d−1] ← a[i+2^(d+1)−1]             %Setleftchild 
                a[i+2^(d+1)−1] ← t + a[i+2^(d+1)−1]     %Setrightchild
    */

    /*
    // Set Identity
    //int nSumTemp = outputArray[nPaddedValues-1];
    if (nTid == 0) {
        outputArray[nPaddedValues-1] = 0;
    }

    pthread_barrier_wait(pthreadBarrier);

    for (int d = log2(nPaddedValues)-1; d >= 0 ; d--) {
        cout << "pfx: Tid[" << nTid << "] Reduce operations level: " << d << " of " << log2(nPaddedValues)-1 << endl;  // debug statement
        stride = pow(2,d+1);
        for (int i = 0; i <= nPaddedValues-1; i+=stride) {
            int index2 = i + stride - 1;
            int index1 = i + pow(2,d) - 1;
            int nTemp = outputArray[index1];  // Issue: Facing cache coherence and interleaving issues.  Open question on how to solve this.

            pthread_barrier_wait(pthreadBarrier);  // Everyone needs to capture the temp value locally before modifying.

            if ((nTidScopeIndexStart < index1) && (index1 < nTidScopeIndexEnd)) {
                outputArray[index1] = outputArray[index2];
                cout << "pfx: Tid[" << nTid << "]: Step (" << d << ") Prescan - index("<< index1 << ") with value: " << outputArray[index2] << endl;  // debug statement
            }

            pthread_barrier_wait(pthreadBarrier);  // Everyone needs to wait for left child to be set.

            if ((nTidScopeIndexStart < index2) && (index2 < nTidScopeIndexEnd)) {
                outputArray[index2] = scan_op(nTemp, outputArray[index2], nLoops);
                cout << "pfx: Tid[" << nTid << "]: Step (" << d << ") Prescan - index("<< index2 << ") with value: " << outputArray[index2] << endl;  // debug statement
            }
        }
        pthread_barrier_wait(pthreadBarrier);  // Everyone needs to sync-up after modifying the shared / global memory.
    }
    */

    // pthread_barrier_wait(pthreadBarrier);  // Everyone needs to wait before exiting their function.
    
    cout << "pfx: Tid[" << nTid << "] End of prefix_sum.cpp." << endl;

    return 0;
}
