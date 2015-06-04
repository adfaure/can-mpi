#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

// max sizes
#define MAX_SIZE_BUFFER 100
#define MAX_SIZE_NEIGHBOUR 100

//LOG define
#define LAND_LOG 0
#define NEIGHBOUR_LOG 1


//root process
#define ROOT_PROCESS 0

// COmmunication tags
#define ROOT_TAG_INIT_NODE   0
#define GET_ENTRY_POINT      4
#define SEND_LAND_ORDER      65
#define ACK                  78
#define SEND_ENTRY_POINT     5
#define RES_REQUEST_TO_JOIN  12
#define SEND_NEIGBOUR_ORDER  34
#define LOCALIZE_RESP        13
#define IDL                  32
#define ACK_TAG_BOOTSTRAP    42
#define REQUEST_INIT_SPLIT   64
#define NEGOCIATE_LAND       123
#define REQUEST_RECEIVE_LAND 234
#define LOCALIZE             321
#define REQUEST_TO_JOIN      666
#define RES_INIT_NEIGHBOUR   56
#define UPDATE_NEIGBOUR      789

// world size
#define SIZE_X 1000
#define SIZE_Y 1000

// voisin variables
#define VOISIN_V           1 //vertical
#define VOISIN_H           2 //horizontal
#define VOISIN_TOP         3
#define VOISIN_BOT         4
#define VOISIN_BOT_RIGHT   5
#define VOISIN_BOT_LEFT    6
#define VOISIN_LEFT        7
#define VOISIN_RIGHT       8
#define VOISIN_TOP_RIGHT   9
#define VOISIN_TOP_LEFT    10
#define VOISIN_NONE        0


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef struct _pair {
  int x, y;
} pair;

typedef struct _land {
  unsigned int x, y;
  unsigned int size_x, size_y;
} land;

typedef struct _cell {
  void *data;
  struct _cell *next;
} cell;

typedef struct _neighbour  {
  unsigned int orientation; //
  unsigned int x, y; // begin point top left
  unsigned int size;
  unsigned int com_rank;
} neighbour;

typedef enum { false, true } bool;

typedef struct _list {
  int nb_elem;
  size_t element_size;
  cell *first;
} list;

/**
 * init a generique liste
 */
 void init_list(list *l, unsigned int element_size);

/**
 * add elem in a generique list
 */
void list_add_front(list *l, void *data );

/**
 * get elemnt i of the list return 0 if something wrong;
 */
int list_get_index(const  list *l,int i, void *data);

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
bool is_land_contains(const land *l, unsigned  int x,unsigned  int y);

/**
 * return true if the land contains the pair
 */
bool is_land_contains_pair(const land *l,const pair *p);

/**
 * split old_land and init new_land initialized to the half of old_land
 */
void split_land(land *new_land ,land *old_land);

/**
 *
 */
void split_land_update_neighbour(land *new_land , land *old_land, list *new_n ,list *old_n, int ,int);

/**
 *  init a land
 */
void init_land(land *l,unsigned  int x,unsigned  int y,unsigned int s_x,unsigned int s_y);

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
int CAN_Receive_neighbour(neighbour *neighbour,int mpi_tag ,int mpi_src , MPI_Comm comm );

/**
 *
 */
int CAN_Send_neighbour(const neighbour *neighbour,int mpi_tag ,int mpi_destinataire , MPI_Comm comm);

/**
 *
 */
int CAN_Send_neighbour_list(const list *l ,int mpi_tag ,int mpi_destinataire , MPI_Comm comm);

/**
 *
 */
long long now();

void init_neighbour(neighbour *n, unsigned int x, unsigned int y, unsigned int size , unsigned int or, unsigned int com_rank);

int is_neigbour(const land *land, const neighbour *n);

bool is_neigbour_top(const land *land, const neighbour *n);

bool is_neigbour_bot(const land *land, const neighbour *n);

bool is_neigbour_left(const land *land, const neighbour *n);

bool is_neigbour_right(const land *land, const neighbour *n);

