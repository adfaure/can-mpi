#include <mpi.h>
#include <stdio.h>

typedef void (*CAN_Callback)(void* data);

void callback_test_no_args(void *dummy) {
  printf("called \n");
}

void callback_test_string(void *data) {
  printf("called %d\n", *((int*)data));
}

int main(int argc, char**argv) {
  MPI_Init (&argc, &argv);
  int nb_proc, com_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);
  int i = 4;
  CAN_Callback cb[2];
  cb[0] = &callback_test_no_args;
  cb[1] = &callback_test_string;

  cb[0](NULL);
  cb[1](&i);

  MPI_Finalize();
  return 0;
}
