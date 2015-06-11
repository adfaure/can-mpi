#include <stdio.h>
#include "CUnit/Basic.h"

#include "../src/cartesian_space.h"

#define UNUSED(x) (void)(x)

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
  init_land(&l, 500, 500, 500, 500);
  CU_ASSERT(is_land_contains(&l, 598, 532));
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

void test_IS_NEIGHBOUR(void) {
  land l1;
  init_land(&l1, 500, 250, 250, 500); // ((500,  250),  (250,  500))

  neighbour n_top, n_bot1, n_bot2, n_ri1, n_ri2, n_le1, n_le2;
  init_neighbour(&n_top, 500, 250, 250, VOISIN_H, 0); // frontière haute
  init_neighbour(&n_bot1, 500, 750, 125, VOISIN_H, 0); // frontière en bas, partie gauche
  init_neighbour(&n_bot2, 625, 750, 125, VOISIN_H, 0); // frontière en bas, partie droite
  init_neighbour(&n_ri1, 750, 500, 125, VOISIN_V, 0); // frontière a droite, partie haute
  init_neighbour(&n_ri2, 750, 625, 125, VOISIN_V, 0); // frontière en droite, partie basse
  init_neighbour(&n_le1, 500, 500, 125, VOISIN_V, 0); // frontière a gauche, partie haute
  init_neighbour(&n_le2, 500, 625, 125, VOISIN_V, 0); // frontière en gauche, partie basse
  CU_ASSERT(is_neighbour(&l1, &n_top));
  CU_ASSERT(is_neighbour(&l1, &n_bot1));
  CU_ASSERT(is_neighbour(&l1, &n_bot2));
  CU_ASSERT(is_neighbour(&l1, &n_ri1));
  CU_ASSERT(is_neighbour(&l1, &n_ri2));
  CU_ASSERT(is_neighbour(&l1, &n_le1));
  CU_ASSERT(is_neighbour(&l1, &n_le2));
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

  init_land(&l, 500 , 250, 250, 250);
  init_neighbour(&n, 500, 750, 125, VOISIN_H, 0);
  CU_ASSERT(!is_neighbour_bot(&l, &n));

  init_neighbour(&n, 625, 750, 125 , VOISIN_H, 0);
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

  init_land(&l, 500, 250, 250, 250);
  init_neighbour(&n, 750, 500, 125, VOISIN_V, 0);
  CU_ASSERT(!is_neighbour_right(&l, &n));

  init_land(&l, 500, 500, 250, 250);
  init_neighbour(&n, 750, 250, 500, VOISIN_V, 0);
  CU_ASSERT(is_neighbour_right(&l, &n));

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
  CU_ASSERT(is_contains_neighbour(&n1, &n2));
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

void test_LAND_EXTRACT_NEIGHBOURG_AFTER_SPLIT(void) {
  land old;
  land new;
  neighbour n1;
  neighbour n2;

  init_land(&old, 0, 0, 500, 1000); // a vertical rectangle
  split_land(&new, &old); // new: ((0, 500), (500, 500))
  land_extract_neighbourg_after_split(&new, &old, &n1, &n2);
  CU_ASSERT(n1.orientation == VOISIN_H);
  CU_ASSERT(n2.orientation == VOISIN_H);
  CU_ASSERT(n1.x == 0);
  CU_ASSERT(n1.y == 500);
  CU_ASSERT(n2.x == 0);
  CU_ASSERT(n2.y == 500);

  init_land(&old, 0, 0, 1000, 500); // a vertical rectangle
  split_land(&new, &old); // new: ((0, 500), (500, 500))
  land_extract_neighbourg_after_split(&new, &old, &n1, &n2);
  CU_ASSERT(n1.orientation == VOISIN_V);
  CU_ASSERT(n2.orientation == VOISIN_V);
  CU_ASSERT(n1.x == 500);
  CU_ASSERT(n1.y == 0);
  CU_ASSERT(n2.x == 500);
  CU_ASSERT(n2.y == 0);
}

void test_ADJUST_NEIGHBOUR(void) {
  land old;
  land new;
  land new2;
  neighbour n1;
  neighbour n2;

  init_land(&old, 0, 0, 500, 1000); // a vertical rectangle
  split_land(&new, &old); // new: ((0, 500), (500, 500))
  land_extract_neighbourg_after_split(&new, &old, &n1, &n2);
  // n1 et n1 contiennent la frontière entre new et old
  split_land(&new2, &new); // new2: ((250,  250),  (500,  500))
  //print_land(&new2);
  //print_neighbour(&n1);
  CU_ASSERT(n1.x == 0);
  CU_ASSERT(n1.y == 500);
  CU_ASSERT(n2.x == 0);
  CU_ASSERT(n2.y == 500);

  adjust_neighbour(&new2, &n1);
  adjust_neighbour(&new2, &n2);
  CU_ASSERT(n1.x == 250);
  CU_ASSERT(n1.y == 500);
  CU_ASSERT(n2.x == 250);
  CU_ASSERT(n2.y == 500);

  land_extract_neighbourg_after_split(&new, &old, &n1, &n2); // TODO works but avoid to have so much side effects
  adjust_neighbour(&new, &n1);
  adjust_neighbour(&new, &n2);
  CU_ASSERT(n1.x == 0);
  CU_ASSERT(n1.y == 500);
  CU_ASSERT(n2.x == 0);
  CU_ASSERT(n2.y == 500);
}

void test_UPDATE_BORDER(void) {
  neighbour n1, n2;
  init_neighbour(&n1, 0, 0, 500,VOISIN_H  ,0);
  init_neighbour(&n2, 0, 0, 500,VOISIN_V  ,0);
  CU_ASSERT(!update_border(&n1, &n2));

  init_neighbour(&n2, 250, 0, 500,VOISIN_H  ,0);
  CU_ASSERT(update_border(&n1, &n2));
  CU_ASSERT(n1.size == 250);
  CU_ASSERT(n1.x    == 0);
  CU_ASSERT(n1.y    == 0);

  init_neighbour(&n1, 250, 500, 500,VOISIN_H  ,0);
  init_neighbour(&n2, 0  , 500, 500,VOISIN_H  ,0);
  CU_ASSERT(update_border(&n1, &n2));
  CU_ASSERT(n1.size == 250);
  CU_ASSERT(n1.x == n2.x + n2.size);

  init_neighbour(&n1, 0, 500, 500,VOISIN_H  ,0);
  init_neighbour(&n2, 0, 500, 500,VOISIN_H  ,0);
  CU_ASSERT(update_border(&n1, &n2));
  CU_ASSERT(n1.size == 0);

  init_neighbour(&n1, 0, 500, 500,VOISIN_V  ,0);
  init_neighbour(&n2, 0, 500, 500,VOISIN_V  ,0);
  CU_ASSERT(update_border(&n1, &n2));
  CU_ASSERT(n1.size == 0);

  init_neighbour(&n1, 250  , 0  , 500 ,VOISIN_V  ,0);
  init_neighbour(&n2, 250  , 250, 500 ,VOISIN_V  ,0);
  CU_ASSERT(update_border(&n1, &n2));
  CU_ASSERT(n1.size == 250);

  init_neighbour(&n1, 250  , 0  , 500  ,VOISIN_V  ,0);
  init_neighbour(&n2, 250  , 250, 500,VOISIN_V  ,0);
  CU_ASSERT(update_border(&n1, &n2));
  CU_ASSERT(n1.size == 250);
  CU_ASSERT(n1.y    ==  0);
  init_neighbour(&n1, 500, 0  , 1000  , VOISIN_V  ,0);
  init_neighbour(&n2, 500, 500 ,  500 , VOISIN_V  ,0);
  CU_ASSERT(update_border(&n1, &n2));
  CU_ASSERT(n1.size == 500);
  CU_ASSERT(n1.y    ==  0);
}

void print_one_neighbour(void * nghbr) {
  print_neighbour((neighbour *) nghbr);
}

void do_nothing (void * n) {UNUSED(n);}

void test_SPLIT_LAND_UPDATE_NEIGHBOUR(void) {
  land l1, l_out;
  init_land(&l1, 500, 250, 250, 500); // ((500,  250),  (250,  500))

  list nghbrs1, nghbrs_out;
  init_list(&nghbrs1, sizeof(neighbour));
  init_list(&nghbrs_out, sizeof(neighbour));

  neighbour n_top, n_bot1, n_bot2, n_ri1, n_ri2, n_le1, n_le2;
  init_neighbour(&n_top, 500, 250, 250, VOISIN_H, 666); // frontière haute
  list_add_front(&nghbrs1, &n_top);

  split_land_update_neighbour(&l_out, &l1, &nghbrs_out, &nghbrs1, 42, 43);

  CU_ASSERT(nghbrs1.nb_elem == 2);
  CU_ASSERT(nghbrs_out.nb_elem == 1);


  list_clear(&nghbrs_out,do_nothing);
  list_clear(&nghbrs1 ,do_nothing);

  init_land(&l1, 500, 250, 250, 500); // ((500,  250),  (250,  500))
  init_neighbour(&n_top, 500, 250, 250,  VOISIN_H, 666); // frontière haute
  init_neighbour(&n_bot1, 500, 750, 125, VOISIN_H, 58);  // frontière en bas, partie gauche
  init_neighbour(&n_bot2, 625, 750, 125, VOISIN_H, 15);  // frontière en bas, partie droite
  list_add_front(&nghbrs1, &n_top );
  list_add_front(&nghbrs1, &n_bot1);
  list_add_front(&nghbrs1, &n_bot2);

  split_land_update_neighbour(&l_out, &l1, &nghbrs_out, &nghbrs1, 42, 43);

  CU_ASSERT(nghbrs1.nb_elem == 2);
  CU_ASSERT(nghbrs_out.nb_elem == 3);


  list_clear(&nghbrs_out,do_nothing);
  list_clear(&nghbrs1 ,do_nothing);

  init_land(&l1, 500, 250, 250, 500); // ((500,  250),  (250,  500))
  init_neighbour(&n_top, 500, 250, 250,  VOISIN_H, 666); // frontière haute
  init_neighbour(&n_bot1, 500, 750, 125, VOISIN_H, 58);  // frontière en bas, partie gauche
  init_neighbour(&n_bot2, 625, 750, 125, VOISIN_H, 15);  // frontière en bas, partie droite


  list_add_front(&nghbrs1, &n_top );
  list_add_front(&nghbrs1, &n_bot1);
  list_add_front(&nghbrs1, &n_bot2);

  split_land_update_neighbour(&l_out, &l1, &nghbrs_out, &nghbrs1, 42, 43);

  CU_ASSERT(nghbrs1.nb_elem == 2);
  CU_ASSERT(nghbrs_out.nb_elem == 3);

  list_clear(&nghbrs_out,do_nothing);
  list_clear(&nghbrs1 ,do_nothing);
  init_land(&l1, 500, 250, 250, 500); // ((500,  250),  (250,  500))

  init_neighbour(&n_top, 500, 250, 250,  VOISIN_H, 666); // frontière haute
  init_neighbour(&n_bot1, 500, 750, 125, VOISIN_H, 58);  // frontière en bas, partie gauche
  init_neighbour(&n_bot2, 625, 750, 125, VOISIN_H, 15);  // frontière en bas, partie droite
  init_neighbour(&n_ri1, 750, 500, 125, VOISIN_V, 28);   // frontière a droite, partie haute
  init_neighbour(&n_ri2, 750, 250, 500, VOISIN_V, 89);   // frontière en droite, partie basse

  list_add_front(&nghbrs1, &n_top );
  list_add_front(&nghbrs1, &n_bot1);
  list_add_front(&nghbrs1, &n_bot2);
  list_add_front(&nghbrs1, &n_ri1);
  list_add_front(&nghbrs1, &n_ri2);

  split_land_update_neighbour(&l_out, &l1, &nghbrs_out, &nghbrs1, 42, 43);

  CU_ASSERT(nghbrs1.nb_elem == 3);
  CU_ASSERT(nghbrs_out.nb_elem == 5);


  list_clear(&nghbrs_out,do_nothing);
  list_clear(&nghbrs1 ,do_nothing);
  init_land(&l1, 500, 250, 250, 500); // ((500,  250),  (250,  500))

  init_neighbour(&n_top, 500, 250, 250,  VOISIN_H, 666); // frontière haute
  init_neighbour(&n_bot1, 500, 750, 125, VOISIN_H, 58);  // frontière en bas, partie gauche
  init_neighbour(&n_bot2, 625, 750, 125, VOISIN_H, 15);  // frontière en bas, partie droite
  init_neighbour(&n_ri1, 750, 500, 125, VOISIN_V, 28); // frontière a droite, partie haute
  init_neighbour(&n_ri2, 750, 625, 125, VOISIN_V, 69); // frontière en droite, partie basse
  init_neighbour(&n_le1, 500, 500, 125, VOISIN_V, 78); // frontière a gauche, partie haute
  init_neighbour(&n_le2, 500, 625, 125, VOISIN_V, 32); // frontière en gauche, partie basse

  list_add_front(&nghbrs1, &n_top );
  list_add_front(&nghbrs1, &n_bot1);
  list_add_front(&nghbrs1, &n_bot2);
  list_add_front(&nghbrs1, &n_ri1);
  list_add_front(&nghbrs1, &n_ri2);
  list_add_front(&nghbrs1, &n_le1);
  list_add_front(&nghbrs1, &n_le2);
  split_land_update_neighbour(&l_out, &l1, &nghbrs_out, &nghbrs1, 42, 43);

  neighbour temp;
  for(int i = 0 ; i < nghbrs_out.nb_elem ; i++ ) {
	  list_get_index(&nghbrs_out, i ,&temp);
	  CU_ASSERT(is_neighbour(&l_out, &temp));
  }

  for(int i = 0 ; i < nghbrs1.nb_elem ; i++ ) {
	  list_get_index(&nghbrs1, i ,&temp);
	  CU_ASSERT(is_neighbour(&l1, &temp));
  }

  list_clear(&nghbrs_out,do_nothing);
  list_clear(&nghbrs1 ,do_nothing);
  init_land(&l1, 500, 0, 500, 1000); // ((500,  250),  (250,  500))

/*
  -> 1 ((0,  250),  (0,  500))

  -> 1 | [4] (250,  0),  (500)
  -> 1 -- [3] (0,  500),  (250)
  -> 2 ((500,  500),  (0,  500))
  -> 2 -- [5] (750,  500),  (250)
  -> 2 | [3] (500,  500),  (500)
  -> 2 -- [6] (500,  500),  (250)
  -> 2 | [4] (500,  0),  (500)
  -> 2 | [1] (500,  0),  (500)
  -> 3 ((0,  500),  (500,  500))
  -> 3 -- [4] (250,  500),  (250)
  -> 3 | [2] (500,  500),  (500)
  -> 3 -- [1] (0,  500),  (250)
  -> 4 ((250,  250),  (0,  500))
  -> 4 | [2] (500,  0),  (500)
  -> 4 -- [3] (250,  500),  (250)
  -> 4 | [1] (250,  0),  (500)
  -> 5 ((750,  250),  (500,  500))
  -> 5 -- [2] (750,  500),  (250)
  -> 5 | [6] (750,  500),  (500)
  -> 6 ((500,  250),  (500,  500))
  -> 6 | [5] (750,  500),  (500)
  -> 6 -- [2] (500,  500),  (250)
*/

  init_neighbour(&n_top, 500, 0, 500 , VOISIN_V, 1); // frontière haute
  init_neighbour(&n_le1, 500, 500, 500 , VOISIN_V, 3); // frontière haute

  list_add_front(&nghbrs1, &n_top );
  list_add_front(&nghbrs1, &n_le1 );
  split_land_update_neighbour(&l_out, &l1, &nghbrs_out, &nghbrs1, 5, 2);

  // relatif au problème PROBLEME_1
  land l_1, l_2;
  list ns_1, ns_2;
  neighbour n_1_2, n_2_1, n_2_4;

  init_land(&l_1, 0, 0, 500, 1000);
  init_land(&l_2, 500, 0, 250, 500);

  // neighbours of l_1:
  init_list(&ns_1, sizeof(neighbour));
  init_neighbour(&n_1_2, 500, 0, 500, VOISIN_V, 2);
  list_add_front(&ns_1, &n_1_2);

  // neighbours of l_2:
  init_list(&ns_2, sizeof(neighbour));
  init_neighbour(&n_2_1, 500, 0, 500, VOISIN_V, 1);
  init_neighbour(&n_2_4, 750, 0, 500, VOISIN_V, 4);
  list_add_front(&ns_2, &n_2_1);
  list_add_front(&ns_2, &n_2_4);

  // faire un neigbour avec
  // | [2] (750,  250),  (250)
  neighbour n_5_2;
  init_neighbour(&n_5_2, 750, 250, 250, VOISIN_V, 2);

  //printf("\n****Les frontières du land 2: \n");
  //list_apply(&ns_2, print_one_neighbour);
  //printf("\n");

  update_neighbours(&ns_2, &l_2, &n_5_2);

  //printf("\n****Les frontières du land 2 après le update_neighbours: \n");
  //list_apply(&ns_2, print_one_neighbour);
  //printf("\n");

  list_get_index(&ns_2, 2, &n_2_1);
  CU_ASSERT(n_2_1.size == 500);
}

void test_PROBLEME_2 (void) {
    // **************************************************************************
    // relatif au problème PROBLEME_2

    // situation OK:
    //  description textuelle (log): https://paste.kde.org/p4sib67pd
    //  SVG : http://i.imgur.com/EVOrvUZ.png

    // situation pas bonne (après split de 2 en (2,5)):
    //  description textuelle (log): https://paste.kde.org/pnzpiayoz
    //  SVG : http://i.imgur.com/CuXT0e7.png

    // le land 4 avait parmis ses voisins le land 3
    // et après le split de 2 en (2,5), le 5 à prévenu le 4
    // d'une mise à jour, 4 à bien mis à jour son coté droit,
    // mais a aussi mis a jour son coté gauche et a supprimé la bordure qu'il avait avec 3 !!!


    // on va avoir besoin du land 4, du land 5
    land l_4, l_5;
    // et de leurs listes de frontières
    list ns_4, ns_5;
    // et des frontières
    neighbour n_4_3, n_4_2, n_5_4;
    // n_5_4 est la nouvelle frontière.
    neighbour n_after_1, n_after_2;

    init_land(&l_4, 250, 500, 250, 500); // le land qui va faire nawak avec ses frontières
    init_land(&l_5, 500, 500, 500, 500); // le land issue du split

    init_list(&ns_4, sizeof(neighbour));
    init_list(&ns_5, sizeof(neighbour));

    init_neighbour(&n_4_3, 250, 500, 500, VOISIN_V, 3); // la frontière qui va etre supprimé ANORMALEMENT
    init_neighbour(&n_4_2, 500, 500, 500, VOISIN_V, 2); // la frontière qui va etre supprimé normalement
    init_neighbour(&n_5_4, 500, 500, 500, VOISIN_V, 4); // la nouvelle frontière

    list_add_front(&ns_4, &n_4_3);
    list_add_front(&ns_4, &n_4_2);

    list_add_front(&ns_5, &n_5_4);

    // printf("\n****Les frontières du land 2 (tout va bien): \n"); // je dis land 2 mais j'affiche land 5 mais c'est normal car je veux éviter de déclarer un land pour rien, mais c'est bon
    // list_apply(&ns_5, print_one_neighbour);
    // printf("\n");
    //
    // printf("\n****Les frontières du land 4 (tout va bien): \n");
    // list_apply(&ns_4, print_one_neighbour);
    // printf("\n");
    //
    update_neighbours(&ns_4, &l_4, &n_5_4);
    //
    // printf("\n****Les frontières du land 5 (apres update): \n");
    // list_apply(&ns_5, print_one_neighbour);
    // printf("\n");
    //
    // printf("\n****Les frontières du land 4 (apres update): \n");
    // list_apply(&ns_4, print_one_neighbour);
    // printf("\n");

    // le land 4 devrait avoir 2 frontières, la nouvelle et l'ancienne non touchée !!!
    CU_ASSERT(ns_4.nb_elem == 2);

    // l'ordre n'est pas important
    list_get_index(&ns_4, 0, &n_after_1);
    list_get_index(&ns_4, 1, &n_after_2);

    // mais l'une doit etre:
    //  | [x] (500,  500),  (500)
    // et l'autre doit etre:
    // | [x] (250,  500),  (500)
    CU_ASSERT((n_after_1.x == 250 && n_after_2.x == 500) || (n_after_1.x == 500 && n_after_2.x == 250));
}

void test_UPDATE_NEIGHBOURS(void) {
  land la;
  neighbour n, n2, n_out;
  list li;

  // ---------
  // |   |   |
  // | 1 |   |
  // |   |   |
  // ---------

  init_land(&la, 0, 0, 500, 1000); // rectangle vertical
  init_list(&li, sizeof(neighbour));
  init_neighbour(&n, 500, 0, 1000, VOISIN_V, 42); // | [42] (500,  0),  (1000)
  list_add_front(&li, &n);
  CU_ASSERT(li.nb_elem == 1);
  CU_ASSERT(is_neighbour(&la, &n));

  // use case
  // la partie droite se splite
  // n2 nouvelle bordure entre 1 et le "land" en bas à droite
  init_neighbour(&n2, 500, 500, 500, VOISIN_V, 43); // | [43] (500,  500),  (500)
  update_neighbours(&li, &la, &n2);
  CU_ASSERT(li.nb_elem == 2);
  CU_ASSERT(list_get_index(&li, 0, &n_out));
  CU_ASSERT(n_out.x == 500);
  CU_ASSERT(n_out.y == 500);
  CU_ASSERT(list_get_index(&li, 1, &n_out));
  CU_ASSERT(n_out.x == 500);
  CU_ASSERT(n_out.y == 0);
  list_clear(&li, do_nothing);

 /*
  * Problème lors de l'iunsertion de 4
  -> 1 ((0,  250),  (0,  500))
  -> 1 | [4] (250,  0),  (500)
  -> 1 -- [3] (0,  500),  (250)
  -> 2 ((500,  500),  (0,  1000))
  -> 2 | [3] (500,  500),  (500)
  -> 2 | [4] (500,  0),  (500)
  -> 2 | [1] (500,  0),  (500)
  -> 3 ((0,  500),  (500,  500))
  -> 3 -- [4] (250,  500),  (250)
  -> 3 | [2] (500,  500),  (500)
  -> 3 -- [1] (0,  500),  (250)
  -> 4 ((250,  250),  (0,  500))
  -> 4 | [2] (500,  0),  (500)
  -> 4 -- [3] (250,  500),  (250)
  -> 4 | [1] (250,  0),  (500)
*/

  init_land(&la, 500, 0, 500, 1000); // rectangle vertical
  init_list(&li, sizeof(neighbour));
  init_neighbour(&n, 500, 0, 500, VOISIN_V, 1); // | [42] (500,  0),  (1000)
  list_add_front(&li, &n);
  init_neighbour(&n, 500, 500, 500, VOISIN_V, 3); // | [42] (500,  0),  (1000)
  list_add_front(&li, &n);

  init_neighbour(&n, 500, 0, 500 ,VOISIN_V ,4);
  update_neighbours(&li, &la, &n);

  CU_ASSERT(li.nb_elem == 2);
/*
   printf("\n");
  list_apply(&li, print_one_neighbour);
  printf("\n");

*/
}

void test_PROBLEME_1 (void) {
    // test case for PROBLEME_1 http://i.imgur.com/hyjGK3C.png https://paste.kde.org/pm8trlzly
    // to http://i.imgur.com/8qXTTlj.png https://paste.kde.org/pitfjpgq3
    // the problem occurs when 4 splits, he contacts 2, and 2 must not change his borbers with 1!

    land l_1, l_2, l_4;
    list ns_1, ns_2, ns_4;
    neighbour n_1_2, n_2_1, n_2_3, n_2_4, n_4_3, n_4_2; // n_2_1 means a neighbour in land 2 that is frontier with land 1

    init_land(&l_1, 0, 0, 500, 1000);
    init_land(&l_2, 500, 0, 250, 500);
    init_land(&l_4, 750, 0, 250, 500);

    // neighbours of l_1:
    init_list(&ns_1, sizeof(neighbour));
    init_neighbour(&n_1_2, 500, 0, 500, VOISIN_V, 2);
    list_add_front(&ns_1, &n_1_2);

    // neighbours of l_2:
    init_list(&ns_2, sizeof(neighbour));
    init_neighbour(&n_2_1, 500, 0, 500, VOISIN_V, 1);
    init_neighbour(&n_2_3, 500, 500, 250, VOISIN_H, 3);
    init_neighbour(&n_2_4, 750, 0, 500, VOISIN_V, 4);
    list_add_front(&ns_2, &n_2_1);
    list_add_front(&ns_2, &n_2_3);
    list_add_front(&ns_2, &n_2_4);

    // neighbours of l_4:
    init_list(&ns_4, sizeof(neighbour));
    init_neighbour(&n_4_2, 750, 0, 500, VOISIN_V, 2);
    init_neighbour(&n_4_3, 750, 500, 250, VOISIN_H, 3);
    list_add_front(&ns_4, &n_4_2);
    list_add_front(&ns_4, &n_4_3);

    // end of configuration http://i.imgur.com/hyjGK3C.png

    // spliting 4 to (4, 5)
    land l_5;
    list ns_5;
    init_list(&ns_5, sizeof(neighbour));
    split_land_update_neighbour(&l_5, &l_4, &ns_5, &ns_4, 5, 4);

    // on va voir ce qu'il y a comme frontières dans le land_5:
    //printf("\nLes frontières du land 5: \n");
    //list_apply(&ns_5, print_one_neighbour);
    //printf("\n");
    // les frontières du land_5 sont bonnes

    // on va voir ce qu'il y a comme frontières dans le land_2:
    //printf("\nLes frontières du land 2: \n");
    //list_apply(&ns_2, print_one_neighbour);
    //printf("\n");
    // Les frontières du land 2:
    // | [4] (750,  0),  (500)
    // -- [3] (500,  500),  (250)
    // | [1] (500,  0),  (500)
    // le land 2 n'a pas encore redimensionné sa frontière avec 4 (c'est normal on lui a pas demandé)

    // on demande au land 2 de mettre a jour ses frontières
    // pour cela on a besoin de choper le nouveau neighbour (présent dans le land_5)
    // on triche un peu car on sait que c'est le 3 élément, donc indice 2
    neighbour n_5_2;
    list_get_index(&ns_5, 2, &n_5_2);
    update_neighbours(&ns_2, &l_2, &n_5_2);

    // on va voir a nouveau ce qu'il y a comme frontières dans le land_2:
    //printf("\nLes frontières du land 2 après le update_neighbours: \n");
    //list_apply(&ns_2, print_one_neighbour);
    //printf("\n");
    // | [2] (750,  250),  (250)
    // | [4] (750,  0),  (250)
    // -- [3] (500,  500),  (250)
    // | [1] (500,  0),  (250)


    // il faut simuler l'envoit des nouveaux neighbours de 5 à 2
    // comme ce que fait void CAN_REQ_Rec_Neighbours(MPI_Status *req_status, MPI_Comm comm, int com_rank, land *land_id, list *voisins, int *wait_for) {
    // mais sans MPI_SEND :)

    // neighbour_to_buffer(l,  buffer);


    // land 2 must have 4 frontiers now
    // checks frontiers of land 2 to be sure that the frontier with land 1 NOT CHANGED
    // TODO IMPLEMENT ME !!!! :)
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
     (NULL == CU_add_test(pSuite, "test of test_is_neighbour()", test_IS_NEIGHBOUR)) ||
     (NULL == CU_add_test(pSuite, "test of is_contains_neighbour()", test_IS_CONTAINS_NEIGHBOUR)) ||
     (NULL == CU_add_test(pSuite, "test of is_contains_neighbour_end()", test_IS_OVER_NEIGHBOUR_END)) ||
     (NULL == CU_add_test(pSuite, "test of land_extract_neighbourg_after_split()", test_LAND_EXTRACT_NEIGHBOURG_AFTER_SPLIT)) ||
     (NULL == CU_add_test(pSuite, "test of update_border()", test_UPDATE_BORDER)) ||
     (NULL == CU_add_test(pSuite, "test of split_land_update_neighbour()", test_SPLIT_LAND_UPDATE_NEIGHBOUR)) ||
     (NULL == CU_add_test(pSuite, "test of test_UPDATE_NEIGHBOURS()", test_UPDATE_NEIGHBOURS)) ||
     (NULL == CU_add_test(pSuite, "test case for PROBLEME_1 http://i.imgur.com/hyjGK3C.png to http://i.imgur.com/8qXTTlj.png", test_PROBLEME_1)) ||
     (NULL == CU_add_test(pSuite, "test case for PROBLEME_2 http://i.imgur.com/EVOrvUZ.png --> http://i.imgur.com/CuXT0e7.png", test_PROBLEME_2))

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
