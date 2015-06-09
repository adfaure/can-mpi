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
 * @brief CAN_Receive_neighbour
 * @param neighbour
 * @param mpi_tag
 * @param mpi_src
 * @param comm
 * @return
 */
int CAN_Receive_neighbour(neighbour *neighbour, int mpi_tag, int mpi_src, MPI_Comm comm );

/**
 * @brief CAN_Send_neighbour
 * @param neighbour
 * @param mpi_tag
 * @param mpi_destinataire
 * @param comm
 * @return
 */
int CAN_Send_neighbour(const neighbour *neighbour, int mpi_tag, int mpi_destinataire, MPI_Comm comm);

/**
 * @brief CAN_Send_neighbour_list
 * @param l
 * @param mpi_tag
 * @param mpi_destinataire
 * @param comm
 * @return
 */
int CAN_Send_neighbour_list(const list *l, int mpi_tag, int mpi_destinataire, MPI_Comm comm);


/**
 * Initiatise un voisin en position @x, @y, de taille @size,
 *  avec l'orientation @or, et accessible via @com_rank
 * TESTED
 */
void init_neighbour(neighbour *n, unsigned int x, unsigned int y, unsigned int size, unsigned int or, unsigned int com_rank);

/**
 * @brief Vérifie si la frontière est valide
 *  Une frontière est valide ssi sa taille est > 0
 * Utile quand on souhaite invalider une frontière
 */
int is_neighbour_valid(const neighbour *n);

/**
 * @brief  Vérifie si le voisin neighbour partage une frontière avec land.
 *          Cette frontière peut etre complete, partielle, incluse
 * @param land
 * @param n
 * @return
 * TODO test
 */
int is_neighbour(const land *land, const neighbour *n);

/**
 * @brief Retourne vrai une frontière en haut avec land possède un voisin neighbour sur sa frontière gauche.
 * @param land
 * @param n
 * @return
 * PARTIALLY TESTED
 */
bool is_neighbour_top(const land *land, const neighbour *n);

/**
 * @brief  * Retourne vrai une frontière en bas avec land possède un voisin neighbour sur sa frontière gauche.
 * @param land
 * @param n
 * @return
 * TESTED
 */
bool is_neighbour_bot(const land *land, const neighbour *n);


/**
 * @brief Retourne vrai une frontière a gauche avec land possède un voisin neighbour sur sa frontière gauche.
 * @param land
 * @param n
 * @return
 * TESTED
 */
bool is_neighbour_left(const land *land, const neighbour *n);

/**
 * @brief  Retourne vrai une frontière a droite avec land possède un voisin neighbour sur sa frontière gauche.
 * @param land
 * @param n
 * @return
 * TESTED
 */
bool is_neighbour_right(const land *land, const neighbour *n);


/**
 * @brief  Fonction de callback permettant d'afficher un neighbour
 *  utile dans @list_apply
 * @param n
 * TESTED
 */
void print_neighbour_cb(void *n);

/**
 * Fonction de callback permettant de désalouer un neighbour
 * utile dans @list_clear@brief free_neighbour_cb
 */
void free_neighbour_cb(void *);

/**
 *
 */
void free_land_cb(void *);

/**
 * @brief  A utiliser apèrs @split_land
 *         Permet d'extraire les deux frontières (@n1, @n2)
 *         résultantes d'un split qui a généré land1 et land2
 * @param land1
 * @param land2
 * @param n1
 * @param n2
 * TESTED
 */
void land_extract_neighbourg_after_split(land *land1, land *land2, neighbour *n1, neighbour *n2);


/**
 * @brief  Renormalise/Ajuste une frontière (@neigbour) pour correspondre aux vrais frontières du @land
 * @param land
 * @param n
 * @return
 * TESTED
 */
int adjust_neighbour(land *land, neighbour *n);

/**
 * @brief  * Affiche une frontière
 * @param n
 */
void print_neighbour(const neighbour *n);

/**
 * @brief Place dans @res le voisin qui va servir au prochain saut lors du routage
 * En calculant les distances euclidiennes entre pair et chaque milieu de segment de la liste
 * @param l
 * @param pair
 * @param res
 * @return
 */
int find_neighbour(const list *l, const pair *pair, neighbour *res );

/**
 * @brief Remplit le buffer avec les valeurs de la liste
 * @param l
 * @param buffer
 * TODO TEST
 */
void neighbour_to_buffer(const list *l, unsigned int buffer[MAX_SIZE_BUFFER]);

/**
 * @brief retourne la distance entre le milieu de neighbour et x,y
 * @param x1
 * @param y1
 * @param neighbour
 * @return
 */
double entire_dist_neigbourg(int x1, int y1, const neighbour *neighbour);

/**
 * @brief urecoit une nouvelle frontière (new_n) et met à jour la liste en focntione de ça et
 * @param list
 * @param land
 * @param new_n
 * @return
 */
int update_neighbours(list *list, const land *land, const neighbour *new_n);

//
/**
 * @brief Retire de chaque neigbour les zone qui se chauvechent entre n1 et n2
 * @param n1
 * @param n2
 * @return
 * TESTED
 */
int update_border(neighbour *n1, const neighbour *n2);

/**
 * @brief is_over_neighbour
 * @param n1
 * @param n2
 * @return
 */
int is_over_neighbour(const neighbour *n1, const neighbour *n2);

/**
 * @brief are_equals_neighbours
 * @param n1
 * @param n2
 * @return
 * TODO  TESTED
 */
int are_equals_neighbours(const neighbour*n1, const neighbour *n2);

/**
 * @brief are_over_neighbour
 * @param n1
 * @param n2
 * @return
 */
int are_over_neighbour(const neighbour *n1, const neighbour *n2);


/**
 * @brief is_contains_neighbour retourne vrai si la frontière n2 est strictement inclue dans n1
 * @param n1
 * @param n2
 * @return
 * n1 x--------------x
 * n2    x-------x
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
 * @brief is_land_contains  return true if the land cotains the point defined by x and y is
 * @param l
 * @param x
 * @param y
 * @return
 * TESTED
 */
bool is_land_contains(const land *l, unsigned  int x, unsigned  int y);

/**
 * @brief is_land_contains_pair  return true if the land contains the pair
 * @param l
 * @param p
 * @return
 * TESTED
 */
bool is_land_contains_pair(const land *l, const pair *p);

/**
 * @brief split_land split old_land and init new_land initialized to the half of old_land
 * @param new_land
 * @param old_land
 * TESTED
 */
void split_land(land *new_land, land *old_land);

/**
 * @brief split_land_update_neighbour
 * @param new_land
 * @param old_land
 * @param new_n
 * @param old_n
 * TODO TEST
 */
void split_land_update_neighbour(land *new_land, land *old_land, list *new_n, list *old_n, int, int);

/**
 * @brief initialiseur d'un land
 * @param l
 * @param x
 * @param y
 * @param s_x
 * @param s_y
 * TESTED
 */
void init_land(land *l, unsigned  int x, unsigned  int y, unsigned int s_x, unsigned int s_y);

/**
 * @brief free_land destructeur d'un land
 * @param l
 */
void free_land(land *l);

/**
 * @brief print_land Affichage d'un objet land
 * @param l
 */
void print_land(const land *l);

/**
 * @brief log_factory  Journalise dans un fichier de log
 * @param f
 * @param data
 * @param CODE
 * @param from
 */
void log_factory(FILE *f, const void *data, int CODE, int from);

/**
 *
 */
void create_svg_logs(const char* path,const int size_x, const int size_y ,const list *list_lands);

#endif
