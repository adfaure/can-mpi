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
  CU_ASSERT(n.com_rank == comm_rank);
  CU_ASSERT(n.orientation == orientation);
}

void test_INIT_LAND(void) {
  land l;
  unsigned int x = 250;
  unsigned int y = 250;
  unsigned int s_x = 500;
  unsigned int s_y = 500;
  init_land(&l, x, y, s_x, s_y);

  CU_ASSERT(l.x == x);
  CU_ASSERT(l.y == y);
  CU_ASSERT(l.size_x == s_x);
  CU_ASSERT(l.size_y == s_y);
}

void test_IS_LAND_CONTAINS(void) {
  land l;
  unsigned int x = 0;
  unsigned int y = 0;
  unsigned int s_x = 500;
  unsigned int s_y = 500;
  init_land(&l, x, y, s_x, s_y);
  CU_ASSERT(is_land_contains(&l, 0, 0));
  CU_ASSERT(! is_land_contains(&l, -1, -1));
  CU_ASSERT(is_land_contains(&l, 250, 250));
  CU_ASSERT(is_land_contains(&l, 499, 499));
  CU_ASSERT(! is_land_contains(&l, 500, 500));
  CU_ASSERT(is_land_contains(&l, 0, 499));
  CU_ASSERT(! is_land_contains(&l, 0, 500));
  CU_ASSERT(is_land_contains(&l, 499, 0));
  CU_ASSERT(! is_land_contains(&l, 500, 0));
}

void test_IS_LAND_CONTAINS_PAIR(void) {
  land l;
  pair p;

  unsigned int x = 0;
  unsigned int y = 0;
  unsigned int s_x = 500;
  unsigned int s_y = 500;
  init_land(&l, x, y, s_x, s_y);

  p.x = -1; p.y = -1;
  CU_ASSERT(! is_land_contains_pair(&l, &p));
  p.x = 0; p.y = 0;
  CU_ASSERT(is_land_contains_pair(&l, &p));
  p.x = 500; p.y = 500;
  CU_ASSERT(! is_land_contains_pair(&l, &p));
  p.x = 250; p.y = 250;
  CU_ASSERT(is_land_contains_pair(&l, &p));
  p.x = 499; p.y = 499;
  CU_ASSERT(is_land_contains_pair(&l, &p));
  p.x = 500; p.y = 0;
  CU_ASSERT(!is_land_contains_pair(&l, &p));
  p.x = 0; p.y = 500;
  CU_ASSERT(!is_land_contains_pair(&l, &p));
}

void test_SPLIT_LAND(void) {
  land old;
  land new;

  init_land(&old, 0, 0, 500, 1000); // a vertical rectangle
  split_land(&new, &old);

  CU_ASSERT(is_land_contains(&old, 0, 0));
  CU_ASSERT(is_land_contains(&old, 0, 499));
  CU_ASSERT(is_land_contains(&old, 499, 0));
  CU_ASSERT(is_land_contains(&old, 499, 499));
  CU_ASSERT(! is_land_contains(&old, 500, 500));
  CU_ASSERT(! is_land_contains(&old, 0, 500));
  CU_ASSERT(! is_land_contains(&old, 500, 0));

  CU_ASSERT(! is_land_contains(&new, 0, 0));
  CU_ASSERT(! is_land_contains(&new, 0, 499));
  CU_ASSERT(! is_land_contains(&new, 499, 0));
  CU_ASSERT(! is_land_contains(&new, 499, 499));
  CU_ASSERT(is_land_contains(&new, 0, 500));
  CU_ASSERT(is_land_contains(&new, 499, 500));
  CU_ASSERT(is_land_contains(&new, 0, 999));
  CU_ASSERT(is_land_contains(&new, 499, 999));

  init_land(&old, 0, 0, 1000, 500); // a horizontal rectangle
  split_land(&new, &old);

  CU_ASSERT(is_land_contains(&old, 0, 0));
  CU_ASSERT(is_land_contains(&old, 499, 499));
  CU_ASSERT(!is_land_contains(&old, 500, 499));
  CU_ASSERT(!is_land_contains(&old, 499, 500));

  CU_ASSERT(is_land_contains(&new, 500, 0));
  CU_ASSERT(is_land_contains(&new, 500, 499));
  CU_ASSERT(is_land_contains(&new, 999, 0));
  CU_ASSERT(is_land_contains(&new, 999, 499));
  CU_ASSERT(!is_land_contains(&new, 999, 500));
}

void test_IS_NEIGHBOUR_BOT(void) {
  neighbour n;
  land l;
  init_neighbour(&n, 0, 500, 500, VOISIN_H, 0);

  init_land(&l, 0, 0, 500, 500);
  CU_ASSERT(is_neighbour_bot(&l, &n));

  init_land(&l, 0, 0, 600, 500);
  CU_ASSERT(is_neighbour_bot(&l, &n));

  init_neighbour(&n, 500, 500, 500, VOISIN_H, 0);
  init_land(&l, 499, 0, 501, 500);
  CU_ASSERT(is_neighbour_bot(&l, &n));

  init_land(&l, 0, 0, 490, 500);
  CU_ASSERT(!is_neighbour_bot(&l, &n));
}

