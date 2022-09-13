#ifndef _SPIN_BARRIER_H
#define _SPIN_BARRIER_H

#include <pthread.h>
#include <iostream>

// Additional headers / namespaces
#include <semaphore.h>

class spin_barrier {
public:
    // Constructor with thread argument
    spin_barrier(int);
    // method to call wait
    void wait();
    // Deconstruct the class object instance
    ~spin_barrier();
private:
    int nThreads;
    sem_t arrival;
    sem_t departure;
    int counter;
};

class pthread_barrier {
    public:
        // Constructor with thread argument
        pthread_barrier(int);
        // method to call wait
        void wait();
        // Deconstruct the class object instance
        ~pthread_barrier();
    private:
        int nThreads;
        pthread_barrier_t pthreadBarrier;
};

#endif
