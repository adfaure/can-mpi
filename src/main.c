#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ROOT_PROCESS 0
#define SIZE_X 1000
#define SIZE_Y 1000

#define TAG_INIT_NODE 0

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef struct _pair {
  int x, y;
} pair;

typedef struct _land {
  int x, y;
  unsigned int size_x, size_y;
} land;

typedef enum { false, true } bool;

/**
 * Get a pair with random value between [min, max]
 */
void get_random_id(pair *p, int min, int max);

/**
 * affiche une pair sur la sortie standard
 */
void print_pair(const pair *p);

/**
 * return true if the land cotains the point defined by x and y is
 */
bool is_land_contains(const land *l, int x, int y);

/**
 * split old_land and init new_land initialized to the half of old_land
 */
void split_land(land *new_land ,land *old_land);

/**
 *  init a land
 */
void init_land(land *l, int x, int y,unsigned int s_x,unsigned int s_y);

/**
 *  init a land
 */
void free_land(land *l);

/**
 *
 */
void print_land(const land *l);

/**
 *
 */
int **alloc_2d_int(int rows, int cols);

/**
 *
 */
float **alloc_2d_float(int rows, int cols);

int main(int argc, char**argv) {
  int nb_proc, com_rank,node_number ,buffer_int, i;
  bool bootsrap = false;
  pair pair_id;
  land land_id;
  MPI_Status status;
  MPI_Init (&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);

  srand((unsigned) time(NULL) + com_rank * nb_proc);

  if(com_rank == ROOT_PROCESS) {
    for(i = 0 ; i < nb_proc; i++) {
      if(i != ROOT_PROCESS) {
        MPI_Send(&i, 1, MPI_INT, i, TAG_INIT_NODE, MPI_COMM_WORLD);
      }
    }
  } else {
    get_random_id(&pair_id, SIZE_X, SIZE_Y);
    while(1==1) {
      MPI_Recv(&node_number,1 ,MPI_INT, ROOT_PROCESS, MPI_ANY_TAG, MPI_COMM_WORLD , &status);
      if(bootsrap == false && node_number == 1 ) {
        bootsrap = true;
        init_land(&land_id, 0, 0, SIZE_X, SIZE_Y);
      } else {
        printf("recu %d avec tag : %d from %d \n", node_number, status.MPI_TAG, status.MPI_SOURCE);
      }
    }
  }

  MPI_Finalize();
  return 0;
}

void get_random_id(pair *p, int min, int max) {
  p->x = rand() % max;
  p->y = rand() % max;
}

void print_pair(const pair *p) {
  printf("paire : (%d ,%d) \n", p->x, p->y);
}

void init_land(land *l, int x, int y,unsigned int s_x,unsigned int s_y) {
  l->x = x;
  l->y = y;
  l->size_x = s_x;
  l->size_y = s_y;
}

bool is_land_contains(const land *l, int x, int y) {
  return ((l->x < x && l->x + l->size_x > x) && l->y < y && l->y + l->size_y > y);
}

void split_land(land *new_land ,land *old_land) {
  int new_x, new_y, new_size_x, new_size_y, is_odd_x, is_odd_y;
  if(old_land->size_x > old_land->size_y) {
    is_odd_x = (old_land->size_x % 2) ? 0 : 1;
    new_x = old_land->x + (old_land->size_x / 2) + is_odd_x ;
    new_y = old_land->y;
    new_size_y = old_land->y;
    new_size_x = old_land->size_x / 2;
    old_land->size_x = new_size_x + is_odd_x;
  } else {
    is_odd_y = (old_land->size_y % 2) ? 0 : 1;
    new_x    = old_land->x;
    new_y = old_land->y + (old_land->size_y / 2 ) + is_odd_y;
    new_size_y = old_land->size_y / 2;
    new_size_x = old_land->size_x;
    old_land->size_y = new_size_y + is_odd_y;
  }
  init_land(new_land, new_x, new_y, new_size_x, new_size_y);
}

void print_land(const land *l) {
  printf("((%d , %u), (%d , %u))", l->x, l->size_x, l->y, l->size_y);
}

float **alloc_2d_float(int rows, int cols) {
    float *data = (float *)malloc(rows*cols*sizeof(float));
    float **array= (float **)malloc(rows*sizeof(float*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

int **alloc_2d_int(int rows, int cols) {
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i < rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}
