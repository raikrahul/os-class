# Getting started with micro bench marks 

Books/Blogs/Speakers/Advice tells - *not all memory is same* . 
Some cost _more cash_ and others _aren't  in the cache flow_. 
After this write up, we should be able to - 
 - Set up micro bench mark.
 - Use it to run some hello benchmark code. 
 - Measure some memory.
 - Measure some memory at 10 thousand feet.
 - Publish results and see it on their own computer.

*I am using a decade old laptop [ not a virtual machine ] [ purchased before 2010 ] for the runs.*

# Set Up  - 

On your ubuntu machine, please follow the instructions mentioned at 
https://github.com/google/benchmark#installation and be able to run the ``Hello World`` !? for it.
Please make sure you are able to do the *global install*. 
For my machine I will list out the the non bench mark setup :  

 - ubuntu  : ``$shell>cat /etc/os-release``
	  ``  
	NAME="Ubuntu"``  
	``VERSION="20.04 LTS (Focal Fossa)" ``
	``ID=ubuntu``  
	``D_LIKE=debian``  
	``PRETTY_NAME="Ubuntu 20.04 LTS"``  
	``VERSION_ID="20.04"``  
	
- cmake : ```$shell> cmake --version```
		``cmake version 3.16.3  CMake suite maintained and supported by Kitware ...``

- clang++: ``$shell>clang++ -v ``
		 ``clang version 10.0.0-4ubuntu`` 
		``Target: x86_64-pc-linux-gnu``  
		``Thread model: posix``
- CPU information: ``$shell> cat /proc/cpuinfo``
		``model name : Intel(R) Core(TM)2 Duo CPU T5670 @ 1.80GHz `` 
		``stepping : 13``
		``microcode : 0xa4``
		``cpu MHz : 913.479``
		``cache size : 2048 KB``
		``...............................``  ``.... ``
 **offline!**

## Seeing Hello World  

Let us type the following in a file `b1.cpp` and try to compile with `clang++ b1.cpp`

    // b1.cpp
    #include <benchmark/benchmark.h>
    BENCHMARK_MAIN(); 
We must get the following errors 

    in function `main'
    undefined reference to `benchmark::Initialize(int*, char**)'
    undefined reference to `benchmark::ReportUnrecognizedArguments(int, char**)
    undefined reference to `benchmark::RunSpecifiedBenchmarks()'
    in function `__static_initialization_and_destruction_0(int, int)':
    in function `__static_initialization_and_destruction_0(int, int)':
    in function `__static_initialization_and_destruction_0(int, int)'
    
This is intentionally done so that we know that we have succeeded in failing the initial test. It is not hard to figure out how to fix this, or if you are reading this blog, you might have already done. This error demonstration was for the person who would get started right with bench marking, and would probably give up. 

So, let's type ``$shell> clang++ b1.cpp -lbenchmark``

We must get the following errors [ This is intentionally shown for the absolute naive ] 

     in function 
     std::thread::thread<void (*)(benchmark::internal::BenchmarkInstance const*, 
     unsigned long, int, benchmark:
     :internal::ThreadManager*), benchmark::internal::BenchmarkInstance const*, 
     unsigned long&, int, benchmark::internal::ThreadManager*, void>
     (void (*&&)	(benchmark::internal::BenchmarkInstance const*, unsigned long, 
     int, benchmark::internal::ThreadManager*), 
     benchmark::internal::BenchmarkInstance const*&&, unsigned long&, int&&, 
     benchmark::internal::ThreadManager*&&)':
     undefined reference to `pthread_create'
     
So, let's type ``$shell> clang++ b1.cpp -lbenchmark -lpthread``

We must get the `a.out` executable ``$shell> l``
Let's run the executable ``$shell> ./a.out``
We must get the following output ``Failed to match any benchmarks against regex: .``

## If you got this far, you have patience

