#include "mpi_can.h"
#include "can_communication.h"

int main(int argc, char**argv) {
    const char str_debug[] = "debug";
    const char str_seed[]  = "seed";

    int com_rank, nb_proc;
    MPI_Init (&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);

    int seed;
    if (argc >= 3 && (strcmp(str_seed, argv[1]) == 0)) {
        seed = atoi(argv[2]);
    } else {
        seed = time(NULL);
    }
    seed = seed  * (com_rank + 1) * nb_proc;
    srand(seed);
    printf("[ %d ]random seed = %d \n",com_rank, seed);
    fflush(stdout);

    if(com_rank == ROOT_PROCESS) {
        if (
            (argc == 2 && (strcmp(str_debug, argv[1]) == 0)) ||
            (argc == 4 && (strcmp(str_debug, argv[3]) == 0))
        ) {
            prompt(ROOT_PROCESS, MPI_COMM_WORLD, nb_proc);
        } else {
            CAN_Root_Process_Job(ROOT_PROCESS, MPI_COMM_WORLD, nb_proc);
            printf("insert ok \n");
        }
    } else {
        CAN_Node_Job(com_rank, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
