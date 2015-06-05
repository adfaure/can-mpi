#include <stdio.h>

#include "../src/cartesian_space.h"

int main()
{
  neighbour n;
  unsigned int x = 0;
  unsigned int y = 0;
  unsigned int size = 1000;
  unsigned int orientation = 0;
  unsigned int comm_rank = 0;
  init_neighbour(&n, x, y, size, orientation, comm_rank);
  printf("lol");
  return 0;
}