void list_add_front(list * l, void *elem);

void init_list(list *l, unsigned int element_size);

void land_extract_neighbourg_after_split(land *land1, land * land2, neighbour *n1, neighbour *n2);

void adjust_neighbour(land *land, neighbour *n);

void print_neighbour(const neighbour *n);

int find_neighbour(const list *l, const pair *pair, neighbour *res );

void neighbour_to_buffer(const list *l, unsigned int buffer[MAX_SIZE_BUFFER]);

void log_factory(FILE *f,const void *data, int CODE, int from);

double entire_dist_neigbourg(int x1, int y1, const neighbour *neighbour);

void print_neighbour_cb(void *n);
void free_neighbour_cb(void *);
void list_apply(const list *l, void(*cb)(void * data));

void list_clear(list *l, void(*free_function)(void *data));

int main(int argc, char**argv) {
  FILE *file;
  file = fopen ("log.txt", "w+");

  if(!file) {
    printf(" erreur lors de l'ouverture du fichier de log \n");
    return 0;
  }
  fprintf(file, "log \n");
  int nb_proc, com_rank,node_number , i, corresp, localise;
  unsigned int land_buffer[4] , buffer_ui[MAX_SIZE_BUFFER], buffer_simple_int = 0;
  int wait_for = -1;
  int main_loop_tag, main_loop_from, count, main_loop_buffer_int[MAX_SIZE_BUFFER], send_int_buffer[MAX_SIZE_BUFFER], wait_array[2]; // wait_array on attend un message d'une source avec un tag
  bool bootstrap = false, active = false, is_waiting = false;
  list voisins, temp_voisins;
  neighbour neighbour_temp_find, temp_voisin;
  pair pair_id, pair_join_request;
  land land_id, new_land, temp_land;
  MPI_Status main_loop_status;
  MPI_Request main_loop_request;
  MPI_Init (&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);
  init_list(&voisins, sizeof(neighbour));
  init_list(&temp_voisins, sizeof(neighbour));
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
        } else if(main_loop_tag == GET_ENTRY_POINT) {
          main_loop_buffer_int[0] = 1;
          MPI_Send(&main_loop_buffer_int[0] ,1 , MPI_INT, i, SEND_ENTRY_POINT, MPI_COMM_WORLD );
        }
      }
    }

    int dummy = 0;
    for(int i = 0; i < 10000000; i++) {
      dummy = i * i;
    }

    for(i = 0; i < nb_proc; i++) {
      if(i != ROOT_PROCESS) {
          MPI_Send(&buffer_simple_int ,1 , MPI_INT, i, SEND_LAND_ORDER, MPI_COMM_WORLD);
          MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, i, ACK, MPI_COMM_WORLD, &main_loop_status);
          init_land(&temp_land, land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
          log_factory(file, &temp_land, LAND_LOG, i);
          MPI_Send(&buffer_simple_int ,1 , MPI_INT, i, SEND_NEIGBOUR_ORDER, MPI_COMM_WORLD);
          MPI_Probe(i, ACK, MPI_COMM_WORLD, &main_loop_status);
          MPI_Get_count (&main_loop_status, MPI_UNSIGNED, &count);
          MPI_Recv(&buffer_ui[0] ,count, MPI_UNSIGNED, i, ACK, MPI_COMM_WORLD, &main_loop_status);
          int nb_voisins = count / (sizeof(neighbour) / sizeof(unsigned int));
          int idx = 0;
          for(int j = 0; j < nb_voisins; j++) {
            init_neighbour(&temp_voisin, buffer_ui[idx], buffer_ui[idx+1], buffer_ui[idx+2],buffer_ui[idx+3], buffer_ui[idx+4]);
            list_add_front(&voisins, &temp_voisin);
            log_factory(file, &temp_voisin, NEIGHBOUR_LOG, i);
            print_neighbour(&temp_voisin);
            idx += 5;
          }
        }
    }

  } else {
    get_random_id(&pair_id, SIZE_X, SIZE_Y);
    while(1) {
      if(wait_for != -1) {
        // printf("je suis [%d] en attente d'un message de %d \n", com_rank, wait_for);
      }
      //  printf("[ %d ] en attente de message \n", com_rank);
      MPI_Probe(wait_for, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
      main_loop_from = main_loop_status.MPI_SOURCE;
      main_loop_tag  = main_loop_status.MPI_TAG;
      // printf("[ %d ],  recu %d, from %d \n", com_rank, main_loop_tag ,main_loop_from );
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
          MPI_Send(&main_loop_buffer_int[0] ,1 , MPI_INT ,ROOT_PROCESS ,GET_ENTRY_POINT ,MPI_COMM_WORLD);
          MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, ROOT_PROCESS, SEND_ENTRY_POINT, MPI_COMM_WORLD, &main_loop_status);
          send_int_buffer[0] = com_rank; send_int_buffer[1] = pair_id.x; send_int_buffer[2] = pair_id.y;
          MPI_Send((&send_int_buffer[0]), 3, MPI_INT, main_loop_buffer_int[0], REQUEST_TO_JOIN, MPI_COMM_WORLD);
        }
      }
      else if(main_loop_tag == REQUEST_TO_JOIN) {
      // Request de localisation, le noeud contenant la paire transporté répondra au noeud de la requete RES_REQUEST_TO_JOIN
        MPI_Recv(&main_loop_buffer_int[0] ,3, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        init_pair(&pair_join_request, main_loop_buffer_int[1], main_loop_buffer_int[2]);
        if(is_land_contains_pair(&land_id, &pair_join_request)) {
            MPI_Send(&com_rank, 1, MPI_INT, main_loop_buffer_int[0], RES_REQUEST_TO_JOIN,  MPI_COMM_WORLD);
            wait_for = main_loop_buffer_int[0];
        } else {
          if(find_neighbour(&voisins, &pair_join_request, &neighbour_temp_find)) {
            print_pair(&pair_join_request);
            MPI_Send(&main_loop_buffer_int[0], 3, MPI_INT, neighbour_temp_find.com_rank, REQUEST_TO_JOIN,  MPI_COMM_WORLD);
          } else {
            printf("ERROR lors de la recherche de voisins \n");
          }
        }
      }
      else if(main_loop_tag == SEND_LAND_ORDER) {
        MPI_Get_count (&main_loop_status, MPI_INT, &count);
        MPI_Recv(&buffer_simple_int , count , MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(main_loop_from == ROOT_PROCESS ) {
          printf("recu SEND_LAND_ORDER \n");
          land_buffer[0] = land_id.x; land_buffer[1] = land_id.y;land_buffer[2] = land_id.size_x;land_buffer[3] = land_id.size_y;
          MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, main_loop_from, ACK, MPI_COMM_WORLD);
        }
      }
      else if(main_loop_tag == RES_INIT_NEIGHBOUR) {
        int nb_voisins, idx = 0;
        MPI_Get_count (&main_loop_status, MPI_UNSIGNED, &count);
        MPI_Recv(&buffer_ui[0] ,count, MPI_UNSIGNED, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        nb_voisins = count / (sizeof(neighbour) / sizeof(unsigned int));
        for(int i = 0; i < nb_voisins; i++) {
          init_neighbour(&temp_voisin, buffer_ui[idx], buffer_ui[idx+1], buffer_ui[idx+2],buffer_ui[idx+3], buffer_ui[idx+4]);
          list_add_front(&voisins, &temp_voisin);
          if(main_loop_from == temp_voisin.com_rank ) {
            continue;
          }
          printf("MAJ des voisins ! %d \n", com_rank);
          CAN_Send_neighbour(&temp_voisin,UPDATE_NEIGBOUR, temp_voisin.com_rank, MPI_COMM_WORLD );
          print_neighbour(&temp_voisin);
          idx += 5;
        }
      }

      else if(main_loop_tag == UPDATE_NEIGBOUR) {
        CAN_Receive_neighbour(&temp_voisin, main_loop_tag, main_loop_from, MPI_COMM_WORLD);
        printf("Je suis %d, Mes amis je suis heureux de vous annoncer que nous acceuilons à présent un nouveau voisins [%d]! \n ", com_rank, main_loop_from);
      }

      else if(main_loop_tag == RES_REQUEST_TO_JOIN) {
        MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        MPI_Send(&(main_loop_buffer_int[0]),1 , MPI_INT , main_loop_from , REQUEST_INIT_SPLIT, MPI_COMM_WORLD);
        MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, main_loop_from, REQUEST_RECEIVE_LAND, MPI_COMM_WORLD, &main_loop_status);
        init_land(&land_id, land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
        print_land(&land_id);
      }

      else if(main_loop_tag == SEND_NEIGBOUR_ORDER) {
        neighbour_to_buffer(&voisins, buffer_ui);
        MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        MPI_Send((&buffer_ui[0]), voisins.nb_elem * (sizeof(neighbour)/sizeof(unsigned int)), MPI_UNSIGNED, main_loop_from, ACK, MPI_COMM_WORLD);
      }

      else if(main_loop_tag == REQUEST_INIT_SPLIT) {
        MPI_Get_count (&main_loop_status, MPI_INT, &count);
        MPI_Recv(&main_loop_buffer_int[0] ,count, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        split_land_update_neighbour(&new_land, &land_id, &temp_voisins, &voisins, main_loop_from , com_rank);
        land_buffer[0] = new_land.x; land_buffer[1] = new_land.y;land_buffer[2] = new_land.size_x;land_buffer[3] = new_land.size_y;
        MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, main_loop_from, REQUEST_RECEIVE_LAND, MPI_COMM_WORLD );
        CAN_Send_neighbour_list(&temp_voisins,RES_INIT_NEIGHBOUR , main_loop_from, MPI_COMM_WORLD);
        list_clear(&temp_voisins, free_neighbour_cb);
        printf("je suis %d , transaction fini avec %d \n",com_rank, wait_for);
        wait_for = -1;
      }

      else if(main_loop_tag == LOCALIZE) {
        corresp = main_loop_buffer_int[0];
        init_pair(&pair_join_request,  main_loop_buffer_int[1], main_loop_buffer_int[2]);
        if(is_land_contains_pair(&land_id, &pair_join_request)) {
          MPI_Send(&com_rank, 1, MPI_INT, corresp, LOCALIZE_RESP, MPI_COMM_WORLD);
        } else {
        }
      }

      else   {
        printf("unknow tag \n");
      }
    }
  }

  MPI_Finalize();
  fclose(file);
  return 0;
}

