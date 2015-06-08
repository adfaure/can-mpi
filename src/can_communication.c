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


int CAN_Root_Process_Job(int root_rank, MPI_Comm comm, int nb_proc) {

    MPI_Status main_loop_status;
    unsigned int land_buffer[4] , buffer_ui[MAX_SIZE_BUFFER], buffer_simple_int = 0;
    int main_loop_tag, count, main_loop_buffer_int[MAX_SIZE_BUFFER]; // wait_array on attend un message d'une source avec un tag
    land temp_land;
    list voisins, temp_voisins, lands;
    FILE *file;
    neighbour temp_voisin;

    file = fopen ("log.txt", "w+");
    if(!file) {
        printf(" erreur lors de l'ouverture du fichier de log \n");
        return 0;
    }
    init_list(&voisins, sizeof(neighbour));
    init_list(&temp_voisins, sizeof(neighbour));
    init_list(&lands, sizeof(land));

    for(int i = 0; i < nb_proc; i++) {
        if(i != root_rank) {
            MPI_Send(&i, 1, MPI_INT, i, ROOT_TAG_INIT_NODE, comm);
            MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, i, MPI_ANY_TAG, comm, &main_loop_status);
            main_loop_tag  = main_loop_status.MPI_TAG;
            if(main_loop_tag == ACK_TAG_BOOTSTRAP) {
                printf("bootstraping node did it well \n");
            } else if(main_loop_tag == GET_ENTRY_POINT) {
                main_loop_buffer_int[0] = 1;
                MPI_Send(&main_loop_buffer_int[0] ,1 , MPI_INT, i, SEND_ENTRY_POINT, comm);
            }
        }
    }

    int dummy = 0;
    for(int j = 0; j < 10000000; ++j) {
        dummy = j * j;
    }
    dummy++;

    for(int i = 0; i < nb_proc; i++) {
        if(i != root_rank) {
            MPI_Send(&buffer_simple_int ,1 , MPI_INT, i, SEND_LAND_ORDER, comm);
            MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, i, ACK, comm, &main_loop_status);
            init_land(&temp_land, land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
            log_factory(file, &temp_land, LAND_LOG, i);
            list_add_front(&lands , &temp_land);
            MPI_Send(&buffer_simple_int ,1 , MPI_INT, i, SEND_NEIGBOUR_ORDER, comm);
            MPI_Probe(i, ACK, comm, &main_loop_status);
            MPI_Get_count (&main_loop_status, MPI_UNSIGNED, &count);
            MPI_Recv(&buffer_ui[0] ,count, MPI_UNSIGNED, i, ACK, comm, &main_loop_status);
            int nb_voisins = count / (sizeof(neighbour) / sizeof(unsigned int));
            int idx = 0;
            for(int j = 0; j < nb_voisins; j++) {
                init_neighbour(&temp_voisin, buffer_ui[idx], buffer_ui[idx+1], buffer_ui[idx+2],buffer_ui[idx+3], buffer_ui[idx+4]);
                list_add_front(&voisins, &temp_voisin);
                log_factory(file, &temp_voisin, NEIGHBOUR_LOG, i);
                idx += 5;
            }
        }
    }
    create_svg_logs("world.svg",SIZE_X, SIZE_Y ,&lands);
    fclose(file);
    return 1;
}

int CAN_Root_init(MPI_Status *req_status, MPI_Comm comm, int com_rank, land *land_id) {
	int count,send_int_buffer[MAX_SIZE_BUFFER] ,rec_buffer_int[MAX_SIZE_BUFFER], req_src;
	pair pair_id;

    get_random_id(&pair_id, SIZE_X, SIZE_Y);
    MPI_Get_count (req_status, MPI_INT, &count);
    MPI_Recv(&(rec_buffer_int[0]) ,count , MPI_INT, req_src,
            req_status->MPI_TAG , comm, MPI_STATUS_IGNORE);

    if(rec_buffer_int[0] == 1) {
        init_land(land_id, 0, 0, SIZE_X, SIZE_Y);
        MPI_Send((&rec_buffer_int[0]) ,1 ,MPI_INT ,ROOT_PROCESS ,ACK_TAG_BOOTSTRAP ,comm);
    } else {
        rec_buffer_int[0] = com_rank;
        MPI_Send((&rec_buffer_int[0]) ,1 , MPI_INT ,ROOT_PROCESS ,GET_ENTRY_POINT ,comm);
        MPI_Recv((&rec_buffer_int[0]) ,1 , MPI_INT, ROOT_PROCESS, SEND_ENTRY_POINT, comm, req_status);
        send_int_buffer[0] = com_rank; send_int_buffer[1] = pair_id.x; send_int_buffer[2] = pair_id.y;
        MPI_Send((&send_int_buffer[0]), 3, MPI_INT, rec_buffer_int[0], REQUEST_TO_JOIN, comm);
    }
    return 1;
}

