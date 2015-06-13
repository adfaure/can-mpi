#ifndef H_CAN_COMMUNICATION
#define H_CAN_COMMUNICATION

#include "cartesian_space.h"
#include "utils.h"

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
#define RES_INIT_DATA        57
#define UPDATE_NEIGBOUR      789
#define ATTACH_NEW_DATA      888
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
 *
 */
void init_can_node(can_node *node);

/**
 *
 */
void CAN_Recv_localise(int *loc, const pair *_pair, int self_rank, int first_node, MPI_Comm comm);


/**
 *
 */
void CAN_Fetch_data(MPI_Comm comm, int com_rank, int first_rank ,const pair *pair, can_data * elem);

/**
 *
 */
void CAN_Recv_localise_timeout(int *loc, const pair *pair, int self_rank, int first_node, MPI_Comm comm, unsigned int timeout);

/**
 * job for the root process
 */
int CAN_Root_Process_Job(int root_rank, MPI_Comm comm,int nb_proc);

/**
 *
 */
int CAN_Root_Process_Job_Insert_One(int root_rank, MPI_Comm comm, int proc_to_insert);

/**
 * job nodes
 */
int CAN_Node_Job(int com_rank, MPI_Comm comm);

/**
 *
 */
void CAN_Attach_new_data(int self_rank, int first_node, MPI_Comm comm, pair *_pair, void *data, int data_type, unsigned int data_size);

/**
 *
 */
void prompt(int root_rank, MPI_Comm comm, int nb_proc);

/**
 *
 */
void chunk_to_buffer(const list* ch, unsigned int buffer[MAX_SIZE_BUFFER]);

/**
 *
 */
void chunks_to_buffer(const list *list, char buffer[MAX_SIZE_BUFFER_CHAR],unsigned int *char_size);


/**
 *
 */
void buffer_to_chunk(list *list, const char buffer[MAX_SIZE_BUFFER_CHAR]);

/**
 *
 */
void distribute_data_after_split(const land *land, list *old_list, list *new_list);

/**
 *
 */
int DHT_put(int root_rank, MPI_Comm comm, unsigned int x, unsigned int y, int data);

/**
 *
 */
int DHT_get(MPI_Comm comm, int root_rank, int x, int y);

/**
 *
 */
CAN_shuffle_insert(int data_type, int nb_elem, int self_rank, MPI_Comm comm);

#endif
