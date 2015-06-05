#ifndef H_CARTESIAN_SPACE
#define H_CARTESIAN_SPACE

#include "utils.h"

/**
 * Structure représentant une zone rectangulaire dans l'overlay
 * @param x: coordonnée sur les colonnes du coin en haut à gauche
 * @param y: coordonnée sur les colonnes du coin en haut à gauche
 */
typedef struct _land {
  unsigned int x, y;
  unsigned int size_x, size_y;
} land;

/**
 * Structure représentant un segment
 * @param x: coordonnée sur les colonnes du coin en haut à gauche
 * @param y: coordonnée sur les colonnes du coin en haut à gauche
 * @param orientation: verticale ou hoizontale
 * @param size: taille du segment
 * @param com_rank: communicateur permettant de communiquer avec le voisin (dont le segment est la frontière)
 */
typedef struct _neighbour {
  unsigned int orientation;
  unsigned int x, y;
  unsigned int size;
  unsigned int com_rank;
} neighbour;

/**
 *
 */
int CAN_Receive_neighbour(neighbour *neighbour, int mpi_tag, int mpi_src, MPI_Comm comm );

/**
 *
 */
int CAN_Send_neighbour(const neighbour *neighbour, int mpi_tag, int mpi_destinataire, MPI_Comm comm);

/**
 *
 */
int CAN_Send_neighbour_list(const list *l, int mpi_tag, int mpi_destinataire, MPI_Comm comm);


/**
 * Initiatise un voisin en position @x, @y, de taille @size,
 *  avec l'orientation @or, et accessible via @com_rank
 * TESTED
 */
void init_neighbour(neighbour *n, unsigned int x, unsigned int y, unsigned int size, unsigned int or, unsigned int com_rank);

/**
 * Vérifie si le voisin neighbour partage une frontière avec land.
 *  Cette frontière peut etre complete, partielle, incluse
 * Retourne VOISIN_TOP|VOISIN_BOT|VOISIN_LEFT|VOISIN_RIGHT
 *
 * TODO test
 */
int is_neighbour(const land *land, const neighbour *n);

/**
 * Retourne vrai une frontière en haut avec land possède un voisin neighbour sur sa frontière gauche.
 * PARTIALLY TESTED
 */
bool is_neighbour_top(const land *land, const neighbour *n);

/**
 * Retourne vrai une frontière en bas avec land possède un voisin neighbour sur sa frontière gauche.
 */
bool is_neighbour_bot(const land *land, const neighbour *n);

/**
 * Retourne vrai une frontière a gauche avec land possède un voisin neighbour sur sa frontière gauche.
 */
bool is_neighbour_left(const land *land, const neighbour *n);

/**
 * Retourne vrai une frontière a droite avec land possède un voisin neighbour sur sa frontière gauche.
 */
bool is_neighbour_right(const land *land, const neighbour *n);

/**
 * Fonction de callback permettant d'afficher un neighbour
 * utile dans @list_apply
 */
void print_neighbour_cb(void *n);

/**
 * Fonction de callback permettant de désalouer un neighbour
 * utile dans @list_clear
 */
void free_neighbour_cb(void *);


/**
 * A utiliser apèrs @split_land
 * Permet d'extraire les deux frontières (@n1, @n2)
 *  résultantes d'un split qui a généré land1 et land2
 */
void land_extract_neighbourg_after_split(land *land1, land *land2, neighbour *n1, neighbour *n2);

/**
 * Renormalise/Ajuste une frontière (@neigbour) pour correspondre aux vrais frontières du @land
 */
int adjust_neighbour(land *land, neighbour *n);

/**
 * Affiche une frontière
 */
void print_neighbour(const neighbour *n);

/**
 * Place dans @res le voisin qui va servir au prochain saut lors du routage
 * En calculant les distances euclidiennes entre pair et chaque milieu de segment de la liste
 */
int find_neighbour(const list *l, const pair *pair, neighbour *res );

/**
 * Remplit le buffer avec les valeurs de la liste TODO
 */
void neighbour_to_buffer(const list *l, unsigned int buffer[MAX_SIZE_BUFFER]);

// retourne la distance entre le milieu de neighbour et x,y
double entire_dist_neigbourg(int x1, int y1, const neighbour *neighbour);

// recoit une nouvelle frontière (new_n) et met à jour la liste en focntione de ça et
// TODO F42
int update_neighbours(list *list, const land *land, const neighbour *new_n);

// Retire de chaque neigbour les zone qui se chauvechent entre n1 et n2
int update_border(neighbour *n1, const neighbour *n2);

//
int is_over_neighbour(const neighbour *n1, const neighbour *n2);

//
int are_over_neighbour(const neighbour *n1, const neighbour *n2);

/**
 * retourne vrai si la frontière n2 est strictement inclue dans n1
 * n1 x--------------x
 * n2    x-------x
 *
 * TESTED
 */
int is_contains_neighbour(const neighbour *n1, const neighbour *n2);

// return true if
// n1 : x--------------x
// n2 :         x-------------x
int is_over_neighbour_end(const neighbour *n1, const neighbour *n2);

// return true if
// n1 :         x--------------x
// n2 : x--------------x
int is_over_neighbour_begin(const neighbour *n1, const neighbour *n2);

/**
 * return true if the land cotains the point defined by x and y is
 * TESTED
 */
bool is_land_contains(const land *l, unsigned  int x, unsigned  int y);

/**
 * return true if the land contains the pair
 * TESTED
 */
bool is_land_contains_pair(const land *l, const pair *p);

/**
 * split old_land and init new_land initialized to the half of old_land
 * TESTED
 */
void split_land(land *new_land, land *old_land);

/**
 *
 * TODO TEST
 */
void split_land_update_neighbour(land *new_land, land *old_land, list *new_n, list *old_n, int, int);

/**
 *  allocateur d'un land
 * TESTED
 */
void init_land(land *l, unsigned  int x, unsigned  int y, unsigned int s_x, unsigned int s_y);

/**
 *  Désalocation d'un land
 */
void free_land(land *l);

/**
 * Affichage d'un objet land
 */
void print_land(const land *l);

/**
 * Journalise dans un fichier de log
 */
void log_factory(FILE *f, const void *data, int CODE, int from);

#endif
