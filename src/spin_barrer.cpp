#include <spin_barrier.h>


/************************
 * Your code here...    *
 * or wherever you like *
 ************************/

// When using header files definig your class definition, you must use the scope unary operator :: to specificy the class that the function implementaiton is for.
spin_barrier::spin_barrier(int nThreads) {
    // initialize a semaphore using unnamed semaphores to allocate to process memory.  Assuming our program is multi-threaded vs. multi-process, given we are using pthreads.  pshared will be 0, so it's not shared across processes.
    this->nThreads = nThreads;
    sem_init(&arrival, 0, 1);
    sem_init(&departure, 0, 0);
    counter = 0;
}

void spin_barrier::wait() {
    sem_wait(&arrival);  // if 0 wait, otherwise decrement from arrival value of 1.
    if (++counter < nThreads) {  // increment global atomic counter and check condition.
        sem_post(&arrival);  // increment arrival.
    }
    else {
        sem_post(&departure);  // increment departure
    }
    sem_wait(&departure);  // if 0 wait, otherwise decrement from departure value of 1.
    if (--counter > 0) {  // decrement global atomic counter and check condition.
        sem_post(&departure);  // increment departure
    }
    else {
        sem_post(&arrival);  // increment arrival.
    }
}

spin_barrier::~spin_barrier() {
    // destroy barrier when class object instance is deconstructed
    sem_destroy(&arrival);
    sem_destroy(&departure);
}

