# Prefix Scans and Barriers

Goals of this lab are:
- To learn about parallel prefix scan algorithms
- Compare their performance using different synchronization primitives
- Gain experience implementing your own barrier primitive
- Understand the performance tradeoffs made by your barrier implementation

Environment Notes: Ubuntu Guest VM on WSL2 with 4 cores and 4GB RAM running on an AMD64 8 Core (16 Logical Processor) CPU with 24 GB of RAM.

## Step 1: Parallel Implementation for Prefix Scan
Used Guy Blelloch Algorithm (+-Reduction and +-Prescan) to write a work-efficient parallel prefix scan and used the “Circuit representation of a work-efficient 16-input parallel prefix sum” to formulate an
approach to divide the dataset across threads on the shared / global memory array.

Below is the graph the execution time of the parallel implementation over a sequential prefix scan implementation as a function of the number of worker threads used. Vary from 2 to 32 threads in increments of 2. The loops was set to 100,000 (-l 100000).

![Step 1: Parallel Prefix Scan (Loop 100,000)](/markdown_assets/Step1.png)

Notes: 
- The graph is in speedup:
$$ Speedup = \frac{T_{serial}}{T_{parallel}} $$

When the loop is set to 100,000 loop operations, I observed that there is speedup as the program uses more threads (more parallel execution) on large datasets.  The speed up jumps significantly above 2.00, when the following is set for each dataset size:
- (1k dataset) When the number of threads are set to 4, I observed a speedup being maintained between 1.90 and 2.30 as the threads increase.
- (8k dataset) When the number of threads are set above 10, I observed a speedup being maintained betwen 2.60 and 2.90 as the threads increase.
- (16k dataset) When the number of threads are set above 4, I observed immediate speedup at 3.00 and it increases as more threads are added peaking a speedup of 5.74.

The speedup does level out as threads are increased.  I speculate that this is due to the behavior explained in Amdahl’s Law - _“the curve of speed up as the number CPU / threads goes up is going to have diminishing returns”_.

For smaller datasets like 64 values / inputs, I observed that there is no speedup, but slowdown when using Parallel Prefix Scan.  I believe I am observing the drawbacks for the overhead required to coordinate and orchestrate parallelism (extra operations decomposing the data, sync and setup threads and barriers) vs. using simple lightweight sequential operations for a single thread / processor.

Why do I see diminishing returns from the Speedup Trend Graph?  Amdah’s Law which is associated to strong scaling, explains that when the work is fixed, the parallel portion in the parallel prefix scan is also fixed, so speedup stops increasing when the number of CPUs and threads grows.

Also, the Blelloch Algorithm that I am using is asymptotically doing no more work than the sequential version, so again, the parallel portion / work is fixed.

For the slow down with smaller datasets, we are seeing the parallel overheads being the cause.
- Load balancing and ensuring threads are writing within their area of decomposition for the shared / global array (exclusive read / write for each thread).
- The overhead of creating threads
- The overhead of synchronizing threads

## Step 2: Playing With Operators (Loops)
Changed the amount of loops the operator does in various increments to simulate what happens to parallelism when the complexity of the operation varies.

![Step 2: Speedup by Loops for Thread of 4](/markdown_assets/Step2.png)

Notes:
-  This graph is based from 4 threads, because we notice significant speedup with it.

Observed the following inflection points for the 64, 1k, 8k, and 16k datasets:
- (64 dataset) The inflection point is around 100,000 loops.
- (1k dataset) The inflection point seems to be between 75,000 and 100,000.
- (8k and 16k dataset) The inflection piont is not immediate obvious, but could be between 10,000 and 100,000 loops.

The loop imitates operations that can take longer to run due to having more execution complexity.  When you add additional execution loops it adds additional execution time.  For sequential algorithms, when you decrease the number of loops, you significantly shorten the overall execution time of executing all operations in serial.  For example, if you had n operations that had a looping time of 5 microsecond, the total increase in execution time will be 5 microseconds*n.  Whereas with parallel processing, if the n operations can be done in parallel, the execution time will only be 5 seconds and will not be added across each n operations.

The most important characteristics of it is that if additional operation times are added it is added for each sequential operation, magnifying the impact on sequential execution time.  Whereas if there’s no additional operational time, it aligns with sequential and parallel performance methodologies that we can use the “best” serial algorithm as a baseline for parallel performance.  With it we can gauge if the parallel algorithm has the same performance time with no more work complexity.

## Step 3: Barrier Implementation
I implemented a semaphore barrier which uses a strong object (e.g., a semaphore) and a central manager to keep the threads waiting.  The semaphore barrier also uses an atomic counter to help keep the Semaphore Barrier code symmetrical between an arrival and departure semaphore object.

