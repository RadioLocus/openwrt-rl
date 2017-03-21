#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"

#include "../../../lib/commonFunctions.h"

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

void testCombineString(void) {
    char *stack_str1, *stack_str2, *heap_str1, *heap_str2, *tmpStr;
    stack_str1 = "stack";
    stack_str2 = "Stacked";
    heap_str1 = (char *) malloc(15);
    strcpy(heap_str1, "heap");
    heap_str2 = (char *) malloc(15);
    strcpy(heap_str2, "Heaped");
    if (NULL != temp_file){
	rewind(temp_file);
        tmpStr = combineString(stack_str1,stack_str2);
        CU_ASSERT(0 == strcmp(tmpStr,"stackStacked"));
        free(tmpStr);
        tmpStr = combineString(heap_str1,heap_str2);
        CU_ASSERT(0 == strcmp(tmpStr,"heapHeaped"));
        free(tmpStr);
        tmpStr = combineString(NULL,NULL);
        CU_ASSERT(NULL == tmpStr);
        free(tmpStr);
        tmpStr= combineString(NULL,heap_str2);
        CU_ASSERT(0 == strcmp(tmpStr,"Heaped"));
        free(tmpStr);
        tmpStr= combineString(heap_str1,NULL);
        CU_ASSERT(0 == strcmp(tmpStr,"heap"));
        free(tmpStr);
    }
    free(heap_str1);
    free(heap_str2);

}

void test_str_replace(void) {
    char *stack_str1, *stack_str2, *stack_str3, *heap_str1, *heap_str2, *heap_str3, *tmpStr;
    stack_str1 = "stack";
    stack_str2 = "Stacked";
    stack_str3 = "tac";
    heap_str1 = (char *) malloc(15);
    strcpy(heap_str1, "heap");
    heap_str2 = (char *) malloc(15);
    strcpy(heap_str2, "Heaped");
    heap_str3 = (char *) malloc(15);
    strcpy(heap_str3, "eap");
     if (NULL != temp_file){
	rewind(temp_file);
        tmpStr = str_replace(stack_str1,stack_str3,stack_str2);
        CU_ASSERT(0 == strcmp(tmpStr, "sStackedk"));
        free(tmpStr);
        tmpStr = str_replace(heap_str1,heap_str3,heap_str2);
        CU_ASSERT(0 == strcmp(tmpStr,"hHeaped"));
        free(tmpStr);
        tmpStr = str_replace(NULL,NULL,NULL);
        CU_ASSERT(NULL == tmpStr);
        free(tmpStr);
        tmpStr = str_replace(heap_str1,NULL,heap_str2);
        CU_ASSERT(0 == strcmp(tmpStr,"heap"));
        free(tmpStr);
        tmpStr = str_replace(stack_str1,NULL,stack_str2);
        CU_ASSERT(0 == strcmp(tmpStr,"stack"));
        free(tmpStr);
        tmpStr = str_replace(heap_str1,heap_str3,NULL);
        CU_ASSERT(0 == strcmp(tmpStr,"heap"));
        free(tmpStr);
        tmpStr = str_replace(stack_str1,stack_str3,NULL);
        CU_ASSERT(0 == strcmp(tmpStr,"stack"));
        free(tmpStr);
    }
    free(heap_str1);
    free(heap_str2);
    free(heap_str3);

}

void teststrautocat(void){
    char *stack_str1, *stack_str2, *heap_str1, *heap_str2, *tmpStr;
    stack_str1 = "stack";
    stack_str2 = "Stacked";
    heap_str1 = (char *) malloc(15);
    memset(heap_str1, '\0', 14);
    strcpy(heap_str1, "heap");
    heap_str2 = (char *) malloc(15);
    memset(heap_str2, '\0', 14);
    strcpy(heap_str2, "Heaped");
    if (NULL != temp_file){
        rewind(temp_file);
        //strautocat(&stack_str1,stack_str2);
        //CU_ASSERT(0 == strcmp(stack_str1,"stackStacked"));
        strautocat(&heap_str1,heap_str2);
        CU_ASSERT(0 == strcmp(heap_str1,"heapHeaped"));
        //strautocat(NULL,NULL);
        //CU_ASSERT(NULL == NULL);
        free(heap_str1);
        heap_str1 = NULL;
        strautocat(&heap_str1,heap_str2);
        CU_ASSERT(0 == strcmp(heap_str1,"Heaped"));
        strautocat(&heap_str1,NULL);
        CU_ASSERT(0 == strcmp(heap_str1,"Heaped"));
    }
    //printf("%s, %s\n", &heap_str1, &heap_str2);
    free(heap_str1);
    free(heap_str2);

}




//void testAppendString(void) {
//    char *stack_str1, *stack_str2, *heap_str1, *heap_str2, *tmpStr;
//    stack_str1 = "stack";
//    stack_str2 = "Stacked";
//    heap_str1 = (char *) malloc(15);
//    memset(heap_str1, '\0', 14);
//    strcpy(heap_str1, "heap");
//    heap_str2 = (char *) malloc(15);
//    memset(heap_str2, '\0', 14);
//    strcpy(heap_str2, "Heaped");
//    if (NULL != temp_file){
//        rewind(temp_file);
//        str_append(stack_str1,stack_str2);
//        CU_ASSERT(0 == strcmp(stack_str1,"stackStacked"));
//        str_append(heap_str1,heap_str2);
//        CU_ASSERT(0 == strcmp(heap_str1,"heapHeaped"));
//        str_append(NULL,NULL);
//        CU_ASSERT(NULL == NULL);
//        free(heap_str1);
//        heap_str1 = NULL;
//        str_append(heap_str1,heap_str2);
//        CU_ASSERT(0 == strcmp(heap_str1,"Heaped"));
//        str_append(heap_str1,NULL);
//        CU_ASSERT(0 == strcmp(heap_str1,"Heaped"));
//    }
//    //printf("%s, %s\n", &heap_str1, &heap_str2);
//    free(heap_str1);
//    free(heap_str2);
//
//}



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
       (NULL == CU_add_test(pSuite, "test of combineString()", testCombineString)) ||
       (NULL == CU_add_test(pSuite, "test of str_teststrautocat()", teststrautocat)) ||
       (NULL == CU_add_test(pSuite, "test of str_replace()", test_str_replace))
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

