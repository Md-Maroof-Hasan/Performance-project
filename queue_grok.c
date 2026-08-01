#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Node structure for the linked list
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Queue structure
typedef struct Queue {
    Node* front;
    Node* rear;
    int size;
} Queue;

// Initialize the queue
Queue* initQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (!queue) {
        perror("Failed to allocate queue");
        exit(1);
    }
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    return queue;
}

// Enqueue an element (with timing)
void enqueue(Queue* queue, int data, long long* totalLatency, int* allocCount) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Failed to allocate node");
        exit(1);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    long long latency = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    *totalLatency += latency;
    (*allocCount)++;
    
    newNode->data = data;
    newNode->next = NULL;
    
    if (queue->rear == NULL) {
        queue->front = queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

// Dequeue an element
int dequeue(Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\n");
        return -1; // Error value
    }
    
    Node* temp = queue->front;
    int data = temp->data;
    queue->front = queue->front->next;
    
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    
    free(temp);
    queue->size--;
    return data;
}

// Free all memory in the queue
void freeQueue(Queue* queue) {
    Node* current = queue->front;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    free(queue);
}

int main() {
    // Initialize queue
    Queue* queue = initQueue();
    
    // Timing variables
    long long totalLatency = 0;
    int allocCount = 0;
    
    // Constants for workload
    const int CYCLES = 10;
    const int ENQUEUE_PER_CYCLE = 10000;
    const int DEQUEUE_PER_CYCLE = ENQUEUE_PER_CYCLE / 2; // Half
    
    printf("Starting dynamic queue workload simulation...\n");
    
    for (int cycle = 1; cycle <= CYCLES; cycle++) {
        // Enqueue batch
        for (int i = 0; i < ENQUEUE_PER_CYCLE; i++) {
            enqueue(queue, i + (cycle - 1) * ENQUEUE_PER_CYCLE, &totalLatency, &allocCount);
        }
        
        // Dequeue half
        for (int i = 0; i < DEQUEUE_PER_CYCLE; i++) {
            dequeue(queue);
        }
        
        // Periodic output
        printf("Cycle %d: Allocations so far: %d, Total latency: %lld ns\n", cycle, allocCount, totalLatency);
    }
    
    // Final summary
    long long avgLatency = (allocCount > 0) ? totalLatency / allocCount : 0;
    printf("\nFinal Summary:\n");
    printf("Total allocations: %d\n", allocCount);
    printf("Total latency: %lld ns\n", totalLatency);
    printf("Average latency per allocation: %lld ns\n", avgLatency);
    
    // Free all memory
    freeQueue(queue);
    
    return 0;
}
