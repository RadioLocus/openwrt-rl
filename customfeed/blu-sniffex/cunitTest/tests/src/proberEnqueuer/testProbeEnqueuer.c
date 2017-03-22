#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "CUnit/Basic.h"

#include "../../../lib/commonFunctions.h"
#include "../../../lib/queue.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;
int spillcounter = 0;
char *g_str_test_thread_return = NULL;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */

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
   char sampleOp[200]= "enq 5\nenq 4\nenq 3\nenq 2\nenq 1\ndeq enq 3\ndeq enq 2\ndeq enq 1\ndeq none\ndeq none\n";
   //char sampleOp[200];
   char* sensor_id = "test123";
   queue = createQueue(3);
   generate_tuple("wlan0-mon0", queue, 100, 3, 1, 0, sensor_id);
   //CU_ASSERT(0 == strcmp(sampleOp,g_str_test_thread_return));
   //pthread_mutex_destroy(&mutex);
   //free(g_str_test_thread_return);
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
       (NULL == CU_add_test(pSuite, "test of testCreateQueue()", testCreateQueue)) 
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

