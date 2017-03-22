#include <stdlib.h>
#include <pthread.h>
#include "commonFunctions.h"

struct Queue{
    int front, rear, size, spillcounter;
    unsigned capacity;
    char** array;
};

struct Queue* createQueue(int);

char* front(struct Queue*);

char* rear(struct Queue*);

int isFull(struct Queue*);

int isEmpty(struct Queue*);

void dequeue(struct Queue*);

//void check_consumer_dequeue(struct Queue*, char*, pthread_mutex_t *);
void check_consumer_dequeue(struct Queue*, char*);

//char* consumer_dequeue(struct Queue*, pthread_mutex_t *);
char* consumer_dequeue(struct Queue*);

//char* consumer_dequeue_all(struct Queue*, int , pthread_mutex_t *);
char* consumer_dequeue_all(struct Queue*, int);

//void enqueue(struct Queue*, char*, pthread_mutex_t *, int *);
void enqueue(struct Queue*, char*, int);

