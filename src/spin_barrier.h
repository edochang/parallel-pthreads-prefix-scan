#ifndef _SPIN_BARRIER_H
#define _SPIN_BARRIER_H

#include <pthread.h>
#include <iostream>
#include <atomic>

// Additional headers / namespaces
#include <semaphore.h>

using namespace std;

class spin_barrier {
public:
    // Constructor with thread argument
    spin_barrier(int nThreads);
    // method to call wait
    void wait();
    // Deconstruct the class object instance
    ~spin_barrier();
private:
    int nThreads;
    sem_t arrival;
    sem_t departure;
    atomic<int> counter;
};

#endif
