#include <stdio.h>
#include "CUnit/Basic.h"

#include "../src/utils.h"
#include "../src/cartesian_space.h"

int init_suite1 (void) {return 0;}

int clean_suite1(void) {return 0;}

void test_42(void) {
  unsigned int life = 42;
  CU_ASSERT(42 == life);
}

void test_INIT_LIST(void) {
  list nghbrs;
  init_list(&nghbrs, sizeof(neighbour));
  CU_ASSERT(nghbrs.nb_elem == 0);
  CU_ASSERT(nghbrs.first == NULL);
  CU_ASSERT(nghbrs.element_size == sizeof(neighbour));
}

void test_LIST_ADD_FRONT(void) {
  list nghbrs;
  init_list(&nghbrs, sizeof(neighbour));

  neighbour n;
  init_neighbour(&n, 42, 42, 42*42, VOISIN_H, 42);
  list_add_front(&nghbrs, &n);
  CU_ASSERT(nghbrs.nb_elem == 1);
  CU_ASSERT(nghbrs.element_size == sizeof(neighbour));

  neighbour n2;
  list_get_index(&nghbrs, 0, &n2);
  CU_ASSERT(n.x == n2.x);
  CU_ASSERT(n.y == n2.y);
  CU_ASSERT(n.size == n2.size);
  CU_ASSERT(n.com_rank == n2.com_rank);
  CU_ASSERT(n.orientation == n2.orientation);
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
   if (

     (NULL == CU_add_test(pSuite, "test of 42 !!", test_42)) ||
     (NULL == CU_add_test(pSuite, "test of init_list()", test_INIT_LIST)) ||
     (NULL == CU_add_test(pSuite, "test of list_add_front()", test_LIST_ADD_FRONT))

     ) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
