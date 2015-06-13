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
#define MAX_SIZE_BUFFER 10000
#define MAX_SIZE_BUFFER_CHAR 10000
#define MAX_SIZE_NEIGHBOUR 100

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


//LOG define
#define LAND_LOG      0
#define NEIGHBOUR_LOG 1
#define SVG_FORMAT    2

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

typedef struct _data {
	void *data;
	unsigned int element_size, data_type;
} can_data;

typedef struct _land_storage {
	unsigned int element_size, size_x, size_y;
	can_data ***data;
} land_storage;

typedef struct _chunk {
    can_data data_wrapper;
	unsigned int x, y;
} chunk;

/**
 *
 */
void init_chunk(chunk * chunk, unsigned int x, unsigned int y, const can_data * data_wrapper);

/**
 *
 */
void get_data( const chunk * chunk,can_data * data_wrapper);

/**
 *
 */
void print_one_chunk(void * c);

/**
 *
 */
void free_chunk_cb(void * elem);

/**
 * Place dans found le premier élement de la liste l pour lequel le calback cb retourne vrai
 *  Remarque: si aucun élément ne match, alors found contiendra le dernier element de la liste
 */
int list_find(const list * l,const void* params, int(*cb)(const void *elem, const void*params), void * found);

/**
 *
 */
void init_data(can_data *data, unsigned int data_size, unsigned int data_type, const void *elem);

/**
 *
 */
void free_can_data_(can_data *data);

/**
 *
 */
void can_data_get_element(const can_data *c_data, void *elem);

/**
 *
 */
void init_land_storage(land_storage *ls, unsigned int size_x,unsigned int size_y);

/**
 *
 */
void print_data(const can_data *data);


/**
 * store a data to the specified position
 * Return true if everything is ok
 */
int land_storage_store_value(land_storage *ls, unsigned int x, unsigned int y,const can_data* data);


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
 * Affiche une pair sur la sortie standard
 */
void print_pair_cb(void *p);

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
