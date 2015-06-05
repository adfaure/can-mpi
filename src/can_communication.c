#include "can_communication.h"

void CAN_Recv_localise(int *loc, const pair *_pair,  int self_rank,  int first_node, MPI_Comm comm) {
  MPI_Status status;
  int *buffer = (int*) malloc(sizeof(int) * 3);
  buffer[0] = self_rank;
  buffer[1] = _pair->x;
  buffer[2] = _pair->y;
  MPI_Send(&(buffer[0]), 3,  MPI_INT,  first_node,  LOCALIZE,  comm);
  MPI_Recv(loc, 1,  MPI_UNSIGNED,  MPI_ANY_SOURCE,  LOCALIZE_RESP,  comm,  &status);
  free(buffer);
}

// int MPI_Iprobe(int source,  int tag,  MPI_Comm comm,  int *flag,
    // MPI_Status *status)
void CAN_Recv_localise_timeout(int *loc, const pair *_pair,  int self_rank,  int first_node, MPI_Comm comm, unsigned int timeout) {
  int *buffer = (int*) malloc(sizeof(int) * 3),  flag;
  MPI_Status status;
  unsigned long begin_time = 0,  time_elapsed = 0;
  buffer[0] = self_rank;
  buffer[1] = _pair->x;
  buffer[2] = _pair->y;
  MPI_Send(&(buffer[0]),  3,  MPI_INT,  first_node,  LOCALIZE,  comm);
  begin_time = now();
  while(time_elapsed < timeout) {
    MPI_Iprobe(MPI_ANY_SOURCE,  MPI_ANY_TAG,  comm,  &flag,  &status);
    if(flag) {
      MPI_Recv(loc,  1,  MPI_INT,  MPI_ANY_SOURCE,  LOCALIZE_RESP,  comm,  &status);
      return;
    }
    time_elapsed = now() - begin_time;
  }
  *loc = -1;
  free(buffer);
}
