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



void CAN_Log_informations(MPI_Comm comm,const int root_rank, const int nb_proc, const char *base_path) {
    MPI_Status main_loop_status;
    unsigned int land_buffer[4] , buffer_ui[MAX_SIZE_BUFFER], buffer_simple_int = 0;
    int count; // wait_array on attend un message d'une source avec un tag
    land temp_land;
    list  temp_voisins, lands;
    neighbour temp_voisin;
    FILE * file;

    const char svg[4] = ".svg";
    const char txt[4] = ".txt";
    int size = strlen(base_path);
    char *txt_file      = malloc((size * sizeof(char)) + ( 4 * sizeof(char)) + 1);
	char *svg_name_file = malloc((size * sizeof(char)) + ( 4 * sizeof(char)) + 1);
	txt_file[size] = '\n';
	svg_name_file[size] = '\n';

	strcpy(txt_file, base_path);
	strcat(txt_file, txt);

	strcpy(svg_name_file, base_path);
	strcat(svg_name_file, svg);

    file = fopen (txt_file, "w+");
    if(!file) {
        printf(" erreur lors de l'ouverture du fichier de log \n");
        return;
    }

    init_list(&temp_voisins, sizeof(neighbour));
    init_list(&lands, sizeof(land));

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
                list_add_front(&temp_voisins, &temp_voisin);
                log_factory(file, &temp_voisin, NEIGHBOUR_LOG, i);
                idx += 5;
            }
        }
    }
    create_svg_logs(svg_name_file , SIZE_X, SIZE_Y ,&lands);
    list_clear(&temp_voisins, free_neighbour_cb);
    list_clear(&lands,  free_land_cb);
    free(svg_name_file);
    free(txt_file);
    fclose(file);
}


int CAN_Root_Process_Job(int root_rank, MPI_Comm comm, int nb_proc) {

    MPI_Status main_loop_status;
    int main_loop_tag, main_loop_buffer_int[MAX_SIZE_BUFFER]; // wait_array on attend un message d'une source avec un tag
    const char* base_path = "logs/log_in_process";
    const char* name[MAX_SIZE_BUFFER];
    list voisins, temp_voisins, lands;

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
                MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, i, ACK, comm, &main_loop_status);
                sprintf((char *)name, "%s_%d", base_path, i);
                CAN_Log_informations(comm,root_rank, nb_proc, (char *)name);
            }
        }
    }
    int data = 42;
    pair test;
    init_pair(&test, 550, 300);
    CAN_Attach_new_data(root_rank, 1, comm, &test, &data, DATA_INT , sizeof(int));

    CAN_Log_informations(comm, root_rank, nb_proc, "logs/end_log");
    printf("ROOT PROCESS DONE\n");
    return 1;
}



void CAN_Attach_new_data(int self_rank, int first_node, MPI_Comm comm, pair *_pair, void *data,int data_type, unsigned int data_size) {
    MPI_Status status;
    unsigned int total_size = 0;
    char *buffer = (char*) malloc((sizeof(int) * 4) + (sizeof(char) * data_size )), res;
    memcpy(&buffer[0]                       , &(self_rank) , (sizeof(unsigned int))); //from
    memcpy(&buffer[1 * sizeof(unsigned int)], &(_pair->x)  , (sizeof(unsigned int))); // x
    memcpy(&buffer[2 * sizeof(unsigned int)], &(_pair->y)  , (sizeof(unsigned int))); // y
    memcpy(&buffer[3 * sizeof(unsigned int)], &(data_size) , (sizeof(unsigned int))); // data size
    memcpy(&buffer[4 * sizeof(unsigned int)], &(data_type) , (sizeof(unsigned int))); // data type

    total_size = (5 * (sizeof(unsigned int))) + data_size;
    memcpy(&(buffer[5 * (sizeof(unsigned int))]), data, data_size);

    MPI_Send(&(buffer[0]), total_size,  MPI_CHAR,  first_node,  ATTACH_NEW_DATA,  comm);
    MPI_Recv(&res, 1,  MPI_INT,  MPI_ANY_SOURCE,  ACK,  comm,  &status);
    free(buffer);
}


