#ifndef H_CAN_COMMUNICATION
#define H_CAN_COMMUNICATION

#include "utils.h"

/**
 *
 */
void CAN_Recv_localise(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm);

/**
 *
 */
void CAN_Recv_localise_timeout(int *loc ,const pair *pair, int self_rank , int first_node ,MPI_Comm comm, long long timeout);

#endif
