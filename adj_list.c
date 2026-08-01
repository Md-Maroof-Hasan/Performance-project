#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_VERTICES 5000
#define NUM_EDGES 500000
#define INITIAL_CAPACITY 16

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

// 🔷 Node
typedef struct Node {
    int dest;
    struct Node* next;
} Node;

// 🔷 Graph
typedef struct Graph {
    int vertices;
    Node** adj;
} Graph;

// 🔷 Create graph
Graph* createGraph(int vertices) {
    Graph* graph = (Graph*) timed_malloc(sizeof(Graph));
    graph->vertices = vertices;

    graph->adj = (Node**) timed_malloc(vertices * sizeof(Node*));

    for (int i = 0; i < vertices; i++) {
        graph->adj[i] = NULL;
    }

    return graph;
}

// 🔷 Create node
Node* createNode(int dest) {
    Node* newNode = (Node*) timed_malloc(sizeof(Node));
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// 🔷 Add edge
void addEdge(Graph* graph, int src, int dest) {
    Node* newNode = createNode(dest);
    newNode->next = graph->adj[src];
    graph->adj[src] = newNode;
}

// 🔷 Traverse graph
void traverseGraph(Graph* graph) {
    for (int i = 0; i < graph->vertices; i++) {
        Node* curr = graph->adj[i];
        while (curr != NULL) {
            curr = curr->next;
        }
    }
}

// 🔷 Free graph
void freeGraph(Graph* graph) {
    for (int i = 0; i < graph->vertices; i++) {
        Node* curr = graph->adj[i];
        while (curr != NULL) {
            Node* temp = curr;
            curr = curr->next;
            free(temp);
        }
    }
    free(graph->adj);
    free(graph);
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

    Graph* graph = createGraph(NUM_VERTICES);

    // 🔷 Build graph
    for (int i = 0; i < NUM_EDGES; i++) {
        addEdge(graph, i % NUM_VERTICES, (i * 2) % NUM_VERTICES);
    }

    // 🔷 Traverse
    traverseGraph(graph);

    // 🔷 Free
    freeGraph(graph);

    // 🔷 Print stats
    print_latency_stats();

    free(latencies);

    return 0;
}
