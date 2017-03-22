#include <stdio.h>
#include <string.h>
#include <time.h>
#include "CUnit/Basic.h"

#include "../../../lib/commonFunctions.h"
#include "../../../lib/metrics.h"

/* Pointer to the file used by the tests. */
static FILE* temp_file = NULL;

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

void testAdd(void){
   if (NULL != temp_file) {
      rewind(temp_file);
      CU_ASSERT(9 == add(4,5));
      CU_ASSERT(10 == add(5,5));
   }
}

void testCreateMetric(void) {
    char timestamp[20];
    sprintf(timestamp, "%u\n", (unsigned)time(NULL));
    struct Metric* metric = createMetric("testMetric", 32, atof(timestamp));
    CU_ASSERT(0 == strcmp("testMetric", metric->metricLabel));
    CU_ASSERT(32 == metric->value);
    CU_ASSERT(atof(timestamp) == metric->timestamp);
    destroyMetric(metric);

}

void testCreateJson(void) {
    struct Metric* metrics[3];
    char timestamp[20];
    sprintf(timestamp, "%u\n", (unsigned)time(NULL));
    metrics[0] = createMetric("UPTIME", 32, atof("1463475745"));
    metrics[1] = createMetric("CPU", 3, atof("1463475745"));
    metrics[2] = createMetric("CPU1", 4, atof("1463475745"));
    CU_ASSERT(0 == strcmp("UPTIME", metrics[0]->metricLabel));
    CU_ASSERT(3 == metrics[1]->value);
    json_object* metricJson = createMetricJson(metrics,3);
    CU_ASSERT(0 == strcmp("{ \"monitoringMetricList\": [ { \"key\": \"UPTIME\", \"value\": 32.000000, \"sensorTs\": { \"millis\": 1463475745.000000 } }, { \"key\": \"CPU\", \"value\": 3.000000, \"sensorTs\": { \"millis\": 1463475745.000000 } }, { \"key\": \"CPU1\", \"value\": 4.000000, \"sensorTs\": { \"millis\": 1463475745.000000 } } ] }", json_object_to_json_string(metricJson)));
    json_object* metricJson1 = createMetricJson(NULL,0);
    CU_ASSERT(0 == strcmp("{ \"monitoringMetricList\": [ ] }", json_object_to_json_string(metricJson1)));
    free(metrics[0]);
    free(metrics[1]);
    free(metrics[2]);
    json_object_put(metricJson);
    json_object_put(metricJson1);
    
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
       (NULL == CU_add_test(pSuite, "test of createMetric()", testCreateMetric)) ||
       (NULL == CU_add_test(pSuite, "test of createJSON()", testCreateJson))
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

