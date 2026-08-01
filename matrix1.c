#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INITIAL_CAPACITY 16

// 🔷 Latency storage
long long *latencies;
int lat_count = 0;
int lat_capacity = INITIAL_CAPACITY;

// 🔷 Store latency
void store_latency(long long value) {
    if (lat_count == lat_capacity) {
        lat_capacity *= 2;
        latencies = realloc(latencies, lat_capacity * sizeof(long long));
    }
    latencies[lat_count++] = value;
}

// 🔷 Timed malloc wrapper
void* timed_malloc(size_t size) {
    struct timespec t1, t2;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    void* ptr = malloc(size);
    clock_gettime(CLOCK_MONOTONIC, &t2);

    long long latency =
        (t2.tv_sec - t1.tv_sec) * 1000000000LL +
        (t2.tv_nsec - t1.tv_nsec);

    store_latency(latency);

    return ptr;
}

// 🔷 Print stats
void print_latency_stats() {
    if (lat_count == 0) return;

    long long min = latencies[0];
    long long max = latencies[0];
    long long sum = 0;

    for (int i = 0; i < lat_count; i++) {
        if (latencies[i] < min) min = latencies[i];
        if (latencies[i] > max) max = latencies[i];
        sum += latencies[i];
    }

    double avg = (double) sum / lat_count;
    double total_us = sum / 1000.0;

    printf("\n--- MALLOC LATENCY STATS ---\n");
    printf("Total malloc calls: %d\n", lat_count);
    printf("Min latency: %lld ns\n", min);
    printf("Max latency: %lld ns\n", max);
    printf("Avg latency: %.2f ns\n", avg);
    printf("Total allocation time: %.2f microseconds\n", total_us);
}

// 🔷 Main matrix program
int main() {
    latencies = (long long*) malloc(lat_capacity * sizeof(long long));

    int n = 20000;

    // 🔹 Allocate matrix (row-wise)
    int **matrix = (int**) timed_malloc(n * sizeof(int*));

    for (int i = 0; i < n; i++) {
        matrix[i] = (int*) timed_malloc(n * sizeof(int));
    }

    // 🔹 Simple computation
    long long sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = i + j;
            sum += matrix[i][j];
        }
    }

    printf("Sum: %lld\n", sum);

    // 🔹 Free memory
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);

    // 🔹 Print latency stats
    print_latency_stats();

    free(latencies);
    return 0;
}
