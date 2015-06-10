#include "mpi_can.h"
#include "can_communication.h"

int main(int argc, char**argv) {
    int com_rank, nb_proc;
    MPI_Init (&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);
    srand(time(NULL) * com_rank * nb_proc);

    if(com_rank == ROOT_PROCESS) {
        prompt(ROOT_PROCESS, MPI_COMM_WORLD, nb_proc);
    } else {
        CAN_Node_Job(com_rank, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
