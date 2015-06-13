#ifndef H_CAN_COMMUNICATION
#define H_CAN_COMMUNICATION

#include "cartesian_space.h"
#include "utils.h"

/**
 * The macro for the communication
 */
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
#define RES_INIT_DATA        57
#define UPDATE_NEIGBOUR      789
#define ATTACH_NEW_DATA      888
#define RM_DATA              4242
#define FETCH_DATA           999
#define SEND_FETCH_DATA      111
#define SEND_DATA_ORDER      222

#define DATA_INT 0


// world size
#define SIZE_X 1000
#define SIZE_Y 1000

//root process
#define ROOT_PROCESS 0

/**
  *
  */
typedef struct _can_node {
	list voisins, data_storage;
	land land_id;
	pair pair_id;
} can_node;

/**
 * @brief init_can_node function to init a CAN node
 * @param node
 */
void init_can_node(can_node *node);

/**
 * @brief CAN_Send_data_update transform the list to a buffer and send it with mpi routine
 * @param list
 * @param mpi_tag
 * @param mpi_destinataire
 * @param comm
 * @return
 */
int CAN_Send_data_update(const list *list, int mpi_tag, int mpi_destinataire, MPI_Comm comm);

/**
 * \deprecated { description } this function is not used
 * @brief CAN_Recv_localise
 * @param loc
 * @param _pair
 * @param self_rank
 * @param first_node
 * @param comm
 */
void CAN_Recv_localise(int *loc, const pair *_pair, int self_rank, int first_node, MPI_Comm comm);

/**
 * @brief CAN_Recv_localise_timeout function to find a node on the orverlay, not used anymore but it is implementing an interesting system of timeout with iprobe
 * @param loc
 * @param pair
 * @param self_rank
 * @param first_node
 * @param comm
 * @param timeout
 */
void CAN_Recv_localise_timeout(int *loc, const pair *pair, int self_rank, int first_node, MPI_Comm comm, unsigned int timeout);

/**
 * @brief CAN_Fetch_data Fonction to Fetch data from the overlay to the postion x and y
 * @param comm
 * @param com_rank
 * @param first_rank
 * @param pair
 * @param elem
 */
void CAN_Fetch_data(MPI_Comm comm, int com_rank, int first_rank ,const pair *pair, can_data * elem);


/**
 * @brief CAN_Root_Process_Job Main job of the root process
 * @param root_rank
 * @param comm
 * @param nb_proc
 * @return
 */
int CAN_Root_Process_Job(int root_rank, MPI_Comm comm,int nb_proc);

/**
 * @brief CAN_Root_Process_Job_Insert_One The root process ask a proc to begin the insertion routine
 * @param root_rank
 * @param comm
 * @param proc_to_insert
 * @return
 */
int CAN_Root_Process_Job_Insert_One(int root_rank, MPI_Comm comm, int proc_to_insert);

/**
 * @brief CAN_Node_Job Main job of a CAN node
 * @param com_rank
 * @param comm
 * @return
 */
int CAN_Node_Job(int com_rank, MPI_Comm comm);

/**
 * @brief CAN_Attach_new_data Put a data on the overlay to the position x , y.
 * @param self_rank the process who asked
 * @param first_node the first node to join
 * @param comm the MPI_Comm
 * @param _pair the destination
 * @param data a pointer to the data
 * @param data_type the type of the data (it will be communicate with a fecth) the user will be able to "recreate the data from the type",itis for user purpose only
 * @param data_size the size of the data (it allow to put array of data type)
 */
void CAN_Attach_new_data(int self_rank, int first_node, MPI_Comm comm, pair *_pair, void *data, int data_type, unsigned int data_size);

/**
 * @brief CAN_Remove_data remove the data to the specified postion (if exist)
 * @param self_rank
 * @param first_node
 * @param comm
 * @param p
 * @return
 */
int CAN_Remove_data(int self_rank, int first_node, MPI_Comm comm, pair * p);

/**
 * @brief prompt prompt for the root node allow user to manualy controle the overlay from the root node
 * @param root_rank
 * @param comm
 * @param nb_proc
 */
void prompt(int root_rank, MPI_Comm comm, int nb_proc);

/**
 * @brief chunk_to_buffer utility function to convert a list of @chunk to buffer ready to be send by mpi routines
 *  only the postion will be send, used for logs
 * @param ch
 * @param buffer
 */
void chunk_to_buffer(const list* ch, unsigned int buffer[MAX_SIZE_BUFFER]);

/**
 * @brief chunks_to_buffer utility function to convert a list of @chunk to buffer ready to be send by mpi routines
 * [nb of chunk | x | y | type | size | data ]
 * the full data will be send, used to retrive the data by @CAN_Fetch_data();
 * @param list
 * @param buffer
 * @param char_size
 */
void chunks_to_buffer(const list *list, char buffer[MAX_SIZE_BUFFER_CHAR],unsigned int *char_size);


/**
 * @brief buffer_to_chunk convert a buffer of  chunk to a list of chunk
 * @param list
 * @param buffer
 */
void buffer_to_chunk(list *list, const char buffer[MAX_SIZE_BUFFER_CHAR]);

/**
 * @brief distribute_data_after_split the list @new list will be fullfill with the data it must own, old list will be lost the data it should not own
 * @param land
 * @param old_list
 * @param new_list
 */
void distribute_data_after_split(const land *land, list *old_list, list *new_list);

/**
 * @brief DHT_put insertion d'un int dans la dht
 * @param root_rank
 * @param comm
 * @param x
 * @param y
 * @param data
 * @return
 */
int DHT_put(int root_rank, MPI_Comm comm, unsigned int x, unsigned int y, int data);

/**
 * @brief DHT_rm suprresion d'une donnée dans la DHT
 * @param root_rank
 * @param comm
 * @param x
 * @param y
 * @return
 */
int DHT_rm(int root_rank, MPI_Comm comm, unsigned int x, unsigned int y);

/**
 * @brief DHT_get Récupère une données depuis l'overlay (displayed when got)
 * @param comm
 * @param root_rank
 * @param x
 * @param y
 * @return
 */
int DHT_get(MPI_Comm comm, int root_rank, int x, int y);



/**
 * @brief CAN_shuffle_insert insert shuffle @nb_elem data
 * @param data_type
 * @param nb_elem
 * @param self_rank
 * @param comm
 */
CAN_shuffle_insert(int data_type, int nb_elem, int self_rank, MPI_Comm comm);

#endif
