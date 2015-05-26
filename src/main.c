#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define ROOT_PROCESS 0

// thus tag can only be send by the root node
#define ROOT_TAG_INIT_NODE 0

//
#define ACK_TAG 42
#define REQUEST_TO_JOIN 666

#define SIZE_X 1000
#define SIZE_Y 1000


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
  int main_loop_tag, main_loop_from;
  bool bootstrap = false, active = false;
  pair pair_id;
  land land_id;
  MPI_Status main_loop_status;
  MPI_Init (&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);

  srand((unsigned) time(NULL) + com_rank * nb_proc);

  if(com_rank == ROOT_PROCESS) {
    for(i = 0; i < nb_proc; i++) {
      if(i != ROOT_PROCESS) {
        MPI_Send(&i, 1, MPI_INT, i, ROOT_TAG_INIT_NODE, MPI_COMM_WORLD);
        MPI_Recv(&buffer_int ,1 , MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
        main_loop_from = main_loop_status.MPI_SOURCE;
        main_loop_tag  = main_loop_status.MPI_TAG;
        if(main_loop_tag == ACK_TAG) {
          printf("bootstraping node did it well \n");
        } else if(main_loop_tag == REQUEST_TO_JOIN) {
          buffer_int = 1;
          MPI_Send(&buffer_int, 1, MPI_INT, i, REQUEST_TO_JOIN, MPI_COMM_WORLD );
        }
      }
    }
  } else {
    get_random_id(&pair_id, SIZE_X, SIZE_Y);
    while(1) {
      MPI_Recv(&node_number ,1 , MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
      main_loop_from = main_loop_status.MPI_SOURCE;
      main_loop_tag  = main_loop_status.MPI_TAG;
      if(main_loop_tag == ROOT_TAG_INIT_NODE) {
        //initialisation of bootstrap node
        if(bootstrap == false && node_number == 1) {
          bootstrap   = true;
          buffer_int = 0;
          init_land(&land_id, 0, 0, SIZE_X, SIZE_Y);
          MPI_Send(&buffer_int ,1 ,MPI_INT ,ROOT_PROCESS ,ACK_TAG ,MPI_COMM_WORLD);
        } else {
          MPI_Send(&buffer_int ,1 ,MPI_INT ,ROOT_PROCESS ,REQUEST_TO_JOIN ,MPI_COMM_WORLD);
        }
      } else if(main_loop_tag == REQUEST_TO_JOIN) {
        printf("voir avec %d pour s'insérer \n", node_number);
      } else {
          printf("unknow tag");
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
  return ((l->x < x && l->x + l->size_x > x) && (l->y < y && l->y + l->size_y > y));
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