void CAN_REQ_Attach_new_data(MPI_Status *req_status, MPI_Comm comm , const int com_rank ,const land *land_id, const  list *voisins) {
	int buffer_int[MAX_SIZE_BUFFER], count, useless = 0;
	char buffer_char[MAX_SIZE_BUFFER];
	int from = req_status->MPI_SOURCE, tag = req_status->MPI_TAG;
	pair rec_pair;
	neighbour neighbour_temp_find;
	void * data;
	can_data temp_data;
    MPI_Get_count (req_status, MPI_CHAR, &count);
    MPI_Recv(&buffer_char[0] ,count, MPI_CHAR, from, tag, comm, req_status);

    memcpy(&buffer_int[0], &buffer_char[0],                        sizeof(unsigned int)); // from
    memcpy(&buffer_int[1], &buffer_char[1 * sizeof(unsigned int)], sizeof(unsigned int)); // x
    memcpy(&buffer_int[2], &buffer_char[2 * sizeof(unsigned int)], sizeof(unsigned int)); // y
    memcpy(&buffer_int[3], &buffer_char[3 * sizeof(unsigned int)], sizeof(unsigned int)); // data size
    memcpy(&buffer_int[4], &buffer_char[4 * sizeof(unsigned int)], sizeof(unsigned int)); // data type

    init_pair(&rec_pair, buffer_int[1], buffer_int[2]);
    if(is_land_contains_pair(land_id, &rec_pair)) {
    	printf("data size : %d \n" ,buffer_int[3]);
    	data = malloc(sizeof(buffer_int[3]));
    	init_data(&temp_data, buffer_int[3],  buffer_int[4], &buffer_char[5* sizeof(unsigned int)]);
    	printf("[ %d ] i'll store it, no worries :  %d \n", com_rank, *((int*)&buffer_char[5* sizeof(unsigned int)]));
    	MPI_Send(&useless, 1, MPI_INT, buffer_int[0], ACK, comm);
    	free(data);
        return;
    } else {
        if(find_neighbour(voisins, &rec_pair, &neighbour_temp_find)) {
        	MPI_Send(&buffer_char[0], count, MPI_CHAR, neighbour_temp_find.com_rank, tag,  comm);
        	printf("[ %d ] --> [ %d ] ", com_rank, neighbour_temp_find.com_rank);
        	print_pair(&rec_pair);
        } else {
            printf("ERROR lors de la recherche de voisins \n");
        }
    }
}

int CAN_REQ_Root_init(MPI_Status *req_status, MPI_Comm comm, int com_rank, land *land_id) {
	int count,send_int_buffer[MAX_SIZE_BUFFER] ,rec_buffer_int[MAX_SIZE_BUFFER];
	pair pair_id;

    get_random_id(&pair_id, SIZE_X, SIZE_Y);
    MPI_Get_count (req_status, MPI_INT, &count);
    MPI_Recv(&(rec_buffer_int[0]) ,count , MPI_INT, req_status->MPI_SOURCE,
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

void CAN_REQ_Rec_Neighbours(MPI_Status *req_status, MPI_Comm comm, int com_rank, land *land_id, list *voisins, int *wait_for) {
    int nb_voisins, idx = 0, count, dummy = 0;
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
        if((unsigned int)req_status->MPI_SOURCE == temp_voisin.com_rank ) {
            continue;
        }
        CAN_Send_neighbour(&temp_voisin,UPDATE_NEIGBOUR, temp_voisin.com_rank, comm);
    }
    printf("[ %d ] voisins recu \n", com_rank);
    list_apply(voisins, print_neighbour_cb);
    printf("\n");
    MPI_Send((&dummy) ,1 ,MPI_INT ,ROOT_PROCESS ,ACK ,comm);
    *wait_for = MPI_ANY_SOURCE;
}

// handle  a Join REQUEST return 1 if wait for is changed (maybe not a good convention)
int CAN_REQ_Request_to_join(MPI_Status *req_status,const MPI_Comm comm,const int com_rank,const land *land_id, const list *list_neighbours,int *wait_for) {
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
            MPI_Send(&buffer_int[0], 3, MPI_INT, neighbour_temp_find.com_rank, REQUEST_TO_JOIN,  comm);
       // 	printf("[ %d ] --> [ %d ] ", com_rank, neighbour_temp_find.com_rank);
       //      print_pair(&rec_pair);
        } else {
            printf("ERROR lors de la recherche de voisins \n");
        }
    }
	return 0;
}

void CAN_REQ_Send_Land_order(MPI_Status *req_status,const MPI_Comm comm, const land *land_id) {
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

void CAN_REQ_Update_Neighbours(MPI_Status *req_status,const MPI_Comm comm,const int com_rank ,const land *land_id, list *voisins) {
	neighbour temp_voisin;
    CAN_Receive_neighbour(&temp_voisin, req_status->MPI_TAG, req_status->MPI_SOURCE,comm);
    printf("[ %d ] Mes amis je suis heureux de vous annoncer que nous acceuilons à présent un nouveau voisins [%d]! \n ", com_rank, req_status->MPI_SOURCE);
    temp_voisin.com_rank = req_status->MPI_SOURCE;
    update_neighbours(voisins, land_id ,&temp_voisin);
    printf("[ %d ] voisins recu \n", com_rank);
    print_neighbour(&temp_voisin);
    printf("\n");
    list_apply(voisins, print_neighbour_cb);
    printf("\n");
}

void CAN_REQ_Res_Request_to_join(MPI_Status *req_status,const MPI_Comm comm,const int com_rank , land *land_id, list *voisins, int *wait_for) {
	int main_loop_buffer_int[MAX_SIZE_BUFFER];
	unsigned int land_buffer[MAX_SIZE_BUFFER];
	MPI_Status status;
    MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, &status);
    MPI_Send(&(main_loop_buffer_int[0]),1 , MPI_INT , req_status->MPI_SOURCE , REQUEST_INIT_SPLIT,comm);
    MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, req_status->MPI_SOURCE , REQUEST_RECEIVE_LAND, comm, &status);
    init_land(land_id, land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
    print_land(land_id);
    (*wait_for) = req_status->MPI_SOURCE;
}

