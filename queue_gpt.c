#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef struct Queue {
    Node *front;
    Node *rear;
    size_t size;
} Queue;

/* Global allocation latency tracking */
static uint64_t alloc_count = 0;
static uint64_t total_latency_ns = 0;

/* Measure a single malloc() latency in nanoseconds */
static void *timed_malloc(size_t size) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    void *ptr = malloc(size);

    clock_gettime(CLOCK_MONOTONIC, &end);

    uint64_t start_ns = (uint64_t)start.tv_sec * 1000000000ULL + start.tv_nsec;
    uint64_t end_ns   = (uint64_t)end.tv_sec   * 1000000000ULL + end.tv_nsec;
    uint64_t latency  = end_ns - start_ns;

    alloc_count++;
    total_latency_ns += latency;

    return ptr;
}

/* Queue operations */
Queue *create_queue(void) {
    Queue *q = (Queue *)timed_malloc(sizeof(Queue));
    if (!q) {
        perror("malloc queue");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

void enqueue(Queue *q, int value) {
    Node *node = (Node *)timed_malloc(sizeof(Node));
    if (!node) {
        perror("malloc node");
        exit(EXIT_FAILURE);
    }
    node->value = value;
    node->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
    q->size++;
}

int dequeue(Queue *q) {
    if (q->front == NULL) {
        fprintf(stderr, "Dequeue from empty queue!\n");
        exit(EXIT_FAILURE);
    }
    Node *temp = q->front;
    int value = temp->value;

    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    q->size--;

    free(temp);
    return value;
}

/* Free remaining nodes and queue itself */
void free_queue(Queue *q) {
    while (q->front != NULL) {
        dequeue(q);
    }
    free(q);
}

/* Periodic reporting */
void print_stats(const char *label) {
    double avg = (alloc_count == 0) ? 0.0 : (double)total_latency_ns / alloc_count;
    printf("[%s] allocations=%llu total_latency=%llu ns avg_latency=%.2f ns\n",
           label,
           (unsigned long long)alloc_count,
           (unsigned long long)total_latency_ns,
           avg);
}

int main(void) {
    Queue *q = create_queue();

    const int total_enqueues = 100000;
    const int batch_size = 1000;   /* enqueue batch size per cycle */
    const int cycles = total_enqueues / batch_size;

    int value = 0;

    for (int c = 1; c <= cycles; c++) {
        /* Enqueue batch */
        for (int i = 0; i < batch_size; i++) {
            enqueue(q, value++);
        }

        /* Dequeue half of the batch (grow-then-shrink pattern) */
        int to_dequeue = batch_size / 2;
        for (int i = 0; i < to_dequeue; i++) {
            dequeue(q);
        }

        /* Periodic reporting */
        if (c % 10 == 0) {
            print_stats("progress");
        }
    }

    /* Clean up all remaining nodes */
    free_queue(q);

    /* Final summary */
    print_stats("final");

    return 0;
}
