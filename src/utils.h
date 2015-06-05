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

//LOG define
#define LAND_LOG 0
#define NEIGHBOUR_LOG 1

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

int list_get_index_ptr(const list *l,int i, void **data);

void print_neighbour_cb(void *n);

void free_neighbour_cb(void *);

void list_apply(const list *l, void(*cb)(void * data));

void list_clear(list *l, void(*free_function)(void *data));

int list_replace_index(list * l, int i ,const void *data);

void land_extract_neighbourg_after_split(land *land1, land * land2, neighbour *n1, neighbour *n2);

int adjust_neighbour(land *land, neighbour *n);

void print_neighbour(const neighbour *n);

int find_neighbour(const list *l, const pair *pair, neighbour *res );

void neighbour_to_buffer(const list *l, unsigned int buffer[MAX_SIZE_BUFFER]);

void log_factory(FILE *f,const void *data, int CODE, int from);

double entire_dist_neigbourg(int x1, int y1, const neighbour *neighbour);

int update_neighbours(list *list,const land*land,  const neighbour *new_n);

int update_border(neighbour *n1, const neighbour *n2);

int is_over_neighbour(const neighbour *n1,const neighbour *n2);

int are_over_neighbour(const neighbour *n1,const neighbour *n2);

// voisin 1 contien voisin 2
// n1 x--------------x
// n2    x-------x
int is_contains_neighbour(const neighbour *n1,const neighbour *n2);

// return true if
// n1 : x--------------x
// n2 :         x-------------x
int is_over_neighbour_end(const neighbour *n1,const neighbour *n2);

// return true if
// n1 :         x--------------x
// n2 : x--------------x
int is_over_neighbour_begin(const neighbour *n1,const neighbour *n2);