void test_IS_NEIGHBOUR_TOP(void) {
  neighbour n;
  land l;
  init_neighbour(&n, 0, 0, 500, VOISIN_H, 0);

  init_land(&l, 0, 0, 500, 500);
  CU_ASSERT(is_neighbour_top(&l, &n));

  init_land(&l, 0, 0, 600, 500);
  CU_ASSERT(is_neighbour_top(&l, &n));

  init_neighbour(&n, 500, 0, 500, VOISIN_H, 0);
  init_land(&l, 499, 0, 501, 500);
  CU_ASSERT(is_neighbour_top(&l, &n));

  init_land(&l, 0, 0, 490, 500);
  CU_ASSERT(!is_neighbour_top(&l, &n));
}

void test_IS_NEIGHBOUR_LEFT(void) {
  neighbour n;
  land l;
  init_neighbour(&n, 0, 0, 500, VOISIN_V, 0);

  init_land(&l, 0, 0, 500, 500);
  CU_ASSERT(is_neighbour_left(&l, &n));

  init_land(&l, 0, 0, 600, 500);
  CU_ASSERT(is_neighbour_left(&l, &n));

  init_neighbour(&n, 500, 0, 500, VOISIN_V, 0);
  init_land(&l, 499, 0, 501, 500);
  CU_ASSERT(!is_neighbour_left(&l, &n));

  init_land(&l, 0, 0, 490, 500);
  CU_ASSERT(!is_neighbour_left(&l, &n));
}

void test_IS_NEIGHBOUR_RIGHT(void) {
  neighbour n;
  land l;
  init_neighbour(&n, 500, 0, 500, VOISIN_V, 0);

  init_land(&l, 0, 0, 500, 500);
  CU_ASSERT(is_neighbour_right(&l, &n));

  init_neighbour(&n, 600, 0, 500, VOISIN_V, 0);
  init_land(&l, 0, 0, 600, 500);
  CU_ASSERT(is_neighbour_right(&l, &n));

  init_neighbour(&n, 500, 0, 500, VOISIN_V, 0);
  init_land(&l, 499, 0, 501, 500);
  CU_ASSERT(!is_neighbour_right(&l, &n));

  init_land(&l, 0, 0, 490, 500);
  CU_ASSERT(!is_neighbour_right(&l, &n));
}

void test_IS_CONTAINS_NEIGHBOUR(void) {

  //   500            1000
  // n1 x--------------x
  // n2    x-------x
  neighbour n1;
  neighbour n2;
  init_neighbour(&n1, 500, 0, 500, VOISIN_H, 0);
  init_neighbour(&n2, 501, 0, 498, VOISIN_H, 0);
  CU_ASSERT(is_contains_neighbour(&n1, &n2));

  // idem mais vertical
  init_neighbour(&n1, 0, 500, 500, VOISIN_V, 0);
  init_neighbour(&n2, 0, 501, 498, VOISIN_V, 0);
  CU_ASSERT(is_contains_neighbour(&n1, &n2));

  init_neighbour(&n1, 0, 500, 500, VOISIN_V, 0);
  init_neighbour(&n2, 0, 500, 498, VOISIN_V, 0);
  CU_ASSERT(!is_contains_neighbour(&n1, &n2));
}

void test_IS_OVER_NEIGHBOUR_END(void) {

  neighbour n1;
  neighbour n2;
  init_neighbour(&n1, 250, 250, 500, VOISIN_H, 0);
  init_neighbour(&n2, 250, 250, 600, VOISIN_H, 0); // ça déborde à droite

  // celui çi devrait être vrai car doit retourner vrai si
  // n1 : x--------------x
  // n2 :         x-------------x
  CU_ASSERT(is_over_neighbour_end(&n1, &n2)); // failed

  // de toute façon l'un des trois devrait etre vrai :///
  CU_ASSERT(!is_contains_neighbour(&n1, &n2));
  CU_ASSERT(!is_over_neighbour_begin(&n1, &n2));
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

     (NULL == CU_add_test(pSuite, "test of init_neighbour()", test_INIT_NEIGHBOUR)) ||
     (NULL == CU_add_test(pSuite, "test of init_land()", test_INIT_LAND)) ||
     (NULL == CU_add_test(pSuite, "test of is_land_contains()", test_IS_LAND_CONTAINS)) ||
     (NULL == CU_add_test(pSuite, "test of is_land_contains_pair()", test_IS_LAND_CONTAINS_PAIR)) ||
     (NULL == CU_add_test(pSuite, "test of split_land()", test_SPLIT_LAND)) ||
     (NULL == CU_add_test(pSuite, "test of is_neighbour_bot()", test_IS_NEIGHBOUR_BOT)) ||
     (NULL == CU_add_test(pSuite, "test of is_neighbour_top()", test_IS_NEIGHBOUR_TOP)) ||
     (NULL == CU_add_test(pSuite, "test of is_neighbour_left()", test_IS_NEIGHBOUR_LEFT)) ||
     (NULL == CU_add_test(pSuite, "test of is_neighbour_right()", test_IS_NEIGHBOUR_RIGHT)) ||
     (NULL == CU_add_test(pSuite, "test of is_contains_neighbour()", test_IS_CONTAINS_NEIGHBOUR)) ||
     (NULL == CU_add_test(pSuite, "test of is_contains_neighbour_end()", test_IS_OVER_NEIGHBOUR_END))

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