The pthread barrier is a counter-based global barrier that tracks how many threads that are left that haven’t entered the barrier.  Once all threads have entered the barrier, the last thread will trigger a broadcast to all threads in waiting / spin state to go ahead and proceed with execution.  The pthread barrier does not use any strong object as compared to a Semaphore barrier and instead uses a broadcast condition signal for the threads to stop waiting.

Scenarios where there is less contention on the semaphores would allow the semaphore barrier to perform better.  When there is high contention on semaphores, it’ll require a lot of overhead to communicate with the OS or runtime thread scheduler to possibly unlock threads from a sleep state when they’re waiting on a semaphore versus just “spin” waiting.  In addition, there is a propagation delay when using a semaphore-based barrier because they’re using strong objects to manage each threads barrier state as they work through the symmetric barrier.

A pathological state for semaphore barriers will be If threads fail to trigger a semaphore operation on arrival, departure, or counter, (e.g., due to thread failures) the threads will be stuck in the barrier because a thread did not set the semaphore in the correct state for threads to move through the barrier.

pthreads barrier will perform better with high contention scenarios, given the whole barrier wait method is locked and uses a remaining counter for how many minimum threads are left before sending a broadcast signal to currently waiting (spinning) threads versus using a two phase – two semaphore process.

Cases where a pthread barrier may not work well would be the all or nothing behavior of signal broadcasting to threads to stop waiting / spinning.  If the last thread or a few remaining threads are long running threads, it can hold up the whole thread group. 

![Step 3.a - Speedup by Loops for Thread of 4](/markdown_assets/Step3a.png)
![Step 3.b Loops Speedup Time for Thread of 4 Between Pthread Barrier and Semaphore Barrier](/markdown_assets/Step3b.png)

The parallel implementation with semaphore barrier performs slower than the pthread barrier.  Again, this is due to the semaphore (strong) object whose operations to read write to a global shared semaphore object is heavier and takes time which creates propagation delays.  Heavier because a semaphore is a synchronization object that requires integration with either the operating system or the runtime thread scheduler to address when threads are locked and go to sleep when they're waiting on a semaphore. 

The inflection points for larger datasets 8k values / inputs or more seem to have an inflection point closer to 1000 loops, whereas for datasets 1k values / inputs or less seem to have an inflection point closer to 100,000 loops.  For larger datasets, the behavior and inflection point are similar to pthread barriers for 4 threads.  For smaller datasets, the sequential algorithm is much faster without the heavier semaphore barrier thus pushing the inflection point more towards longer / complex operations.

For all datasets, being processed by 4 threads, the speedup for the semaphore barrier is less than the pthread barrier.  And this is in line with my expectation set by the lecture where it states as a con to semaphore barriers – propagation delays and using a strong object - semaphore.  Also read from [Performance test: sem_t v.s. dispatch_semaphore_t and pthread_once_t v.s. dispatch_once_t, Stack Overflow](https://stackoverflow.com/questions/3640853/performance-test-sem-t-v-s-dispatch-semaphore-t-and-pthread-once-t-v-s-dispat):  “sem_wait() and sem_post() are heavy weight synchronization facilities that can be used between processes. They always involve round trips to the kernel, and probably always require your thread to be rescheduled. They are generally not the right choice for in-process synchronization.”

If the number of threads increases for a parallel implementation using a semaphore barrier, the speedup will not be much and should expect worse performance than a pthread barrier.  An increase in thread count will increase contention on the semaphore, whereas in a pthread barrier a locked counter is used to decrement how many threads are remaining (simpler decrement operation).
Semaphore barriers are better for multi-process synchronization given its OS / runtime scheduler support over pthread barriers.
# Additional Notes
Use run_tests.py to capture multiple run times for analysis.  Ensure you've configured the script to run the specific tests you want.  To run, navigate to the directory and type the following in the terminal:

    python3 run_tests.py

To only run the prefix_scan application, run the following in the terminal:

    ./bin/prefix_scan -i /home/username/dev/repo/Lab1/tests/seq_64_test.txt -o ./tests/temp -n 2 -l 100000

prefix_scan application needs the following arguments to run:
- -n <number of threads> (int) (0 means sequential, not 1 pthread!)
- -i <absolute path to input file> (char *)
- -o <absolute path to output file> (char *)
- -l <number of loops that the operator will do> (int)
- -s <use your barrier implementation, else default to pthreads barrier> (Optional)


# References
- [Prefix sum, Wikipedia](https://en.wikipedia.org/wiki/Prefix_sum).
- [Prefix Sums and Their Applications, Guy E. Blelloch, Carnegie Mellon University](https://www.cs.cmu.edu/~guyb/papers/Ble93.pdf)
- [Performance test: sem_t v.s. dispatch_semaphore_t and pthread_once_t v.s. dispatch_once_t, Stack Overflow](https://stackoverflow.com/questions/3640853/performance-test-sem-t-v-s-dispatch-semaphore-t-and-pthread-once-t-v-s-dispat)
