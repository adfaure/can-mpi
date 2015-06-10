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

void do_nothing (void * n) {}

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

  /*
  Problème après le split de 2

  -> 1 ((0,  500),  (0,  1000))
  -> 1 | [3] (500,  500),  (500)
  -> 1 | [2] (500,  0),  (500)
  -> 2 ((500,  250),  (0,  500))
  -> 2 | [5] (750,  250),  (250)
  -> 2 | [4] (750,  0),  (250)
  -> 2 -- [3] (500,  500),  (250)
  -> 2 | [1] (500,  0),  (250)
  -> 3 ((500,  500),  (500,  500))
  -> 3 -- [5] (750,  500),  (250)
  -> 3 | [1] (500,  500),  (500)
  -> 3 -- [2] (500,  500),  (250)
  -> 4 ((750,  250),  (0,  250))
  -> 4 -- [5] (750,  250),  (250)
  -> 4 | [2] (750,  0),  (250)
  -> 5 ((750,  250),  (250,  250))
  -> 5 | [2] (750,  250),  (250)
  -> 5 -- [3] (750,  500),  (250)
  -> 5 -- [4] (750,  250),  (250)
  -> 6 ((0,  3972709368),  (0,  32524))
  -> 7 ((0,  4266011640),  (0,  32531))
  -> 8 ((0,  763501560),  (0,  32655))
  -> 9 ((0,  2007423992),  (0,  32712))
  */

  // clear
  list_clear(&nghbrs_out,do_nothing);
  init_land(&l_out, 0, 0, 0, 0);

  land la;
  list li;
  neighbour n, n2, n3, n4;

  init_land(&la, 500, 0, 250, 500); // rectangle vertical
  init_list(&li, sizeof(neighbour));

  init_neighbour(&n, 750, 250, 250, VOISIN_V, 5); // -> 2 | [5] (750,  250),  (250)
  list_add_front(&li, &n);

  init_neighbour(&n2, 750, 0, 250, VOISIN_V, 4); // -> 2 | [4] (750,  0),  (250)
  list_add_front(&li, &n2);

  init_neighbour(&n3, 500, 500, 250, VOISIN_H, 3); // -> 2 -- [3] (500,  500),  (250)
  list_add_front(&li, &n3);

  init_neighbour(&n4, 500, 0, 250, VOISIN_V, 1); // -> 2 | [1] (500,  0),  (250)
  list_add_front(&li, &n4);

  split_land_update_neighbour(&l_out, &la, &nghbrs_out, &nghbrs1, 42, 43);
  list_apply(&nghbrs_out, print_one_neighbour);

  CU_ASSERT(nghbrs_out.nb_elem == 4); // TODO c'est pas suffisant
}



void test_UPDATE_NEIGHBOURS(void) {
  land la;
  neighbour n, n2, n3, n4, n_out;
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
     (NULL == CU_add_test(pSuite, "test of test_UPDATE_NEIGHBOURS()", test_UPDATE_NEIGHBOURS))

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