int CAN_Send_neighbour(const neighbour *neighbour,int mpi_tag ,int mpi_destinataire , MPI_Comm comm) {
  unsigned int buffer[5], size = (sizeof(neighbour) / sizeof(unsigned int));
  buffer[0] = neighbour->x;
  buffer[1] = neighbour->y;
  buffer[2] = neighbour->size;
  buffer[3] = neighbour->orientation;
  buffer[4] = neighbour->com_rank;
  MPI_Send(&buffer[0], size ,MPI_UNSIGNED ,mpi_destinataire, mpi_tag, comm);
  return 1;
}

int CAN_Send_neighbour_list(const list *l ,int mpi_tag ,int mpi_destinataire , MPI_Comm comm) {
  unsigned int buffer[MAX_SIZE_BUFFER], size = l->nb_elem * (sizeof(neighbour) / sizeof(unsigned int));
  neighbour_to_buffer(l, buffer);
  MPI_Send(&buffer[0], size ,MPI_UNSIGNED ,mpi_destinataire, mpi_tag, comm);
  return 1;
}

//MPI_Recv(&main_loop_buffer_int[0] ,count, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
int CAN_Receive_neighbour(neighbour *neighbour,int mpi_tag ,int mpi_src , MPI_Comm comm ) {
  unsigned int buffer[5], size = (sizeof(neighbour) / sizeof(unsigned int));
  MPI_Recv(&buffer[0], size, MPI_UNSIGNED,mpi_src, mpi_tag, comm, MPI_STATUS_IGNORE);
  init_neighbour(neighbour, buffer[0], buffer[1], buffer[2],buffer[3], buffer[4]);
  return 1;
}

