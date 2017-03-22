#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "CUnit/Basic.h"

#include "../../../lib/commonFunctions.h"
#include "../../../lib/queue.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;
char *g_str_test_thread_return = NULL;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */

/* thread functions */
void *withdraw_2(void *arg)
{
    int i=5;
    int s;
    char deque_str[200], *deq_elem;
    struct Queue* queue = arg;
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        printf("pthread_setcancelstate %d", s);
    sleep(10);
    while (i!= 0){
    sleep(2);
    deq_elem = consumer_dequeue(queue);
    if (deq_elem == NULL)
    sprintf(deque_str, "deq2 none\n");
    else
    sprintf(deque_str, "deq2 %s", deq_elem);
    g_str_test_thread_return = strcat(g_str_test_thread_return, deque_str );
    i--;
    free(deq_elem);
    }
    return NULL;
}


void *withdraw(void *arg)
{
    int i=5;
    int s;
    char deque_str[200], *deq_elem;
    struct Queue* queue = arg;
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        printf("pthread_setcancelstate %d", s);
    sleep(6);
    while (i!= 0){
    sleep(2);
    deq_elem = consumer_dequeue(queue);
    if (deq_elem == NULL)
    sprintf(deque_str, "deq none\n");
    else
    sprintf(deque_str, "deq %s", deq_elem);
    g_str_test_thread_return = strcat(g_str_test_thread_return, deque_str );
    i--;
    free(deq_elem);
    }
    free(deq_elem);
    return NULL;
}

void *deposit(void *arg)
{
    int i=5;
    char enqueue_str[200];
    struct Queue* queue = arg;
    g_str_test_thread_return = malloc(1000);
    while (i!= 0){
    sleep(1);
    sprintf(enqueue_str, "enq %d\n", i);
    enqueue(queue, enqueue_str);
    g_str_test_thread_return = strcat(g_str_test_thread_return, enqueue_str);
    i--;
    }
    return NULL;
}


int init_suite1(void)
{
   if (NULL == (temp_file = fopen("temp.txt", "w+"))) {
      return -1;
   }
   else {
      return 0;
   }
}

int clean_suite1(void)
{
   if (0 != fclose(temp_file)) {
      return -1;
   }
   else {
      temp_file = NULL;
      return 0;
   }
}


/*
** Basic checking if the circular queue actually works
** for a queue of capacity 3 adding 5 elems, and then
** dequing them 
*/
void testCreateQueue(void)
{

   struct Queue* queue;
   pthread_t tid0;
   pthread_t tid1;
   char sampleOp[200]= "enq 5\nenq 4\nenq 3\nenq 2\nenq 1\ndeq enq 3\ndeq enq 2\ndeq enq 1\ndeq none\ndeq none\n";
   //char sampleOp[200];
   queue = createQueue(3);
   pthread_create(&tid0, NULL, deposit, queue);
   pthread_create(&tid1, NULL, withdraw, queue);
   //CU_ASSERT(9 == add(4,5));
   pthread_join(tid0, NULL);
   pthread_join(tid1, NULL);
   printf("returns %s\n", g_str_test_thread_return);
   printf("returns %s\n", sampleOp);
   CU_ASSERT(0 == strcmp(sampleOp,g_str_test_thread_return));
   //pthread_mutex_destroy(&mutex);
   free(g_str_test_thread_return);
   destroyQueue(queue);
   free(queue);
}

/*
** Kill the consumer and see if other consumer takes it up
*/
void testKillThread(void)
{
   struct Queue* queue;
   int s;
   pthread_t tid0;
   pthread_t tid1;
   pthread_t tid2;
   char sampleOp[200]= "enq 5\nenq 4\nenq 3\nenq 2\nenq 1\ndeq enq 3\ndeq2 enq 2\ndeq2 enq 1\ndeq2 none\ndeq2 none\n";
   //char sampleOp[200];
   queue = createQueue(3);
   pthread_create(&tid0, NULL, deposit, queue);
   pthread_create(&tid1, NULL, withdraw, queue);
   pthread_create(&tid2, NULL, withdraw_2, queue);
   sleep(9);
   s = pthread_cancel(tid1);
   if (s != 0)
        printf("pthread_setcancelstate %d", s);
   //CU_ASSERT(9 == add(4,5));
   pthread_join(tid0, NULL);
   pthread_join(tid1, NULL);
   pthread_join(tid2, NULL);
   printf("returns %s\n", g_str_test_thread_return);
   printf("returns %s\n", sampleOp);
   CU_ASSERT(0 != strcmp(sampleOp,g_str_test_thread_return));
   //pthread_mutex_destroy(&mutex);
   free(g_str_test_thread_return);
   destroyQueue(queue);
   free(queue);
  
}

/*
** Kill the consumer when the mutex has been 
*/


void testAdd(void){
   if (NULL != temp_file) {
      rewind(temp_file);
      CU_ASSERT(9 == add(4,5));
      CU_ASSERT(10 == add(5,5));
   }
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if ((NULL == CU_add_test(pSuite, "test of add()", testAdd)) || 
       (NULL == CU_add_test(pSuite, "test of testCreateQueue()", testCreateQueue)) ||
       (NULL == CU_add_test(pSuite, "test of testKillThread()", testKillThread))
     )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}

