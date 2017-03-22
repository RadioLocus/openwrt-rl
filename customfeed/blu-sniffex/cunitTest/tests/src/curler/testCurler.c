#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmockery.h>
#include <curl/curl.h>

#include "CUnit/Basic.h"

#include "../../../lib/commonFunctions.h"
#include "../../../lib/curler.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;
//extern CURLcode curl_easy_perform(CURL *);

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


//CURLcode curl_easy_perform(CURL *ch)
//{
//return (CURLcode)mock();
//}

/*
** Basic checking if the circular queue actually works
** for a queue of capacity 3 adding 5 elems, and then
** dequing them 
*/

void testCurlPost(void)
{
   
    char * curlPostStr = "{ \"monitoringMetricList\": [ { \"key\": \"UPTIME\", \"value\": 32.000000, \"sensorTs\": { \"millis\": 1463475745.000000 } }, { \"key\": \"CPU\", \"value\": 3.000000, \"sensorTs\": { \"millis\": 1463475745.000000 } }, { \"key\": \"CPU1\", \"value\": 4.000000, \"sensorTs\": { \"millis\": 1463475745.000000 } } ] }";
    char * curlPostTuple = "3,124c1c76-412a-4a7e-96ff-9476130c2e0c,69905,1,1448186403,4c:bb:58:33:2c:d1,-86.0,Inorbit,1";
    //will_return( curl_easy_perform, CURLE_OK );
    printf("**** %s\n", curlPostStr);
    
    CU_ASSERT(CURLE_OK == curl_post_jsonstr("http://packet.radiolocus.com/monitoring-web/v1/g/m/u/test123" , curlPostStr));
    printf("****1\n");
    CU_ASSERT(CURLE_OK == curl_post_str("http://packet.radiolocus.com/v1/p/u/test123" , curlPostTuple));
    printf("****2\n");
    CU_ASSERT(CURLE_OK != curl_post_str("http://packaet.radiolocus.com/v1/p/u/test123" , curlPostTuple));
    printf("****3\n");
    //assert_int_equal(curl_post("http://packet1.radiolocus1.com/monitoring-web/v1/g/m/u/test123" , curlPostStr), CURLE_OK);
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

   //const UnitTest tests[] = {
   //     unit_test(testCurlPost)
   // };
   //run_tests(tests);
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
       (NULL == CU_add_test(pSuite, "test of testCurlPost()", testCurlPost)) 
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

