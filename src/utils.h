#ifndef H_UTILS
#define H_UTILS

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

// Communication tags
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

typedef enum { false, true } bool;

typedef struct _cell {
  void *data;
  struct _cell *next;
} cell;

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
long long now();

void list_add_front(list * l, void *elem);

void init_list(list *l, unsigned int element_size);

int list_get_index_ptr(const list *l,int i, void **data);

void list_apply(const list *l, void(*cb)(void * data));

void list_clear(list *l, void(*free_function)(void *data));

int list_replace_index(list * l, int i ,const void *data);

void log_factory(FILE *f,const void *data, int CODE, int from);

#endif
