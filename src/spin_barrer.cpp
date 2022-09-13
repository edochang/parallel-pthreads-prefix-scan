#include <spin_barrier.h>


/************************
 * Your code here...    *
 * or wherever you like *
 ************************/

// When using header files definig your class definition, you must use the scope unary operator :: to specificy the class that the function implementaiton is for.
pthread_barrier::pthread_barrier(int nThreads) {
    // Setup pthread Barriers with default (NULL) attributes to wait for n_threads
    pthread_barrier_init(&pthreadBarrier, NULL, nThreads);
}

void pthread_barrier::wait() {
    pthread_barrier_wait(&pthreadBarrier);
}

pthread_barrier::~pthread_barrier() {
    // destroy barrier when class object instance is deconstructed
    pthread_barrier_destroy(&pthreadBarrier);
}

