#ifndef H_CARTESIAN_SPACE
#define H_CARTESIAN_SPACE

#include "utils.h"

typedef struct _land {
  unsigned int x, y;
  unsigned int size_x, size_y;
} land;

typedef struct _neighbour  {
  unsigned int orientation; //
  unsigned int x, y; // begin point top left
  unsigned int size;
  unsigned int com_rank;
} neighbour;


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

void init_neighbour(neighbour *n, unsigned int x, unsigned int y, unsigned int size , unsigned int or, unsigned int com_rank);

int is_neigbour(const land *land, const neighbour *n);

bool is_neigbour_top(const land *land, const neighbour *n);

bool is_neigbour_bot(const land *land, const neighbour *n);

bool is_neigbour_left(const land *land, const neighbour *n);

bool is_neigbour_right(const land *land, const neighbour *n);

void print_neighbour_cb(void *n);

void free_neighbour_cb(void *);

void land_extract_neighbourg_after_split(land *land1, land * land2, neighbour *n1, neighbour *n2);

int adjust_neighbour(land *land, neighbour *n);

void print_neighbour(const neighbour *n);

int find_neighbour(const list *l, const pair *pair, neighbour *res );

void neighbour_to_buffer(const list *l, unsigned int buffer[MAX_SIZE_BUFFER]);


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
 * Journalise dans un fichier de log
 */
void log_factory(FILE *f, const void *data, int CODE, int from);

#endif