void CAN_Rec_Neighbours(MPI_Status *req_status, MPI_Comm comm, int com_rank, land *land_id, list *voisins) {
    int nb_voisins, idx = 0, count;
    unsigned int buffer_ui[MAX_SIZE_BUFFER];
    MPI_Status status;
    neighbour temp_voisin;

    MPI_Get_count (req_status, MPI_UNSIGNED, &count);
    MPI_Recv(&buffer_ui[0] ,count, MPI_UNSIGNED, req_status->MPI_SOURCE , req_status->MPI_TAG ,comm, &status);
    nb_voisins = count / (sizeof(neighbour) / sizeof(unsigned int));
    for(int i = 0; i < nb_voisins; i++) {
        init_neighbour(&temp_voisin, buffer_ui[idx], buffer_ui[idx+1], buffer_ui[idx+2],buffer_ui[idx+3], buffer_ui[idx+4]);
        idx += 5;
        list_add_front(voisins, &temp_voisin);
        if(req_status->MPI_SOURCE == temp_voisin.com_rank ) {
            continue;
        }
        CAN_Send_neighbour(&temp_voisin,UPDATE_NEIGBOUR, temp_voisin.com_rank, comm);
    }
    printf("[ %d ] voisins recu \n", com_rank);
    list_apply(voisins, print_neighbour_cb);
    printf("\n");
}

// handle  a Join REQUEST return 1 if wait for is changed (maybe not a good convention)
int CAN_Request_to_join(MPI_Status *req_status,const MPI_Comm comm,const int com_rank,const land *land_id, const list *list_neighbours,int *wait_for) {
    // Request de localisation, le noeud contenant la paire transporté répondra au noeud de la requete RES_REQUEST_TO_JOIN
	int buffer_int[MAX_SIZE_BUFFER];
	int from = req_status->MPI_SOURCE, tag = req_status->MPI_TAG;
	pair rec_pair;
	neighbour neighbour_temp_find;
    MPI_Recv(&buffer_int[0] ,3, MPI_INT, from, tag, comm, req_status);
    init_pair(&rec_pair, buffer_int[1], buffer_int[2]);
    if(is_land_contains_pair(land_id, &rec_pair)) {
        MPI_Send(&com_rank, 1, MPI_INT, buffer_int[0], RES_REQUEST_TO_JOIN,  comm);
        (*wait_for) = buffer_int[0];
        return 1;
    } else {
        if(find_neighbour(list_neighbours, &rec_pair, &neighbour_temp_find)) {
            print_pair(&rec_pair);
            MPI_Send(&buffer_int[0], 3, MPI_INT, neighbour_temp_find.com_rank, REQUEST_TO_JOIN,  comm);
        } else {
            printf("ERROR lors de la recherche de voisins \n");
        }
    }
	return 0;
}

void CAN_Send_Land_order(MPI_Status *req_status,const MPI_Comm comm, const land *land_id) {
    int buffer_simple_int, count;
    unsigned int land_buffer[MAX_SIZE_BUFFER];
	MPI_Get_count (req_status, MPI_INT, &count);
    MPI_Recv(&buffer_simple_int , count , MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, MPI_STATUS_IGNORE);
    if(req_status->MPI_SOURCE == ROOT_PROCESS ) {
        printf("recu SEND_LAND_ORDER \n");
        land_buffer[0] = land_id->x; land_buffer[1] = land_id->y; land_buffer[2] = land_id->size_x; land_buffer[3] = land_id->size_y;
        MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, req_status->MPI_SOURCE, ACK, MPI_COMM_WORLD);
    }
}

void CAN_Update_Neighbours(MPI_Status *req_status,const MPI_Comm comm,const int com_rank ,const land *land_id, list *voisins) {
	neighbour temp_voisin;
    CAN_Receive_neighbour(&temp_voisin, req_status->MPI_TAG, req_status->MPI_SOURCE,comm);
    printf("[ %d ] Mes amis je suis heureux de vous annoncer que nous acceuilons à présent un nouveau voisins [%d]! \n ", com_rank, req_status->MPI_SOURCE);
    temp_voisin.com_rank = req_status->MPI_SOURCE;
    update_neighbours(voisins, land_id ,&temp_voisin);
    printf("[ %d ] voisins recu \n", com_rank);
    print_neighbour(&temp_voisin);
    printf("\n");
    list_apply(voisins, print_neighbour_cb);
    printf("\n");
}

