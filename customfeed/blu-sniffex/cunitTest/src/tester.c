//#include "lib/commonFunctions.h"
#include "lib/queue.h"
int spillcounter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;      // mutual exclusion lock
pthread_t tPcapThread;
//pthread_t tTupleSender[g_curling_threads];
pthread_t *tTupleSender;
pthread_t tMain;


void *withdraw(void *arg)
{
    int i=5;
    struct Queue* queue = arg;
    while (i!= 0){
    sleep(3);
    printf("hi %s \n", consumer_dequeue(queue, &mutex));
    i--;
    }
    return NULL;
}
 
void *deposit(void *arg)
{
    int i=5;
    struct Queue* queue = arg;
    while (i!= 0){
    sleep(1);
    printf("hi enqueing \n");
     enqueue(queue, "helloworld", &mutex, &spillcounter);
    i--;
    }
    return NULL;
}

int main()
{
char *str1 = "stringsubstring";
char *str2 = "substring";
char *str3 = "sub";
struct Queue* queue;
pthread_t tid0;
pthread_t tid1;
 
tTupleSender = malloc(sizeof(pthread_t)*2);

queue = createQueue(4);
pthread_create(&tid0, NULL, deposit, queue);
pthread_create(&tid1, NULL, withdraw, queue);
puts("\nDeposit and withdraw ...\n");
 
    pthread_join(tid0, NULL);
    pthread_join(tid1, NULL);
 
    printf("balance =\n");
    pthread_mutex_destroy(&mutex);
//enqueue(queue, "helloworld", &mutex, &spillcounter);
//printf("hi %s \n", consumer_dequeue(queue, &mutex));
//printf("%s\n", str_append(NULL,str2));
}