void CAN_Recv_localise(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm) {
  MPI_Status status;
  int *buffer = (int*) malloc(sizeof(int) * 3);
  buffer[0] = self_rank;
  buffer[1] = pair->x;
  buffer[2] = pair->y;
  MPI_Send(&(buffer[0]), 3, MPI_INT, first_node, LOCALIZE, comm);
  MPI_Recv(loc, 1, MPI_UNSIGNED, MPI_ANY_SOURCE, LOCALIZE_RESP, comm, &status);
  free(buffer);
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

void init_land(land *l,unsigned  int x,unsigned  int y,unsigned int s_x,unsigned int s_y) {
  l->x = x;
  l->y = y;
  l->size_x = s_x;
  l->size_y = s_y;
}

bool is_land_contains(const land *l,unsigned int x,unsigned int y) {
  return ((l->x <= x && l->x + l->size_x > x) && (l->y <= y && l->y + l->size_y > y));
}

bool is_land_contains_pair(const land *l,const pair *p) {
  return ((l->x < p->x && l->x + l->size_x > p->x) && (l->y < p->y && l->y + l->size_y > p->y));
}

void split_land(land *new_land ,land *old_land) {
  unsigned int new_x, new_y, new_size_x, new_size_y;
  if(old_land->size_x >= old_land->size_y) {
    new_x = old_land->x + (old_land->size_x / 2);
    new_y = old_land->y;
    new_size_y = old_land->size_y;
    new_size_x = old_land->size_x / 2;
    old_land->size_x = new_size_x ;
  } else {
    new_x = old_land->x;
    new_y = old_land->y + (old_land->size_y / 2 );
    new_size_y = (old_land->size_y / 2);
    new_size_x = old_land->size_x;
    old_land->size_y = new_size_y ;
  }
  init_land(new_land, new_x, new_y, new_size_x, new_size_y);
}

void split_land_update_neighbour(land *new_land , land *old_land, list *new_n ,list *old_n, int new_rank, int old_rank) {
  neighbour temp, old, new;
  split_land(new_land, old_land);
  land_extract_neighbourg_after_split(new_land, old_land, &old, &new);
  old.com_rank = new_rank;
  new.com_rank = old_rank;
  for(int i = 0; i < old_n->nb_elem; i++) {
    list_get_index(old_n,i ,&temp);
    printf("voisins n %d \n" , i);
    print_neighbour(&temp);
    if(is_neigbour(new_land , &temp)) {
      adjust_neighbour(new_land, &temp);
      list_add_front(new_n, &temp);
    }
    list_get_index(old_n,i ,&temp);
    if(is_neigbour(old_land , &temp)) {
      adjust_neighbour(old_land, &temp);
    }
  }

  list_add_front(old_n, &old);
  list_add_front(new_n, &new);

  printf("affichage de la liste des voisins du nouveau noeud ! \n");
  list_apply(new_n, print_neighbour_cb);
  printf("affichage de la liste de mes nouveau voisins ! \n");
  list_apply(old_n, print_neighbour_cb);
}

void land_extract_neighbourg_after_split(land *land1, land * land2, neighbour *n1, neighbour *n2) {
  if(land1->x == land2->x) // alors ils sont l'un au dessu de l'autre
  {
    n1->size = land1->size_y;
    n2->size = land1->size_y;
    n2->x = land2->x;
    n1->x = land1->x;
    n1->orientation = VOISIN_H;
    n2->orientation = VOISIN_H;
    if(land1->y < land2->y) {
      n1->y = land2->y;
      n2->y = land1->y + land1->size_y;
    } else {
      n2->y = land1->y;
      n1->y = land2->y + land2->size_y;
    }
  } else if(land1->y == land2->y) {
    n1->size = land1->size_x;
    n2->size = land1->size_x;
    n2->y = land2->y;
    n1->y = land1->y;
    n1->orientation = VOISIN_V;
    n2->orientation = VOISIN_V;
    if(land1->x < land2->x) {
      n1->x = land2->x;
      n2->x = land1->x + land1->size_x;
    } else {
      n2->x = land1->x;
      n1->x = land2->x + land2->size_x;
    }
  } else {
    printf("impossible \n");
  }
}

int find_neighbour(const list *l, const pair *pair, neighbour *res ) {
  neighbour temp;
  int better;
  double min, val_temp;
  if(l->nb_elem == 0) {
    return 0;
  }
  min = 10000000; better = 0;
  for(int i = 0 ; i < l->nb_elem; i++) {
    list_get_index(l, i, &temp);
    val_temp = entire_dist_neigbourg(pair->x, pair->y, &temp);
    if(val_temp < min) {
      min = val_temp;
      better = i;
    }
  }
  list_get_index(l, better, res);
  return 1;
}

double entire_dist_betwen_points(int x1, int y1, int x2, int y2) {
    return sqrt(pow((double)(x2 - x1), 2) + pow((double)(y2 - y1),2 ));
}

double entire_dist_neigbourg(int x1, int y1, const neighbour *neighbour) {
    int x, y;
    if(neighbour->orientation == VOISIN_H) {
      y = neighbour->y;
      x = neighbour->x + (neighbour->size / 2);
    } else {
      x = neighbour->x;
      y = neighbour->y + (neighbour->size / 2);
    }
    return sqrt(pow((double)(x - x1), 2) + pow((double)(y - y1),2 ));
}

void adjust_neighbour(land *land, neighbour *n) {
  if(n->orientation == VOISIN_V) {
    if(n->x < land->x) {
      n->size = n->size - (land->x - n->x);
      n->x = land->x;
    }
    if(n->x > land->x + land->size_x) {
      n->size = n->size - (n->x - land->x);
      n->x = land->x + land->size_x;
    }
  } else if(n->orientation == VOISIN_H) {
    if(n->y < land->y) {
      n->size = n->size - (land->y - n->y);
      n->y = land->y;
    }
    if(n->y > land->y + land->size_y) {
      n->size = n->size - (n->y - land->y);
      n->y = land->y + land->size_y;
    }
  }
}

void print_land(const land *l) {
  printf("((%u, %u), (%u , %u))\n", l->x,  l->size_x, l->y, l->size_y);
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

void init_neighbour(neighbour *n, unsigned int x, unsigned int y, unsigned int size , unsigned int or, unsigned int com_rank) {
  n->size = size;
  n->x = x;
  n->y = y;
  n->orientation = or;
  n->com_rank = com_rank;
}

int is_neigbour(const land *land,const neighbour *n) {
  if(is_neigbour_top(land, n)) {
    return VOISIN_TOP;
  } else if (is_neigbour_bot(land, n)) {
    return VOISIN_BOT;
  } else if(is_neigbour_left(land, n)) {
    return VOISIN_LEFT;
  } else if(is_neigbour_right(land, n)) {
    return VOISIN_RIGHT;
  } else {
    return VOISIN_NONE;
  }
}

void neighbour_to_buffer(const list *l, unsigned int buffer[MAX_SIZE_BUFFER]) {
  neighbour temp;
  int idx = 0;
  for(int i = 0; i < l->nb_elem; i++) {
    list_get_index(l,i ,&temp);
    buffer[idx]   = temp.x;
    buffer[idx+1] = temp.y;
    buffer[idx+2] = temp.size;
    buffer[idx+3] = temp.orientation;
    buffer[idx+4] = temp.com_rank;
    idx += 5;
  }
}

bool is_neigbour_top(const land *land, const neighbour *n) {
    if(n->orientation == VOISIN_V)
      return false;

    bool temp = (land->x < n->x && land->x + land->size_x > n->x) || (land->x < n->x + n->size  && land->x + land->size_x > n->x + n->size );
    return temp && (land->y == n->y);
}

bool is_neigbour_bot(const land *land, const neighbour *n) {
    if(n->orientation == VOISIN_V)
      return false;

    bool temp = (land->x < n->x && land->x + land->size_x > n->x) || (land->x < n->x + n->size && land->x + land->size_x > n->x + n->size);
    return temp && (n->y == land->y + land->size_y);
}

bool is_neigbour_left(const land *land, const neighbour *n) {
    if(n->orientation == VOISIN_H)
      return false;

    bool temp = (land->y < n->y && land->y + land->size_y > n->y) || (land->y < n->y + n->size  && land->y + land->size_y > n->y + n->size);
    return temp && (n->x == land->x);
}

bool is_neigbour_right(const land *land, const neighbour *n) {
    if(n->orientation == VOISIN_H)
      return false;

    bool temp = (land->y < n->y && land->y + land->size_y > n->y) || (land->y < n->y + n->size  && land->y + land->size_y > n->y + n->size );
    return temp && (n->x == land->x + land->size_x);
}

void print_neighbour(const neighbour *n) {
  if(n->orientation == VOISIN_V) {
    printf("|");
    printf(" [%u] (%d, %d) , (%d) \n",n->com_rank ,n->x,n->y,n->x + n->size);
  } else if(n->orientation == VOISIN_H) {
    printf("--");
    printf(" [%u] (%d, %d) , (%d) \n",n->com_rank ,n->x,n->y ,n->y  + n->size);
  } else {
    printf("invalid neighbour \n");
  }
}

void free_neighbour_cb(void *n) {
  neighbour elem = *((neighbour *) n);
}

void print_neighbour_cb(void *n) {
  neighbour elem = *((neighbour *) n);
  print_neighbour(&elem);
}

void list_apply(const list *l, void(*cb)(void * data)) {
  cell *current = l->first;
  for(int i = 0 ; i < l->nb_elem; i++) {
    cb(current->data);
    current = current->next;
  }
}

void init_list(list *l, unsigned int element_size) {
  l->nb_elem = 0;
  l->first = NULL;
  l->element_size = element_size;
}

void list_add_front(list * l, void *elem) {
  cell *temp,*new_cell = (cell*) malloc(sizeof(cell));
  new_cell->data = malloc(l->element_size);
  memcpy(new_cell->data, elem, l->element_size);
  temp = l->first;
  l->first = new_cell;
  new_cell->next = temp;
  l->nb_elem++;
}

int list_get_index(const list *l,int i, void *data) {
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

void list_clear(list *l, void(*free_function)(void *data)) {
  cell *current = l->first, *temp;
  for(int i = 0 ; i < l->nb_elem; i++) {
    free_function(current->data);
    temp = current;
    current = current->next;
    free(current);
  }
  l->first = NULL;
  l->nb_elem = 0;
}

void log_factory(FILE *f, const void *data, int CODE, int from) {
  if(from != -1) {
    fprintf(f," -> %d ", from);
  }
  land temp_land;
  neighbour temp_neighbour;
  switch(CODE) {
    case LAND_LOG :
      memcpy(&temp_land,data, sizeof(land));
      fprintf(f, "((%u, %u), (%u , %u))\n", temp_land.x,  temp_land.size_x, temp_land.y, temp_land.size_y);
      break;
    case NEIGHBOUR_LOG :
      memcpy(&temp_neighbour, data, sizeof(neighbour));
      if(temp_neighbour.orientation == VOISIN_V) {
        fprintf(f, "|");
        fprintf(f, " [%u] (%d, %d) , (%d) \n",temp_neighbour.com_rank ,temp_neighbour.x,temp_neighbour.y,temp_neighbour.x + temp_neighbour.size);
      } else if(temp_neighbour.orientation == VOISIN_H) {
        fprintf(f,"--");
        fprintf(f," [%u] (%d, %d) , (%d) \n",temp_neighbour.com_rank ,temp_neighbour.x,temp_neighbour.y ,temp_neighbour.y  + temp_neighbour.size);
      } else {
        fprintf(f, "invalid neighbour \n");
      }
      break;
  }
  fflush(f);
}
