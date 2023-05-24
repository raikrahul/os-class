
# False Sharing

I hope the *google micro benchmark* has been set up has been done and the reader has some search about the topic.
Please see *Lesson 1* in the same repository to set up the tools and run some hello world applications. 


# Initial Setups

Let us create a file called `see_false_sharing.cpp` and populate it with 

    #include  <benchmark/benchmark.h>
    #include  <atomic>
    
    #define  REPEAT2(x) x x
    #define  REPEAT4(x) REPEAT2(x) REPEAT2(x)
    #define  REPEAT8(x) REPEAT4(x) REPEAT4(x)
    #define  REPEAT16(x) REPEAT8(x) REPEAT8(x)
    #define  REPEAT32(x) REPEAT16(x) REPEAT16(x)
    #define  REPEAT(x) REPEAT32(x)
    #define  ARGS(N)  ->Threads(N)->UseRealTime()
    
    constexpr  size_t  I = 1 << 12;
    std::atomic<unsigned  long> global_atomic;

	static void BM_globalIncrAto(benchmark::State &state) 
	{
		if (state.thread_index == 0) 
		{
		    global_atomic.store(0); // this could be removed later
	    }
		for (auto _ : state) 
		{
		    for (size_t i = 0; i < I; ++i)
		    {
		      REPEAT(
		          benchmark::DoNotOptimize(++global_atomic);
		          );
		     }
    // all barriers done
	    }
	}
    BENCHMARK(BM_globalIncrAto) ARGS(1);
    BENCHMARK(BM_globalIncrAto) ARGS(2);
    BENCHMARK(BM_globalIncrAto) ARGS(4);
    BENCHMARK(BM_globalIncrAto) ARGS(8);
    BENCHMARK(BM_globalIncrAto) ARGS(10);
    BENCHMARK(BM_globalIncrAto) ARGS(12);
    // I have a total of 12 threads in total [ 6*2 ] ,
    // one can reduce the number of runs, 
    //it can also happen that no new thread is created 
    //depending on the N:M thread mapping 
    //in the user space threading library and kernel's support of threads
    
	BENCHMARK_MAIN();
    
   
   Now one could compile, link, run, like -  [ Please see *Lesson 1*] 
    `clang++ see_false_sharing.cpp -std=c++17 -O3 -lbenchmark -lpthread -o false_sharing`
 
   The point of this exercise is to know how much `time` can updating a global, shared, and protected variable [ take a note of the size of the variable ] can take.

Now, one could introduce a per function's local atomic variable and measure the cost of operations, by inserting into the file `see_false_sharing.cpp`

    static void BM_LocalIncrAto(benchmark::State &state) 
    {
	    std::atomic<unsigned long> local_atomic(0);
	    
	    for (auto _ : state) 
	    {
	        for (size_t i = 0; i < I; ++i) 
	        {
	            REPEAT(

	                benchmark::DoNotOptimize(++local_atomic););
		    }
		}
    }
     BENCHMARK(BM_LocalIncrAto) ARGS(1);
     BENCHMARK(BM_LocalIncrAto) ARGS(2);
     BENCHMARK(BM_LocalIncrAto) ARGS(4);
     BENCHMARK(BM_LocalIncrAto) ARGS(8);
     BENCHMARK(BM_LocalIncrAto) ARGS(10);
     BENCHMARK(BM_LocalIncrAto) ARGS(12);

   One can compile, link, with the same command as above and can run the binary to see the time difference between the two types of atomic variables.
   
 ***Quiz***

Try changing to normal, non atomic variables in both the benchmarks, and change  in just one, while keeping the other atomic. 
Do you see time differences in the real time, in each of the change pairs you made? 


 

## Having unprotected access is not same has having unprotected data 
Let's think - 
In what cases would atomic operations, for the same variables across all threads? 
In what cases will it make sense for  threads to have an independent copy of their own variable? Think for both non atomic and atomic variables. 

In the file created, append the following global array and a new macro.

_We will want to calculate a pattern [a geometric progression] of indices into the array of atomics, each element of which is 8 bytes wide_

_*We will make the threads operate on the pattern we figured out*_, for each run of the threads, 

One can change the indices, but one can risk into getting into segmentaion fault, or not see effects of loads of non _essential atomics_

