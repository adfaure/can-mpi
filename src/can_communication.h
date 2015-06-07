#ifndef H_CAN_COMMUNICATION
#define H_CAN_COMMUNICATION

#include "cartesian_space.h"
#include "utils.h"

/**
 *
 */
void CAN_Recv_localise(int *loc, const pair *_pair, int self_rank, int first_node, MPI_Comm comm);

/**
 *
 */
void CAN_Recv_localise_timeout(int *loc, const pair *pair, int self_rank, int first_node, MPI_Comm comm, unsigned int timeout);

/**
 * job for the root process
 */
int CAN_Root_Process_Job(int root_rank, MPI_Comm comm,int nb_proc);

/**
 * job nodes
 */
int CAN_Node_Job(int com_rank, MPI_Comm comm);


#endif
