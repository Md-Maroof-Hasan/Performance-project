
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ─── Node & Queue structures ───────────────────────────────────────────────

struct Node {
    int data;
    struct Node* next;
};

struct Queue {
    struct Node* front;
    struct Node* rear;
    int size;
};

// ─── Allocation latency logger ─────────────────────────────────────────────

static int alloc_count = 0;
static long total_latency = 0;

struct Node* createNode(int data) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));

    clock_gettime(CLOCK_MONOTONIC, &end);
    long latency_ns = (end.tv_sec - start.tv_sec) * 1000000000L
                    + (end.tv_nsec - start.tv_nsec);

    alloc_count++;
    total_latency += latency_ns;
    printf("Alloc #%d | Size: %zu bytes | Latency: %ld ns\n",
           alloc_count, sizeof(struct Node), latency_ns);

    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// ─── Queue operations ──────────────────────────────────────────────────────

struct Queue* createQueue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    q->size = 0;
    return q;
}

void enqueue(struct Queue* q, int data) {
    struct Node* newNode = createNode(data);
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

int dequeue(struct Queue* q) {
    if (q->front == NULL) return -1;
    struct Node* temp = q->front;
    int data = temp->data;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    q->size--;
    return data;
}

void freeQueue(struct Queue* q) {
    while (q->front != NULL) {
        struct Node* temp = q->front;
        q->front = q->front->next;
        free(temp);
    }
    free(q);
}

// ─── Main ──────────────────────────────────────────────────────────────────

int main() {
    int TOTAL_OPS = 100000;   // total enqueue operations
    int BATCH     = 1000;     // enqueue BATCH items, then dequeue BATCH items

    struct Queue* q = createQueue();

    // Repeated enqueue/dequeue cycles — unique lifecycle pattern
    for (int cycle = 0; cycle < TOTAL_OPS / BATCH; cycle++) {

        // Enqueue a batch
        for (int i = 0; i < BATCH; i++) {
            enqueue(q, cycle * BATCH + i);
        }

        // Dequeue half the batch — memory grows then partially shrinks
        for (int i = 0; i < BATCH / 2; i++) {
            dequeue(q);
        }
    }

    // Drain remaining
    while (q->front != NULL) {
        dequeue(q);
    }

    freeQueue(q);

    // ── Summary ──
    printf("\n========== Allocation Summary ==========\n");
    printf("Total allocations : %d\n", alloc_count);
    printf("Total latency     : %ld ns\n", total_latency);
    if (alloc_count > 0)
        printf("Average latency   : %ld ns per allocation\n",
               total_latency / alloc_count);

    return 0;
}