```c

#define  ARGS_1(N, M)  ->Threads(N)->Arg(M)->UseRealTime()

std::atomic<unsigned long> global_atomic_array[8192];

//.. .. .. 
// .. .. .. 

```

***Quiz***

If we hit the same parts of the array all the time, just to read, but you do via different threads, it  should be conceptually the same to which of the tests already typed above?

If we hit different parts of the array at all times, just to read, but you do via the different threads, it  should be conceptually the same to which of the tests already typed above?

*What about writes,* instead of read, for each of the above questions?

Paste the following in the file; then compile, link, and run the binary.

```c
static void BM_Global_NotSharedAtomics(benchmark::State & state)
{
    if (state.thread_index == 0)
    {
        for (auto it = std::begin(global_atomic_array); it <= std::end(global_atomic_array); it++)
        {
            it -> store(0);
        }
    }
    const auto rank = state.range(0) * state.thread_index;
    for (auto _: state)
    {
        for (size_t i = 0; i < I; ++i)
        {
            REPEAT(
            benchmark::DoNotOptimize(++global_atomic_array[rank]););
        }
    }
}
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(1, 0);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(2, 0);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(4, 0);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(8, 0);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(10, 0);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(12, 0);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(1, 1);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(2, 1);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(4, 1);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(8, 1);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(10, 1);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(12, 1);
```
***Quiz***

Observe the results very carefully, and Re read the previous Quiz. 
Think, if we do this experiment many number of times, what could be said about the minimum expected time needed for

 - each of the threads updating a memory location  [ atomic ] 
 - each of the threads updating a memory location [ non atomic ]
 - each of the threads updating a memory location, exclusive to that thread's stack.
 - each of the threads exclusively updating a memory location, global to the program

Let us add  the following into the file.
Just try to multiply both the numbers passed into the ARG, each of the thread computes a `const` ! We could actually randomise the  area access for each thread. 

```c
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(1, 2);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(2, 2);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(4, 2);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(8, 2);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(10, 2);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(12, 2);


BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(1, 8);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(2, 8);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(4, 8);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(8, 8);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(10, 8);
BENCHMARK(BM_Global_NotSharedAtomics) ARGS_1(12, 8);
```

***Quiz***

What output difference is VISIBLE now? What pattern can one infer?  What happens if you read all the numbers aloud? 

Let's now add the following to the file, compile and run.

```c
BENCHMARK(BM_Global_NotSharedAtomics)->RangeMultiplier(2)->Range(0, 512)->Threads(1);
BENCHMARK(BM_Global_NotSharedAtomics)->RangeMultiplier(2)->Range(0, 512)->Threads(2);
BENCHMARK(BM_Global_NotSharedAtomics)->RangeMultiplier(2)->Range(0, 512)->Threads(4);
BENCHMARK(BM_Global_NotSharedAtomics)->RangeMultiplier(2)->Range(0, 512)->Threads(6);
BENCHMARK(BM_Global_NotSharedAtomics)->RangeMultiplier(2)->Range(0, 512)->Threads(8);
BENCHMARK(BM_Global_NotSharedAtomics)->RangeMultiplier(2)->Range(0, 512)->Threads(10);
BENCHMARK(BM_Global_NotSharedAtomics)->RangeMultiplier(2)->Range(0, 512)->Threads(12);
```
***Quiz***

What output difference is VISIBLE now? What pattern can one infer?  What happens if you read all the numbers aloud?  Can you read out aloud all the numbers as soon as they occur on the terminal? 

 
## Summary	

If someone has read this far, his/her patience is appreciated.

We learned

 - The impact of atomic variables
 - When one sees the impact of atomic variables, why can they be as slow as lock guard 
 - What happens if multiple atomic variables are nearby, while noticing, that an operation to an atomic variable is non uninterruptible. What cost does it incur?
 - Why should false sharing be harder to detect on non atomic variables? 

Please see,

 https://github.com/raikrahul/Learn/blob/master/see_false_sharing.cpp
 
 for the most recent version of the code examples. I avoid posting entire cmake projects and lists, needing to focus on just the bare minimum tools to demonstrate false sharing.

Thanks, We will see another Lesson on costs of simpler operations, like creating a thread, or implementing a critical section using a futex's wrapper. 

