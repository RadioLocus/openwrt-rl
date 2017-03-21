#include "queue.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct Queue* createQueue(int queueSize)
{
    struct Queue* queue1 = (struct Queue*) malloc(sizeof( struct Queue));
    queue1->capacity = queueSize;
    queue1->front = queue1->size = 0; 
    queue1->rear = queueSize - 1;  // This is important, see the enqueue
    queue1->array =  malloc(sizeof(char *) * queueSize); 
    return queue1;
}

void destroyQueue(struct Queue* queue1)
{
    int i = 0;
    if (queue1->size > 0){
        for (i = queue1->size - 1; i >= 0 ; i --){
            printf("deleting %d %s\n", i, queue1->array[i]);
            free(queue1->array[i]);
        } 

    }
    free(queue1->array);
    pthread_mutex_destroy(&mutex);

}


// Function to get front of queue
char* front(struct Queue* queue1)
{
    if (isEmpty(queue1))
        return NULL;
    return queue1->array[queue1->front];
}

// Function to get rear of queue
char* rear(struct Queue* queue1)
{
    if (isEmpty(queue1))
        return NULL;
    return queue1->array[queue1->rear];
}



// Queue is full when size becomes equal to the capacity 
int isFull(struct Queue* queue1)
{  return (queue1->size == queue1->capacity);  }

// Queue is empty when size is 0
int isEmpty(struct Queue* queue1)
{  return (queue1->size == 0); }

/***
queue - pointer to the queue
**/
void dequeue(struct Queue* queue1)
{
    if (isEmpty(queue1))
        return ;
    char* item = queue1->array[queue1->front];
    queue1->front = (queue1->front + 1)%queue1->capacity;
    queue1->size = queue1->size - 1;
    free(item);
    return;
}
//void check_consumer_dequeue(struct Queue* queue1,char* front_item, pthread_mutex_t *mutex)
void check_consumer_dequeue(struct Queue* queue1,char* front_item)
{
    pthread_mutex_lock(&mutex);         // lock access to the queue
    if (isEmpty(queue1)){
        pthread_mutex_unlock(&mutex);       // unlock queue
        return;
    }
    if (front_item!=front(queue1)){
        pthread_mutex_unlock(&mutex);       // unlock queue
        return;
    }
    char* item = queue1->array[queue1->front];
    queue1->front = (queue1->front + 1)%queue1->capacity;
    queue1->size = queue1->size - 1;
    pthread_mutex_unlock(&mutex);       // unlock queue
    free(item);
    return;
}

//char* consumer_dequeue(struct Queue* queue1, pthread_mutex_t *mutex)
char* consumer_dequeue(struct Queue* queue1)
{
    pthread_mutex_lock(&mutex);         // lock access to the queue
    if (isEmpty(queue1)){
        pthread_mutex_unlock(&mutex);       // unlock queue
        return NULL;
    }
    char* item = queue1->array[queue1->front];
    queue1->front = (queue1->front + 1)%queue1->capacity;
    queue1->size = queue1->size - 1;
    pthread_mutex_unlock(&mutex);       // unlock queue
    //free(item);
    return item;
}

//char* consumer_dequeue_all(struct Queue* queue1, int max_tuples_in_batch, pthread_mutex_t *mutex)
char* consumer_dequeue_all(struct Queue* queue1, int max_tuples_in_batch)
{
    pthread_mutex_lock(&mutex);         // lock access to the queue

    if (isEmpty(queue1)){
        pthread_mutex_unlock(&mutex);       // unlock queue
        return;
    }
    char* output=NULL;
    int batchcounter=0;
    while(queue1->size!=0 && batchcounter!=max_tuples_in_batch){
        char* item = queue1->array[queue1->front];
        queue1->front = (queue1->front + 1)%queue1->capacity;
        queue1->size = queue1->size - 1;
        strautocat(&output,item);
        free(item);
        batchcounter++;
    }
    pthread_mutex_unlock(&mutex);       // unlock queue
    //free(item);
    return output;
}


// Function to add an item to the queue.  It changes rear and size
//void enqueue(struct Queue* queue1, char* item, pthread_mutex_t *mutex, int *spillcounter)
void enqueue(struct Queue* queue1, char* item, int throwable)
{
    pthread_mutex_lock(&mutex);         // lock access to the queue
    if (isFull(queue1)){
        if(throwable==1){
            pthread_mutex_unlock(&mutex);
            return;
        }else{
        //spllied then retVal is 1
        queue1->spillcounter = queue1->spillcounter + 1;
        dequeue(queue1);
        }
    }
    queue1->rear = (queue1->rear + 1)%queue1->capacity;
    queue1->array[queue1->rear] = allocateString(item);
    queue1->size = queue1->size + 1;
    pthread_mutex_unlock(&mutex);       // unlock queue
    //printf("@@%d %s enqueued to queue\n",queue1->size, item);
}


