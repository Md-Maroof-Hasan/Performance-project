#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_VERTICES 50000
#define NUM_EDGES 5000000
#define INITIAL_CAPACITY 16
#define NUM_RUNS 10   // 🔥 NEW: number of samples

// 🔷 Latency storage
long long *latencies;
int lat_count = 0;
int lat_capacity = INITIAL_CAPACITY;

// 🔷 Store latency
void store_latency(long long value) {
    if (lat_count == lat_capacity) {
        lat_capacity *= 2;
        latencies = (long long*) realloc(latencies, lat_capacity * sizeof(long long));
    }
    latencies[lat_count++] = value;
}

// 🔷 Timed malloc
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

// 🔷 Print latency stats
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

// 🔷 Main
int main() {
    latencies = (long long*) malloc(lat_capacity * sizeof(long long));

    int V = NUM_VERTICES;
    int E = NUM_EDGES;

    long long final_sum = 0; // 🔥 store last checksum

    // 🔥 NEW: repeat multiple times
    for (int run = 0; run < NUM_RUNS; run++) {

        int *matrix = (int*) timed_malloc((long long)V * V * sizeof(int));

        // Initialize
        for (long long i = 0; i < (long long)V * V; i++) {
            matrix[i] = 0;
        }

        // Add edges
        for (int i = 0; i < E; i++) {
            int src = i % V;
            int dest = (i * 2) % V;
            matrix[src * V + dest] = 1;
        }

        // Traverse
        long long sum = 0;
        for (long long i = 0; i < (long long)V * V; i++) {
            sum += matrix[i];
        }

        final_sum = sum; // just keep last

        free(matrix);
    }

    // Print once (avoid clutter)
    printf("Traversal checksum: %lld\n", final_sum);

    print_latency_stats();
    printf("no of vertices = %d",NUM_VERTICES);
    printf("no of edges = %d \n",NUM_EDGES);

    free(latencies);

    return 0;
}
