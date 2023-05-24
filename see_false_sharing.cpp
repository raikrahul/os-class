// Compile using 
// clang++ see_false_sharing.cpp -std=c++17 -O3 -lbenchmark -lpthread -o false_sharing -Wall
// Please see Lesson 1 for setup and hello world.
//also see official documentation for google benchmarks


// There is redundancy in the examples, usage of benchmak lib is not standard.
// No cmake Lists are provided, just files.


#include <atomic>
#include <benchmark/benchmark.h>

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)
#define ARGS(N) ->Threads(N)->UseRealTime()

#define ARGS_1(N, M) ->Threads(N)->Arg(M)->UseRealTime()

constexpr size_t I = 1 << 12;
std::atomic<unsigned long> global_atomic;
std::atomic<unsigned long> global_atomic_array[8192];

static void BM_globalIncrAto(benchmark::State &state) {
    if (state.thread_index == 0) {
        global_atomic.store(0);
    }
    for (auto _ : state) {
        for (size_t i = 0; i < I; ++i) {
            REPEAT(benchmark::DoNotOptimize(++global_atomic););
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

// I have 12 threads in total [ 6*2 ] ,
// one can reduce the number of runs,
// it can also happen that no new thread is created
// depending on the N:M thread mapping
// in the user space threading library and kernel's support of threads

static void BM_LocalIncrAto(benchmark::State &state) {
    std::atomic<unsigned long> local_atomic(0);
    for (auto _ : state) {
        for (size_t i = 0; i < I; ++i) {
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

static void BM_Global_NotSharedAtomics(benchmark::State &state) {

    if (state.thread_index == 0) {
        for (auto it = std::begin(global_atomic_array);
                it <= std::end(global_atomic_array); it++) {
            it->store(0);
        }
    }
    const auto rank = state.range(0) * state.thread_index;

    for (auto _ : state) {
        for (size_t i = 0; i < I; ++i) {
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

    BENCHMARK(BM_Global_NotSharedAtomics)
    ->RangeMultiplier(2)
->Range(0, 512)
    ->Threads(1);
    BENCHMARK(BM_Global_NotSharedAtomics)
    ->RangeMultiplier(2)
->Range(0, 512)
    ->Threads(2);
    BENCHMARK(BM_Global_NotSharedAtomics)
    ->RangeMultiplier(2)
->Range(0, 512)
    ->Threads(4);
    BENCHMARK(BM_Global_NotSharedAtomics)
    ->RangeMultiplier(2)
->Range(0, 512)
    ->Threads(6);
    BENCHMARK(BM_Global_NotSharedAtomics)
    ->RangeMultiplier(2)
->Range(0, 512)
    ->Threads(8);
    BENCHMARK(BM_Global_NotSharedAtomics)
    ->RangeMultiplier(2)
->Range(0, 512)
    ->Threads(10);
    BENCHMARK(BM_Global_NotSharedAtomics)
    ->RangeMultiplier(2)
->Range(0, 512)
    ->Threads(12);

    // Always remember, a cache line need not be  a multiple of size of words.
    // If CPU needs one bytes, then the entire cache line with that single word
    // is loaded into the cache. NOT JUST THE the exact word
    // If one CPU acts on ONE word, that CPU cache line is loaded into
    // it's embeddded cache.

BENCHMARK_MAIN();