void CAN_REQ_Send_Neighbour_order(MPI_Status *req_status,const MPI_Comm comm, const list *voisins) {
	unsigned int buffer_ui[MAX_SIZE_BUFFER];
	int main_loop_buffer_int;
	MPI_Status status;
	neighbour_to_buffer(voisins, buffer_ui);
	MPI_Recv(&main_loop_buffer_int ,1, MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, &status);
	MPI_Send((&buffer_ui[0]), voisins->nb_elem * (sizeof(neighbour)/sizeof(unsigned int)), MPI_UNSIGNED, req_status->MPI_SOURCE, ACK, comm);
}

void CAN_REQ_Request_init_split(MPI_Status *req_status, const MPI_Comm comm, const int com_rank, list *voisins, land *land_id, int *wait_for) {
	int count, main_loop_buffer_int;
	MPI_Status status;
	land new_land;
	unsigned int land_buffer[MAX_SIZE_BUFFER];
	list temp_voisins;
	init_list(&temp_voisins, sizeof(neighbour));
    MPI_Get_count (req_status, MPI_INT, &count);
    MPI_Recv(&main_loop_buffer_int ,count, MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, &status);
    split_land_update_neighbour(&new_land, land_id, &temp_voisins, voisins, req_status->MPI_SOURCE , com_rank);
    land_buffer[0] = new_land.x; land_buffer[1] = new_land.y;land_buffer[2] = new_land.size_x;land_buffer[3] = new_land.size_y;
    MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, req_status->MPI_SOURCE, REQUEST_RECEIVE_LAND, comm);
    CAN_Send_neighbour_list(&temp_voisins,RES_INIT_NEIGHBOUR , req_status->MPI_SOURCE, comm);
    list_clear(&temp_voisins, free_neighbour_cb);
    printf("je suis %d , transaction fini avec %d \n",com_rank, *wait_for);
    (*wait_for) = MPI_ANY_SOURCE;
}

int CAN_Node_Job(int com_rank, MPI_Comm comm) {
    int wait_for = -1;
    unsigned int main_loop_from;
    int main_loop_tag; // wait_array on attend un message d'une source avec un tag
    list voisins, temp_voisins;
    land land_id;
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
        	//Reception de l'ordre d'insertion
        	CAN_REQ_Root_init(&main_loop_status, comm, com_rank, &land_id);
        }

        else if(main_loop_tag == REQUEST_TO_JOIN) {
        	//Reception d'une demande pour joindre le réseau
        	//réponse possible :faire apsser ou accepter
        	CAN_REQ_Request_to_join(&main_loop_status,comm,com_rank, &land_id, &voisins ,&wait_for);
        }

        else if(main_loop_tag == SEND_LAND_ORDER) {
        	//Reception d'une demande e journalisation de la zone par le proc 0
        	CAN_REQ_Send_Land_order(&main_loop_status , comm, &land_id);
        }

        else if(main_loop_tag == RES_INIT_NEIGHBOUR) {
        	//Reception des nouveau voisin
        	CAN_REQ_Rec_Neighbours(&main_loop_status , comm, com_rank, &land_id, &voisins, &wait_for);
        }

        else if(main_loop_tag == UPDATE_NEIGBOUR) {
        	//Receptiond un nouveau voisin
        	CAN_REQ_Update_Neighbours(&main_loop_status  , comm, com_rank , &land_id, &voisins);
        }

        else if(main_loop_tag == RES_REQUEST_TO_JOIN) {
        	//Apres un,e deamdne pour joindre le réseau, un noued répon positivement
        	CAN_REQ_Res_Request_to_join(&main_loop_status, comm, com_rank , &land_id, &voisins, &wait_for);
        }

        else if(main_loop_tag == SEND_NEIGBOUR_ORDER) {
        	//Reception d'une demand de journalisation des voisisn du proc 0
        	CAN_REQ_Send_Neighbour_order(&main_loop_status, comm, &voisins);
        }

        else if(main_loop_tag == REQUEST_INIT_SPLIT) {
        	//Split la zone et répartie le voisins
        	CAN_REQ_Request_init_split(&main_loop_status , comm, com_rank , &voisins, &land_id, &wait_for);
        }


        else if(main_loop_tag == ATTACH_NEW_DATA) {
        	// traitement
        	CAN_REQ_Attach_new_data(&main_loop_status , comm, com_rank ,&land_id, &voisins);
        }

        else   {
            printf("unknow tag \n");
        }
    }
    return 1;
}
