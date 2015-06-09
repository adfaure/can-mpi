#ifndef H_UTILS
#define H_UTILS

#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>

// world size
#define SIZE_X 1000
#define SIZE_Y 1000

//root process
#define ROOT_PROCESS 0

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
#define ATTACH_NEW_DATA      888

//LOG define
#define LAND_LOG      0
#define NEIGHBOUR_LOG 1
#define SVG_FORMAT    2

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
  unsigned int x, y;
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

typedef struct _land_storage {
	unsigned int element_size, size_x, size_y;
	void *** data;
} land_storage;

/**
 *
 */
void init_land_storage(land_storage *ls, unsigned int size_x,unsigned int size_y, unsigned int element_size);


/**
 * store a data to the specified position
 * Return true if everything is ok
 */
int land_storage_store_value(land_storage *ls, unsigned int x, unsigned int y,const void* data);


/**
 * Fetch a value to the spécified position
 */
int land_storage_fetch_value(const land_storage *ls, unsigned int x, unsigned int y, void* data);


/**
 * initialise une liste simplement chainée
 * @param element_size : taille de l'élément que l'on va stocker
 * TESTED
 */
void init_list(list *l, unsigned int element_size);

/**
 * Ajoute un élément en tête de la liste
 *
 * @pre l est une liste initialisée
 * @post l est de taille +1
 * TESTED
 */
void list_add_front(list *l,const void *data);

/**
 * Récupère dans @data le i-ème élément de la liste
 * Retourne 0 si ok, sinon -1
 * TESTED
 */
int list_get_index(const list *l, int i, void *data);

/**
 * Récupère dans @p des valeurs aléatoires dans [0, max_x] et [0, max_y]
 */
 void get_random_id(pair *p, int max_x, int max_y);

/**
 * Initiatise un couple de coordonnées
 */
void init_pair(pair *p, int x, int y);

/**
 * Affiche une pair sur la sortie standard
 */
void print_pair(const pair *p);

/**
 * Allocateur d'un tableau contigue en mémoire de taille @rows x @cols x sizeof(int)
 */
int **alloc_2d_int(int rows, int cols);

/**
 * Allocateur d'un tableau contigue en mémoire de taille @rows x @cols x sizeof(float)
 */
float **alloc_2d_float(int rows, int cols);

/**
 * Retourne un timestamp en millisecondes (nb de millisec depus 01-01-1970)
 */
long long now();

/**
 * Applique une fonction à chaque élément de la liste
 *  Ne modifie pas la liste
 */
void list_apply(const list *l, void(*cb)(void * data));

/**
 * Prend une fonction de callback pour désalouer chaque élement de la liste
 * puis désaloue la liste et place le nombre d'élément à 0
 */
void list_clear(list *l, void(*free_function)(void *data));

/**
 * Remplace le i-ème élément de la liste par l'élément @data
 */
int list_replace_index(list *l, int i, const void *data);

/**
 * Remove l'élément i de la liste ( i == 0 est le premier élément)
 */
int list_remove_index(list * l, int i , void(*free_function)(void *data));

/**
 *
 */
int list_remove_front(list *l, void(*free_function)(void *data));

/**
 *
 * TODO NOT TESTED
 */
int list_cp_revert(const list *src, void(*free_function)(void *data), list *dst);

#endif