int CAN_Node_Job(int com_rank, MPI_Comm comm) {
    int corresp;
    unsigned int land_buffer[4] , buffer_ui[MAX_SIZE_BUFFER], buffer_simple_int = 0;
    int wait_for = -1;
    unsigned int main_loop_from;
    int main_loop_tag, count, main_loop_buffer_int[MAX_SIZE_BUFFER] ; // wait_array on attend un message d'une source avec un tag
    bool bootstrap = false;
    list voisins, temp_voisins;
    neighbour neighbour_temp_find, temp_voisin;
    pair  pair_join_request;
    land land_id, new_land;
    MPI_Status main_loop_status;
    init_list(&voisins, sizeof(neighbour));
    init_list(&temp_voisins, sizeof(neighbour));
    while(1) {
        list_clear(&temp_voisins, free_neighbour_cb);
        if(wait_for != -1) {
            printf("je suis [%d] en attente d'un message de %d \n", com_rank, wait_for);
        }
        MPI_Probe(wait_for, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
        main_loop_from = main_loop_status.MPI_SOURCE;
        main_loop_tag  = main_loop_status.MPI_TAG;

        if(main_loop_tag == ROOT_TAG_INIT_NODE) {
        	CAN_Root_init(&main_loop_status, comm, com_rank, &land_id);
        }
        else if(main_loop_tag == REQUEST_TO_JOIN) {
        	CAN_Request_to_join(&main_loop_status,comm,com_rank, &land_id, &voisins ,&wait_for);
        }
        else if(main_loop_tag == SEND_LAND_ORDER) {
        	CAN_Send_Land_order(&main_loop_status , comm, &land_id);
        }
        else if(main_loop_tag == RES_INIT_NEIGHBOUR) {
        	CAN_Rec_Neighbours(&main_loop_status , comm, com_rank, &land_id, &voisins);
        }

        else if(main_loop_tag == UPDATE_NEIGBOUR) {
        	CAN_Update_Neighbours(&main_loop_status  , comm, com_rank , &land_id, &voisins);
        }

        else if(main_loop_tag == RES_REQUEST_TO_JOIN) {
            MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, main_loop_from, main_loop_tag, comm, &main_loop_status);
            MPI_Send(&(main_loop_buffer_int[0]),1 , MPI_INT , main_loop_from , REQUEST_INIT_SPLIT,comm);
            MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, main_loop_from, REQUEST_RECEIVE_LAND, comm, &main_loop_status);
            init_land(&land_id, land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
            print_land(&land_id);
        }

        else if(main_loop_tag == SEND_NEIGBOUR_ORDER) {
            neighbour_to_buffer(&voisins, buffer_ui);
            MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, main_loop_from, main_loop_tag, comm, &main_loop_status);
            MPI_Send((&buffer_ui[0]), voisins.nb_elem * (sizeof(neighbour)/sizeof(unsigned int)), MPI_UNSIGNED, main_loop_from, ACK, comm);
        }

        else if(main_loop_tag == REQUEST_INIT_SPLIT) {
            MPI_Get_count (&main_loop_status, MPI_INT, &count);
            MPI_Recv(&main_loop_buffer_int[0] ,count, MPI_INT, main_loop_from, main_loop_tag, comm, &main_loop_status);
            split_land_update_neighbour(&new_land, &land_id, &temp_voisins, &voisins, main_loop_from , com_rank);
            land_buffer[0] = new_land.x; land_buffer[1] = new_land.y;land_buffer[2] = new_land.size_x;land_buffer[3] = new_land.size_y;
            MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, main_loop_from, REQUEST_RECEIVE_LAND, comm);
            CAN_Send_neighbour_list(&temp_voisins,RES_INIT_NEIGHBOUR , main_loop_from, comm);
            list_clear(&temp_voisins, free_neighbour_cb);
            printf("je suis %d , transaction fini avec %d \n",com_rank, wait_for);
            wait_for = -1;
        }

        else if(main_loop_tag == LOCALIZE) {
            corresp = main_loop_buffer_int[0];
            init_pair(&pair_join_request,  main_loop_buffer_int[1], main_loop_buffer_int[2]);
            if(is_land_contains_pair(&land_id, &pair_join_request)) {
                MPI_Send(&com_rank, 1, MPI_INT, corresp, LOCALIZE_RESP, comm);
            } else {
            }
        }

        else   {
            printf("unknow tag \n");
        }
    }
    return 1;
}
