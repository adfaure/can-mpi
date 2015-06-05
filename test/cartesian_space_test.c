#include <stdio.h>
#include "CUnit/Basic.h"

#include "../src/cartesian_space.h"

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int init_suite1 (void) {return 0;}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite1(void) {return 0;}

void test_INIT_NEIGHBOUR(void) {
  neighbour n;
  unsigned int x = 0;
  unsigned int y = 0;
  unsigned int size = 1000;
  unsigned int orientation = 0;
  unsigned int comm_rank = 0;
  init_neighbour(&n, x, y, size, orientation, comm_rank);

  CU_ASSERT(n.x == x);
  CU_ASSERT(n.y == y);
  CU_ASSERT(n.size == size);
  CU_ASSERT(n.com_rank = comm_rank);
  CU_ASSERT(n.orientation == orientation);
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
   if ((NULL == CU_add_test(pSuite, "test of INIT_NEIGHBOUR()", test_INIT_NEIGHBOUR))) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
