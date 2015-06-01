
#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_SIZE_BUFFER 100
#define MAX_SIZE_NEIGHBOUR 100

#define ROOT_PROCESS 0

// thus tag can only be send by the root node
#define ROOT_TAG_INIT_NODE 0

#define ACK_TAG_BOOTSTRAP 42
#define REQUEST_TO_JOIN   666
#define NEGOCIATE_LAND    123
#define LOCALIZE          321
#define LOCALIZE_RESP     13

#define SIZE_X 1000
#define SIZE_Y 1000

#define VOISIN_T 0 //top
#define VOISIN_B 1 //bot
#define VOISIN_L 2 //left
#define VOISIN_R 3 //right

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

typedef struct _neighbour  {
  int orientation; //
  int x, y; // begin point top left
  int size_x, size_y;
} neighbour;

typedef enum { false, true } bool;

typedef struct _cell {
  void *data;
  cell *next;
} cell;

typedef struct _list {
  int nb_elem;
  unsigned element_size;
  cell *first;
} list;

/**
 * init a generique liste
 */
void init_list(list *l);

/**
 * add elem in a generique list
 */
void list_add_front(list *l, void *data );

/**
 * get elemnt i of the list return 0 if something wrong;
 */
int list_get_index(list *l,int i, void *data);

/**
 * Get a pair with random value between [min, max]
 */
void get_random_id(pair *p, int min, int max);

/**
 * Init a pair
 */
void init_pair(pair *, int x, int y);

/**
 * affiche une pair sur la sortie standard
 */
void print_pair(const pair *p);

/**
 * return true if the land cotains the point defined by x and y is
 */
bool is_land_contains(const land *l, int x, int y);

/**
 * return true if the land contains the pair
 */
bool is_land_contains_pair(const land *l,const pair *p);

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

/**
 *
 */
void CAN_Recv_localise(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm);

/**
 *
 */
void CAN_Recv_localise_timeout(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm, long long timeout);

/**
 *
 */
long long now();