Let us write some code, and let us understand memory hierarchy. 
One could review some slides I found with some examples - [ Intentionally redirected ] 
[http://users.ece.cmu.edu/~koopman/lectures/index.html#548](http://users.ece.cmu.edu/~koopman/lectures/index.html#548)
Or we can read up the wikipedia entries for each of the topics. 
Let's *add the* *following content and nothing* else in the file `b1.cpp`

    auto how_long_longs_take(benchmark::State& state)
    {
    	volatile long some_long{0}; 
    	// read up _volatile_ on wikipedia 
    	while ( state.KeepRunning())
    	{
    		benchmark::DoNotOptimize(++some_long);
    	}
    }
Let's try to build and *run*  `$shell> clang++ b1.cpp -lbenchmark -lpthread -std=c++17 ; ./a.out`
We must get the following message - We have see this error before too. 

    Failed to match any benchmarks against regex: .

Lets *add the content* which will make the unit run [ I am taking slow, intentionally for the reader who is lost is the setup and will probably give up ]. 

    // Register the function as a benchmark
    BENCHMARK(how_long_longs_take);

Let's try to build and *run*  `$shell> clang++ b1.cpp -lbenchmark -lpthread -std=c++17 ; ./a.out`
Make sure we record our output - *take a screenshot* [ not just copy the content in a text file ] and put it or show it somewhere. 

I will paste the output for a decade old machine - 

    Run on (2 X 1801 MHz CPU s)                                                                                                                                                                                     
    	CPU Caches:                                                                                                                                                                                                     
    	  L1 Data 32 KiB (x2)                                                                                                                                                                                           
    	  L1 Instruction 32 KiB (x2)                                                                                                                                                                                    
    	  L2 Unified 2048 KiB (x1)                                                                                                                                                                                      
    	Load Average: 0.77, 0.65, 0.68                                                                                                                                                                                  
    	***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.                                                                                          
    	***WARNING*** Library was built as DEBUG. Timings may be affected.                                                                                                                                              
    	--------------------------------------------------------------                                                                                                                                                  
    	Benchmark                    Time             CPU   Iterations                                                                                                                                                  
    	--------------------------------------------------------------                                                                                                                                                  
    	how_long_longs_take       10.6 ns         10.6 ns     63105717 

We must be able to *understand* each of the warnings, [ if you get one ]. 

     1 - ***WARNING*** CPU scaling is enabled .... .... ....
     
 Fix -    [https://github.com/google/benchmark#disabling-cpu-frequency-scaling](https://github.com/google/benchmark#disabling-cpu-frequency-scaling)
 
Read Up  - [(https://www.kernel.org/doc/html/v4.14/admin-guide/pm/cpufreq.html)](https://www.kernel.org/doc/html/v4.14/admin-guide/pm/cpufreq.html)

2 - `***WARNING*** Library was built as DEBUG. Timings may be affected`

Fix -    [https://github.com/google/benchmark#debug-vs-release](https://github.com/google/benchmark#debug-vs-release)

***Quiz***

 - Does fixing the CPU  scaling setting changes the output *?* 
-	Does changing to release version [ we might need to build ]  changes the output *?*
-	How will the output change when run the above tests on a virtual machine with same/different specs *?*
-	Why do we get a different number of iterations each time we run the `./a.out`?  Try running `./a.out` in another  shell / different shell/ wrapped in a script/ via ssh/another emulator.  
-	 In the function `how_long_longs_take`; try running the while loop with counter, ie - run it a fixed number of times - like 4200?    	What do you observe in output after this modification in source and building/running it? Why?  Also, make sure you undo the change before seeing next question.

-	What happens when you change the while condition to just `while(1)` and leave the rest of the contents of the file as they were?  

## I applaud your  perseverance if you gave gotten this far! 

What do  the `iterations` in the output  mean [ when we ran `./a.out` ] ?
Explanation - [https://github.com/google/benchmark#runtime-and-reporting-considerations](https://github.com/google/benchmark#runtime-and-reporting-considerations)
What do the `time` and `CPU` mean in the output?  Try reading about `cpu time` 

***Quiz***

 - *Compile*  the file `b1.cpp` with the flag -O3 and *run* it again. What changes you see in the number of `iterations` and in the `time` in the output?  [ `clang++ b1.cpp -lbenchmark -lpthread  -O3 -std=c++17; ./a.out` 
 - What should one do to trick the compiler to not be able to guess that the loop is just trivial increment of a `long` type? What is the use of  `benchmark::DoNotOptimize`
 
 We will now want to repeat the increments, so that we amortise over the operations of increments. One increment per loop is indistinguishable from  the time it takes to just run on iteration of the loop. 
 
 >We will see the how to test other types with various different loads [ measure X Mib with Y bytes per type ]  in the second Lesson. 	