int main(int argc, char**argv) {
  int nb_proc, com_rank,node_number , i, corresp, localise;
  int main_loop_tag, main_loop_from, count, main_loop_buffer_int[MAX_SIZE_BUFFER], send_int_buffer[MAX_SIZE_BUFFER], wait_array[2]; // wait_array on attend un message d'une source avec un tag
  bool bootstrap = false, active = false, is_waiting = false;
  pair pair_id, pair_join_request;
  land land_id, new_land;
  MPI_Status main_loop_status;
  MPI_Request main_loop_request;
  MPI_Init (&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);

  srand((unsigned) time(NULL) + com_rank * nb_proc);

  if(com_rank == ROOT_PROCESS) {
    for(i = 0; i < nb_proc; i++) {
      if(i != ROOT_PROCESS) {
        MPI_Send(&i, 1, MPI_INT, i, ROOT_TAG_INIT_NODE, MPI_COMM_WORLD);
        MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
        main_loop_from = main_loop_status.MPI_SOURCE;
        main_loop_tag  = main_loop_status.MPI_TAG;
        if(main_loop_tag == ACK_TAG_BOOTSTRAP) {
          printf("bootstraping node did it well \n");
        } else if(main_loop_tag == REQUEST_TO_JOIN) {
          main_loop_buffer_int[0] = i;
          MPI_Send(&main_loop_buffer_int[0] , 1, MPI_INT, i, REQUEST_TO_JOIN, MPI_COMM_WORLD );
        }
      }
    }
  } else {
    get_random_id(&pair_id, SIZE_X, SIZE_Y);
    while(1) {
      MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
      main_loop_from = main_loop_status.MPI_SOURCE;
      main_loop_tag  = main_loop_status.MPI_TAG;
      if(main_loop_tag == ROOT_TAG_INIT_NODE) {
        MPI_Get_count (&main_loop_status, MPI_INT, &count);
        MPI_Recv(&main_loop_buffer_int[0] ,count , MPI_INT, main_loop_from,
          main_loop_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(bootstrap == false && *main_loop_buffer_int == 1) {
          bootstrap   = true;
          main_loop_buffer_int [0] = 0;
          init_land(&land_id, 0, 0, SIZE_X, SIZE_Y);
          MPI_Send(&main_loop_buffer_int[0] ,1 ,MPI_INT ,ROOT_PROCESS ,ACK_TAG_BOOTSTRAP ,MPI_COMM_WORLD);
        } else {
          main_loop_buffer_int[0] = com_rank;
          MPI_Send(&main_loop_buffer_int[0] ,1 ,MPI_INT ,ROOT_PROCESS ,REQUEST_TO_JOIN ,MPI_COMM_WORLD);
        }
      } else if(main_loop_tag == REQUEST_TO_JOIN) {
        MPI_Get_count (&main_loop_status, MPI_INT, &count);
        MPI_Recv(&main_loop_buffer_int[0] ,count , MPI_INT, main_loop_from,
          main_loop_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(main_loop_from == 0) {
          send_int_buffer[0] = com_rank; send_int_buffer[1] = pair_id.x; send_int_buffer[2] = pair_id.y;
          CAN_Recv_localise_timeout(&localise, &pair_id, com_rank, *main_loop_buffer_int, MPI_COMM_WORLD, 1000);
          if(localise == -1) {
            printf("localise timeouted \n");
          } else {

          }
        } else {
          printf("le noeud %d asked me to join \n", main_loop_buffer_int[0]);
        }
      } else if( main_loop_tag == LOCALIZE ) {
        corresp = main_loop_buffer_int[0];
        init_pair(&pair_join_request,  main_loop_buffer_int[1], main_loop_buffer_int[2]);
        if(is_land_contains_pair(&land_id, &pair_join_request)) {
          printf("its here ! \n");
          MPI_Send(&com_rank, 1, MPI_INT, corresp, LOCALIZE_RESP, MPI_COMM_WORLD);
        } else {
          // trouver un voisin et transmettre la requete.
        }
      } else   {
        printf("unknow tag \n");
      }
    }
  }

  MPI_Finalize();
  return 0;
}

void CAN_Recv_localise(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm) {
  MPI_Status status;
  int *buffer = (int*) malloc(sizeof(int) * 3);
  buffer[0] = self_rank;
  buffer[1] = pair->x;
  buffer[2] = pair->y;
  MPI_Send(&(buffer[0]), 3, MPI_INT, first_node, LOCALIZE, comm);
  MPI_Recv(loc, 1, MPI_INT, MPI_ANY_SOURCE, LOCALIZE_RESP, comm, &status);
  free(buffer);
}

void CAN_Negociate_land(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm) {
  printf("negociation betwen %d and %d", self_rank, first_node);
}

// int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag,
    // MPI_Status *status)
void CAN_Recv_localise_timeout(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm, long long timeout) {
  int *buffer = (int*) malloc(sizeof(int) * 3), flag;
  MPI_Status status;
  unsigned long begin_time = 0, time_elapsed = 0;
  buffer[0] = self_rank;
  buffer[1] = pair->x;
  buffer[2] = pair->y;
  MPI_Send(&(buffer[0]), 3, MPI_INT, first_node, LOCALIZE, comm);
  begin_time = now();
  while(time_elapsed < timeout) {
    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &flag, &status);
    if(flag) {
      MPI_Recv(loc, 1, MPI_INT, MPI_ANY_SOURCE, LOCALIZE_RESP, comm, &status);
      return;
    }
    time_elapsed = now() - begin_time;
  }
  *loc = -1;
  free(buffer);
}

void get_random_id(pair *p, int min, int max) {
  p->x = rand() % max;
  p->y = rand() % max;
}


void init_pair(pair *p, int x, int y) {
  p->x = x;
  p->y = y;
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

bool is_land_contains_pair(const land *l,const pair *p) {
  return ((l->x < p->x && l->x + l->size_x > p->x) && (l->y < p->y && l->y + l->size_y > p->y));
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

long long now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long millisecondsSinceEpoch =
            (unsigned long long)(tv.tv_sec) * 1000 +
            (unsigned long long)(tv.tv_usec) / 1000;
    return millisecondsSinceEpoch;
}

void init_neighbour(neighbour *n, int x, int y, int size_x, int size_y, int or) {
  n->size_y = size_y;
  n->size_x = size_x;
  n->x = x;
  n->y = y;
  n->orientation = or;
}

void init_list(list *l,unsigned int element_size ) {
  l->nb_elem = 0:
  l->first = NULL;
  l->element_size = element_size;
}

void list_add_front(list * l, void *elem) {
  cell *temp,*new_cell = (cell*) malloc(sizeof(cell));
  cell->data = malloc(l->element_size);
  memcpy(cell->data, elem, l->element_size);
  temp = l->first;
  l->first = new_cell;
  new_cell->next = temp;
}

int list_get_index(list *l,int i, void *data) {
  if (i >= l->nb_elem || i < 0 ) return 0;
  cell *current = l->first;
  int acc = 0;
  while(acc != i){
    current = current->next;
    acc++;
  }
  memcpy(data, current->data, l->element_size);
  return 1;
}
