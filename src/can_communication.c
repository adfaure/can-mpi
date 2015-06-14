#include "can_communication.h"
#include "utils.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define UNUSED(x) (void)(x)

void init_can_node(can_node *node) {
	init_list(&node->voisins, sizeof(neighbour));
	init_list(&(node->data_storage), sizeof(chunk));
	init_land(&node->land_id, 0 , 0, 0, 0);
	init_pair(&node->pair_id, 0 , 0);
}


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
	MPI_Status main_loop_status, data_status;
	unsigned int land_buffer[4] , buffer_ui[MAX_SIZE_BUFFER], buffer_simple_int = 0;
	int count, data_count; // wait_array on attend un message d'une source avec un tag
	land temp_land;
    list  temp_voisins, lands, data_localisations, int_list;
	neighbour temp_voisin;
	pair temp_pair;
	FILE * file;

	const char svg[5] = ".svg\0";
	const char txt[5] = ".txt\0";
	int size = strlen(base_path);
	char *txt_file      = malloc((size * sizeof(char)) + ( 5 * sizeof(char)) + 1);
	char *svg_name_file = malloc((size * sizeof(char)) + ( 5 * sizeof(char)) + 1);
	txt_file[size - 1] = '\0';
	svg_name_file[size - 1] = '\0';

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
	init_list(&data_localisations, sizeof(pair));
    init_list(&int_list, sizeof(int));

	for(int i = 0; i < nb_proc; i++) {
		if(i != root_rank) {
        	int from = i;
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

			MPI_Send(&buffer_simple_int ,1 , MPI_INT, i, SEND_DATA_ORDER, comm);
			MPI_Probe(i, ACK, comm, &data_status);
			MPI_Get_count (&data_status, MPI_UNSIGNED, &data_count);
			MPI_Recv(&buffer_ui[0] ,data_count, MPI_UNSIGNED, i, ACK, comm, &data_status);
			int nb_data = data_count / (sizeof(struct _pair) / sizeof(unsigned int));
			idx = 0;
			for(int j = 0; j < nb_data; j++) {
				init_pair(&temp_pair, buffer_ui[idx], buffer_ui[idx+1]);
				list_add_front(&data_localisations, &temp_pair);
                list_add_front(&int_list, &from);
				//log_factory(file, &temp_voisin, NEIGHBOUR_LOG, i);
				idx += 2;
			}
		}
	}

    create_svg_logs(svg_name_file , SIZE_X, SIZE_Y ,&lands, &data_localisations, &int_list);

	list_clear(&temp_voisins, free_neighbour_cb);
	list_clear(&lands,  free_land_cb);
	list_clear(&data_localisations, free_land_cb);
   // list_clear(&int_list, free_land_cb);

	free(svg_name_file);
	free(txt_file);
	fclose(file);
}

void nodes_set (bool * nodes_inserted, unsigned int size, unsigned int ith) {
	if (ith < size) {
		nodes_inserted[ith] = true;
	}
}

void nodes_print_not_inserted (bool * nodes_inserted, unsigned int size) {
	printf("availables: ");
	for (unsigned int i = 0; i < size; ++i) {
		if(nodes_inserted[i] == false) {
			printf("node%d, ", i);
		}
	}
	printf("\n");
}

int nodes_is_inserted (const bool * nodes_inserted, unsigned int size, unsigned int ith) {
	if (ith < size && nodes_inserted[ith]) {
		return false;
	} else {
		return true;
	}
}

void nodes_init (bool * nodes_inserted, unsigned int size) {
	for (unsigned int i = 0; i < size; ++i) {
		nodes_inserted[i] = false;
	}
	nodes_inserted[0] = true;
}

void prompt(int root_rank, MPI_Comm comm, int nb_proc) {
	unsigned int MAX_LEN = 128;
	char user_cmd[MAX_LEN];
	const char str_insert[] = "insert";
	const char str_all[] = "insert all";
	const char str_status[] = "status";
	const char str_log[] = "log";
	const char str_put[] = "put";
	const char str_get[] = "get";
	const char str_shuffle[] = "shuffle";
	const char str_rm[] = "rm";
	const char str_question_3[] = "etape3";
	unsigned int i = 1;

	bool * nodes_inserted = (bool *) malloc(sizeof(bool) * nb_proc);
	nodes_init(nodes_inserted, nb_proc);

	usleep(500000);
	printf("\n-- %d nodes availables --\n", nb_proc - 1);
	printf("> status             : show log about the state of the DHT\n");
	printf("> insert 2           : insert the node 2 in the overlay\n");
	printf("> insert all         : insert all nodes in the overlay and run etape 3\n");
	printf("> log                : add a textual/SVG log on logs/ directory\n");
	printf("> put <x> <y> <data> : put <data> in position (x, y)\n");
	printf("> get <x> <y>        : get a data from position (x, y)\n");
	printf("> shuffle <nb>       : randomly insert <nb> random data \n");
	printf("> rm <x> <y>         : remove a data in position (x, y)\n");
	printf("> etape3 <x>         : insert <x> element and try to retrieve the 5 last and first \n");
	printf("\n");

	// init de la première node :s
	if(CAN_Root_Process_Job_Insert_One(root_rank, comm, 1)) {
		nodes_set(nodes_inserted, nb_proc, 1);
	}
	while (1) {
		usleep(1000);
		printf("> ");
		fflush(stdout);
		fgets (user_cmd, MAX_LEN, stdin);
		if ((strlen(user_cmd) > 0) && (user_cmd[strlen (user_cmd) - 1] == '\n')){
			user_cmd[strlen (user_cmd) - 1] = '\0';
		}

		if (strcmp(str_all, user_cmd) == 0) {
			printf("---insert all: %s\n", user_cmd);
			CAN_Root_Process_insert_all(root_rank, comm, nb_proc);
			printf("---nbproc: %d\n", nb_proc);
			for (unsigned int j = 0; j < (unsigned int)nb_proc; ++j) {
				nodes_set(nodes_inserted, nb_proc, j);
			}
		}
		else if (strncmp(str_insert, user_cmd, 6) == 0) {
			printf("---insert : %s\n", user_cmd);
			char number[MAX_LEN];
			for (unsigned int j = 6; j < MAX_LEN-6; ++j) {
				number[j - 6] = user_cmd[j];
			}
			char *useless;
			int node_to_insert = (int)strtol(number, &useless, 10);
			printf("inserting node: %d\n", node_to_insert);

			if (nodes_is_inserted(nodes_inserted, nb_proc, node_to_insert)) {
				if(CAN_Root_Process_Job_Insert_One(root_rank, comm, node_to_insert)) {
					nodes_set(nodes_inserted, nb_proc, node_to_insert);
				}
			}
		}
		else if (strcmp(str_status, user_cmd) == 0) {
			printf("---status: %s\n", user_cmd);
			nodes_print_not_inserted(nodes_inserted, nb_proc);
		}
		else if (strcmp(str_log, user_cmd) == 0) {
			printf("---log: %s\n", user_cmd);
			const char* base_path = "logs/prompt_logged";
			const char* name[MAX_SIZE_BUFFER];
			sprintf((char *)name, "%s_%d", base_path, i++);
			CAN_Log_informations(comm, root_rank, nb_proc, (char *)name);
		}
		else if (strncmp(str_put, user_cmd, 3) == 0) {
			printf("---put: %s\n", user_cmd);
			int x, y, data;
			sscanf (user_cmd, "put %d %d %d", &x, &y, &data);
			DHT_put(root_rank, comm, x, y, data);
		}
		else if (strncmp(str_get, user_cmd, 3) == 0) {
			printf("---get: %s\n", user_cmd);
			int x, y;
			can_data data;
			sscanf (user_cmd, "get %d %d", &x, &y);
			DHT_get(comm, root_rank, x, y,&data);
			printf("found : %d \n", *(int*) data.data);
		} else if(strncmp(str_shuffle, user_cmd, 7) == 0)  {
			int x;
			sscanf (user_cmd, "shuffle %d", &x);
			CAN_shuffle_insert(DATA_INT , x , root_rank , comm);
		} else if (strncmp(str_rm, user_cmd, 2) == 0) {
			printf("---rm: %s\n", user_cmd);
			int x, y;
			sscanf (user_cmd, "rm %d %d", &x, &y);
			DHT_rm(root_rank, comm, x, y);
		} else if (strncmp(str_question_3, user_cmd, 2) == 0) {
			printf("---etape 3 : %s\n", user_cmd);
			int x;
			sscanf (user_cmd, "etape3 %d ", &x);
			CAN_Etape_trois(root_rank, nb_proc, comm, x);
		}
		else {
			printf("invalid command\n");
			fflush(stdout);
		}
	}
}

int CAN_Root_Process_Job_Insert_One(int root_rank, MPI_Comm comm, int proc_to_insert) {

	MPI_Status main_loop_status;
	int main_loop_tag, main_loop_buffer_int[MAX_SIZE_BUFFER]; // wait_array on attend un message d'une source avec un tag
	const char* base_path = "logs/log_in_process";
	const char* name[MAX_SIZE_BUFFER];

	if(proc_to_insert != root_rank) {
		MPI_Send(&proc_to_insert, 1, MPI_INT, proc_to_insert, ROOT_TAG_INIT_NODE, comm);
		MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, proc_to_insert, MPI_ANY_TAG, comm, &main_loop_status);
		main_loop_tag  = main_loop_status.MPI_TAG;
		if(main_loop_tag == ACK_TAG_BOOTSTRAP) {
			printf("bootstraping node did it well \n");
		} else if(main_loop_tag == GET_ENTRY_POINT) {
			main_loop_buffer_int[0] = 1;
			MPI_Send(&main_loop_buffer_int[0] ,1 , MPI_INT, proc_to_insert, SEND_ENTRY_POINT, comm);
			MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, proc_to_insert, ACK, comm, &main_loop_status); // le noeud i a été inséré
			sprintf((char *)name, "%s_%d", base_path, proc_to_insert);
		}
	}

	return 1;
}


int CAN_Root_Process_insert_all(int root_rank, MPI_Comm comm, int nb_proc) {

	MPI_Status main_loop_status;
	int main_loop_tag, main_loop_buffer_int[MAX_SIZE_BUFFER]; // wait_array on attend un message d'une source avec un tag
	const char* base_path = "logs/log_in_process_insertion";
	const char* name[MAX_SIZE_BUFFER];

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
	CAN_Log_informations(comm,root_rank, nb_proc, (char *)name);
	return 1;
}

int CAN_Root_Process_Job(int root_rank, MPI_Comm comm, int nb_proc) {

	MPI_Status main_loop_status;
	int main_loop_tag, main_loop_buffer_int[MAX_SIZE_BUFFER]; // wait_array on attend un message d'une source avec un tag
	const char* base_path = "logs/log_in_process_insertion";
	const char* name[MAX_SIZE_BUFFER];

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
	CAN_Etape_trois(root_rank, nb_proc,comm, 100);
	CAN_Log_informations(comm,root_rank, nb_proc, (char *)name);
	CAN_Quitt_overlay( root_rank,nb_proc, comm);
	return 1;
}

void CAN_Etape_trois(int com_rank,int nb_proc ,MPI_Comm comm, int nb_total) {
	int data[10];
	list pos_list;
	pair rand_pair, parcour_pair;
	int rand_int, idx_for_noob = 5;
	can_data data_res;
	init_list(&pos_list, sizeof(pair));
	printf("insertion of %d data, and then try to retrieve the  last and first \n", nb_total);
	for(int i = 0 ; i < nb_total; i++) {
		rand_int = rand() % 100;
		get_random_id(&rand_pair, SIZE_X, SIZE_Y);
		if(i >= 0 && i < 5) {
			data[i] = rand_int;
			list_add_front(&pos_list, &rand_pair);
		} else if(i >= nb_total - 5) {
			data[ idx_for_noob ] = rand_int;
			list_add_front(&pos_list, &rand_pair);
			idx_for_noob++;
		}
		CAN_Attach_new_data(com_rank, 1, comm, &rand_pair,&rand_int, DATA_INT, sizeof(int));
	}
	printf("all data are inserted in the CAN \n");
	CAN_Log_informations(comm,com_rank, nb_proc, "logs/question_3_log");
	for(int j =  0; j < pos_list.nb_elem ; j++) {
		list_get_index(&pos_list, j, &parcour_pair);
		DHT_get(comm, com_rank, parcour_pair.x, parcour_pair.y, &data_res);
		printf("data numéro : %d  needed : %d , found %d \n", j, data[pos_list.nb_elem - j - 1] ,*(int*)data_res.data);
		assert(data[pos_list.nb_elem - j - 1]  ==  *(int*)data_res.data); // assertion beacause we are crazy (and overconfident)
	}
}

int DHT_put(int root_rank, MPI_Comm comm, unsigned int x, unsigned int y, int data) {
	pair p;
	init_pair(&p, x, y);
	CAN_Attach_new_data(root_rank, 1, comm, &p, &data, DATA_INT , sizeof(int));
	//CAN_Log_informations(comm, root_rank, nb_proc, "logs/end_log");
	return 1;
}

int DHT_rm(int root_rank, MPI_Comm comm, unsigned int x, unsigned int y) {
	pair p;
	init_pair(&p, x, y);
	printf("DHT_rm(int root_rank, MPI_Comm comm, unsigned int x, unsigned int y) \n");
	CAN_Remove_data(root_rank, 1, comm, &p);
	//CAN_Log_informations(comm, root_rank, nb_proc, "logs/end_log");
	return 1;
}


int DHT_get(MPI_Comm comm, int root_rank, int x, int y, can_data * elem) {
	pair p;
	init_pair(&p, x, y);
	CAN_Fetch_data(comm, root_rank, 1, &p, elem);
	return 1;
}

void CAN_Attach_new_data(int self_rank, int first_node, MPI_Comm comm, pair *_pair, void *data, int data_type, unsigned int data_size) {
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

int CAN_Remove_data(int self_rank, int first_node, MPI_Comm comm, pair * p) {
	printf("int CAN_Remove_data(int self_rank, int first_node, MPI_Comm comm, pair * p) {\n");
	MPI_Status status;
	int useless;
	unsigned int total_size = 3;
	unsigned int buffer [3];
	buffer[0] = self_rank;
	buffer[1] = p->x;
	buffer[2] = p->y;
	printf("CAN_REMOVE_DATA%d, %d \n", first_node, self_rank);
	MPI_Send(&(buffer[0]), total_size, MPI_UNSIGNED, first_node, RM_DATA, comm);
	printf("int CAN_Remove_data(int self_rank, int first_node, MPI_Comm comm, pair * p) :: after send\n");
	MPI_Recv(&useless, 1,  MPI_INT,  MPI_ANY_SOURCE,  ACK,  comm,  &status);
	return 1;
}

void CAN_REQ_Attach_new_data(MPI_Status *req_status, MPI_Comm comm, can_node *node) {
	int buffer_int[MAX_SIZE_BUFFER], count, useless = 0;
	char buffer_char[MAX_SIZE_BUFFER];
	int from = req_status->MPI_SOURCE, tag = req_status->MPI_TAG;
	pair rec_pair;
	neighbour neighbour_temp_find;
	void * data;
	can_data can_data_1;
	MPI_Get_count (req_status, MPI_CHAR, &count);
	MPI_Recv(&buffer_char[0] ,count, MPI_CHAR, from, tag, comm, req_status);
	chunk c_1;

	memcpy(&buffer_int[0], &buffer_char[0],                        sizeof(unsigned int)); // from
	memcpy(&buffer_int[1], &buffer_char[1 * sizeof(unsigned int)], sizeof(unsigned int)); // x
	memcpy(&buffer_int[2], &buffer_char[2 * sizeof(unsigned int)], sizeof(unsigned int)); // y
	memcpy(&buffer_int[3], &buffer_char[3 * sizeof(unsigned int)], sizeof(unsigned int)); // data size
	memcpy(&buffer_int[4], &buffer_char[4 * sizeof(unsigned int)], sizeof(unsigned int)); // data type

	unsigned int x = buffer_int[1];
	unsigned int y = buffer_int[2];

	init_pair(&rec_pair, buffer_int[1], buffer_int[2]);
	if(is_land_contains_pair(&(node->land_id), &rec_pair)) {
		data = malloc(sizeof(buffer_int[3]));
		init_data(&can_data_1, buffer_int[3],  buffer_int[4], &buffer_char[5* sizeof(unsigned int)]);

		init_chunk(&c_1, x, y, &can_data_1);
		list_add_front(&(node->data_storage), &c_1);

		MPI_Send(&useless, 1, MPI_INT, buffer_int[0], ACK, comm);
		free(data);
		return;
	} else {
		if(find_neighbour(&(node->land_id), &(node->voisins), &rec_pair, &neighbour_temp_find)) {
			MPI_Send(&buffer_char[0], count, MPI_CHAR, neighbour_temp_find.com_rank, tag,  comm);
			//       	printf("[ %d ] --> [ %d ] ", com_rank, neighbour_temp_find.com_rank);
			//        	print_pair(&rec_pair);
		} else {
			printf("ERROR lors de la recherche de voisins \n");
		}
	}
}

void do_nothing_1 (void * n) {UNUSED(n);}

int list_cb_found (const void * c, const void * params) {
    chunk element = *(chunk *)c;
    pair pa = *(pair*) params;
    if (element.x == pa.x && element.y == pa.y) {
        return true;
    } else {
        return false;
    }
}

void CAN_REQ_Remove_data(MPI_Status * req_status, MPI_Comm comm, can_node *node) {
	neighbour neighbour_temp_find;
	unsigned int buffer [3], useless = 0;
	pair rec_pair;
	int from = req_status->MPI_SOURCE, tag = req_status->MPI_TAG;
	printf("CAN_REQ_Remove_data(MPI_Status * req_status, MPI_Comm comm, const int com_rank , can_node *node)\n");

	MPI_Recv(&buffer[0], 3, MPI_UNSIGNED, from, tag, comm, req_status);
	unsigned int origin_rank = buffer[0];
	unsigned int x = buffer[1];
	unsigned int y = buffer[2];

	init_pair(&rec_pair, x, y);
	print_pair(&rec_pair);
	if(is_land_contains_pair(&(node->land_id), &rec_pair)) {
		chunk c;

		list * data_storage = &(node->data_storage);
		if (list_find(data_storage, &rec_pair, list_cb_found, &c)) {
			int found = -1;
			for (int i = 0; i < data_storage->nb_elem; ++i) {
				list_get_index(data_storage, i, &c);
				if (c.x == x && c.y == y) {
					found = i;
					break;
				}
			}
			if (found != -1) {
				list_remove_index(&(node->data_storage), found, do_nothing_1);
				printf("REMOVED :) :) :) \n");
				MPI_Send(&useless, 1, MPI_UNSIGNED, origin_rank, ACK, comm);
			} else {
				printf("this is highly impossible \n");
				MPI_Send(&useless, 1, MPI_UNSIGNED, origin_rank, ACK, comm);
			}
		} else {
			printf("this is highly impossible \n");
			MPI_Send(&useless, 1, MPI_UNSIGNED, origin_rank, ACK, comm);
		}
	} else {
		if(find_neighbour(&(node->land_id), &(node->voisins), &rec_pair, &neighbour_temp_find)) {
			MPI_Send(&buffer[0], 3, MPI_UNSIGNED, neighbour_temp_find.com_rank, tag,  comm);
		} else {
			printf("ERROR lors de la recherche de voisins \n");
		}
	}
}

void CAN_Fetch_data(MPI_Comm comm, int com_rank, int first_rank ,const pair *pair, can_data * elem) {
	int buffer_int[MAX_SIZE_BUFFER], count;
	char buffer_char[MAX_SIZE_BUFFER];
	MPI_Status status;

	buffer_int[0] = com_rank;
	buffer_int[1] = pair->x;
	buffer_int[2] = pair->y;

	MPI_Send(&buffer_int[0], 3, MPI_INT, first_rank,FETCH_DATA, comm);
	MPI_Probe(MPI_ANY_SOURCE , SEND_FETCH_DATA, MPI_COMM_WORLD, &status);
	MPI_Get_count(&status,MPI_CHAR ,&count);
	MPI_Recv(&(buffer_char[0]), count, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, comm, &status);

	memcpy(&buffer_int[0], &buffer_char[0], sizeof(unsigned int)); // data size
	memcpy(&buffer_int[1], &buffer_char[1 * sizeof(unsigned int)], sizeof(unsigned int)); // data type
	init_data(elem, buffer_int[0], buffer_int[1], &buffer_char[2 *  sizeof(unsigned int)]);
}

void CAN_REQ_Fetch_data(MPI_Status *req_status, MPI_Comm comm, int com_rank, const can_node *node) {
	UNUSED(com_rank);
	int buffer_int[MAX_SIZE_BUFFER], count;
	char buffer_char[MAX_SIZE_BUFFER];
	int from = req_status->MPI_SOURCE, tag = req_status->MPI_TAG;
	pair rec_pair;
	neighbour neighbour_temp_find;
	can_data moc;
	chunk temp_chunk;
	MPI_Get_count (req_status, MPI_INT, &count);
	MPI_Recv(&buffer_int[0] ,count, MPI_INT, from, tag, comm, req_status);
	init_pair(&rec_pair, buffer_int[1], buffer_int[2]);
	if(is_land_contains_pair(&(node->land_id), &rec_pair)) {

		if(list_find(&(node->data_storage), &rec_pair, list_find_paire_equals_cb, &temp_chunk)) {
			get_data(&temp_chunk , &moc);
		} else {
			fprintf(stderr, "erreur \n");
		}
		int total_size = 0;
		memcpy(&buffer_char[0]                         , &(moc.element_size), (sizeof(unsigned int))); // data size
		memcpy(&buffer_char[1 * sizeof(unsigned int)  ], &(moc.data_type)   , (sizeof(unsigned int))); // data type
		memcpy(&buffer_char[2 * sizeof(unsigned int)  ], moc.data, moc.element_size);                  // element itself
		total_size = (2 * sizeof(unsigned int)) + moc.element_size;

		MPI_Send(&(buffer_char[0]), total_size,  MPI_CHAR,  buffer_int[0],  SEND_FETCH_DATA,  comm);
		//free_can_data_(&moc);
		return;
	} else {
		if(find_neighbour(&(node->land_id), &(node->voisins), &rec_pair, &neighbour_temp_find)) {
			MPI_Send(&buffer_int[0], count, MPI_INT, neighbour_temp_find.com_rank, tag,  comm);
			//printf("[ %d ] --> [ %d ] ", com_rank, neighbour_temp_find.com_rank);
			//print_pair(&rec_pair);
		} else {
			printf("ERROR lors de la recherche de voisins \n");
		}
	}
}

int CAN_REQ_Root_init(MPI_Status *req_status, MPI_Comm comm, int com_rank, can_node *node) {
	int count,send_int_buffer[MAX_SIZE_BUFFER] ,rec_buffer_int[MAX_SIZE_BUFFER];
	pair pair_id;

	get_random_id(&pair_id, SIZE_X, SIZE_Y);
	MPI_Get_count (req_status, MPI_INT, &count);
	MPI_Recv(&(rec_buffer_int[0]) ,count , MPI_INT, req_status->MPI_SOURCE,
	req_status->MPI_TAG , comm, MPI_STATUS_IGNORE);

	if(rec_buffer_int[0] == 1) {
		init_land(&(node->land_id), 0, 0, SIZE_X, SIZE_Y);
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

void CAN_REQ_Rec_Neighbours(MPI_Status *req_status, MPI_Comm comm, int com_rank, can_node *node) {
	UNUSED(com_rank);
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
		list_add_front(&(node->voisins), &temp_voisin);
		if((unsigned int)req_status->MPI_SOURCE == temp_voisin.com_rank ) {
			continue;
		}
		CAN_Send_neighbour(&temp_voisin,UPDATE_NEIGBOUR, temp_voisin.com_rank, comm);
		MPI_Recv(&dummy, 1, MPI_INT,temp_voisin.com_rank, ACK, comm, &status);
	}
	MPI_Send((&dummy) ,1 ,MPI_INT ,req_status->MPI_SOURCE ,ACK ,comm);
	//*wait_for = MPI_ANY_SOURCE;
}

int CAN_REQ_Rec_data(MPI_Status *req_status, MPI_Comm comm, int com_rank, can_node *node , int *wait_for) {
	UNUSED(com_rank);
	int count;
	char buffer[MAX_SIZE_BUFFER_CHAR];
	MPI_Get_count (req_status, MPI_CHAR, &count);
	MPI_Recv(&buffer[0], count, MPI_CHAR, req_status->MPI_SOURCE,req_status->MPI_TAG,comm, req_status);
	MPI_Send(&com_rank, 1, MPI_UNSIGNED, req_status->MPI_SOURCE, ACK, comm );
	buffer_to_chunk(&node->data_storage, buffer);
	list_apply(&node->data_storage ,print_one_chunk);
	MPI_Send((&count) ,1 ,MPI_INT , ROOT_PROCESS ,ACK ,comm);
	*wait_for = MPI_ANY_SOURCE;
	return 1;
}

// handle  a Join REQUEST return 1 if wait for is changed (maybe not a good convention)
int CAN_REQ_Request_to_join(MPI_Status *req_status,const MPI_Comm comm,const int com_rank,const can_node *node ,int *wait_for) {
	// Request de localisation, le noeud contenant la paire transporté répondra au noeud de la requete RES_REQUEST_TO_JOIN
	int buffer_int[MAX_SIZE_BUFFER];
	int from = req_status->MPI_SOURCE, tag = req_status->MPI_TAG;
	pair rec_pair;
	neighbour neighbour_temp_find;
	MPI_Recv(&buffer_int[0] ,3, MPI_INT, from, tag, comm, req_status);
	init_pair(&rec_pair, buffer_int[1], buffer_int[2]);
	if(is_land_contains_pair(&(node->land_id), &rec_pair)) {
		MPI_Send(&com_rank, 1, MPI_INT, buffer_int[0], RES_REQUEST_TO_JOIN,  comm);
		(*wait_for) = buffer_int[0];
		return 1;
	} else {
		if(find_neighbour(&(node->land_id), &(node->voisins), &rec_pair, &neighbour_temp_find)) {
			MPI_Send(&buffer_int[0], 3, MPI_INT, neighbour_temp_find.com_rank, REQUEST_TO_JOIN,  comm);
		} else {
			printf("ERROR lors de la recherche de voisins \n");
		}
	}
	return 0;
}

void CAN_REQ_Send_Land_order(MPI_Status *req_status,const MPI_Comm comm,const can_node *node) {
	int buffer_simple_int, count;
	unsigned int land_buffer[MAX_SIZE_BUFFER];
	MPI_Get_count (req_status, MPI_INT, &count);
	MPI_Recv(&buffer_simple_int , count , MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, MPI_STATUS_IGNORE);
	if(req_status->MPI_SOURCE == ROOT_PROCESS ) {
		land_buffer[0] = node->land_id.x; land_buffer[1] = node->land_id.y; land_buffer[2] = node->land_id.size_x; land_buffer[3] = node->land_id.size_y;
		MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, req_status->MPI_SOURCE, ACK, MPI_COMM_WORLD);
	}
}

void CAN_REQ_Update_Neighbours(MPI_Status *req_status,const MPI_Comm comm,const int com_rank ,can_node *node ) {
	UNUSED(com_rank);
	neighbour temp_voisin;
	int dummy = 0;
	CAN_Receive_neighbour(&temp_voisin, req_status->MPI_TAG, req_status->MPI_SOURCE,comm);
	temp_voisin.com_rank = req_status->MPI_SOURCE;
	update_neighbours(&(node->voisins), &(node->land_id) ,&temp_voisin);
	MPI_Send((&dummy) ,1 ,MPI_INT ,req_status->MPI_SOURCE, ACK ,comm);
}

void CAN_REQ_Res_Request_to_join(MPI_Status *req_status,const MPI_Comm comm,const int com_rank ,can_node *node, int *wait_for) {
	UNUSED(com_rank);

	int main_loop_buffer_int[MAX_SIZE_BUFFER];
	unsigned int land_buffer[MAX_SIZE_BUFFER];
	MPI_Status status;
	MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, &status);
	MPI_Send(&(main_loop_buffer_int[0]),1 , MPI_INT , req_status->MPI_SOURCE , REQUEST_INIT_SPLIT,comm);
	MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, req_status->MPI_SOURCE , REQUEST_RECEIVE_LAND, comm, &status);
	init_land(&(node->land_id), land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
	(*wait_for) = req_status->MPI_SOURCE;
}

void CAN_REQ_Send_Neighbour_order(MPI_Status *req_status,const MPI_Comm comm,const can_node *node) {
	unsigned int buffer_ui[MAX_SIZE_BUFFER];
	int main_loop_buffer_int;
	MPI_Status status;
	neighbour_to_buffer(&(node->voisins), buffer_ui);
	MPI_Recv(&main_loop_buffer_int ,1, MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, &status);
	MPI_Send((&buffer_ui[0]), node->voisins.nb_elem * (sizeof(neighbour)/sizeof(unsigned int)), MPI_UNSIGNED, req_status->MPI_SOURCE, ACK, comm);
}

void CAN_REQ_Send_data_order(MPI_Status *req_status ,MPI_Comm comm, int com_rank ,can_node *node) {
	UNUSED(com_rank);
	unsigned buffer[MAX_SIZE_BUFFER];
	int main_loop_buffer_int;
	chunk_to_buffer(&(node->data_storage), buffer);
	MPI_Recv(&main_loop_buffer_int ,1, MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, req_status);
	MPI_Send((&buffer[0]), node->data_storage.nb_elem * (sizeof(pair)/sizeof(unsigned int)), MPI_UNSIGNED, req_status->MPI_SOURCE, ACK, comm);
}

void CAN_REQ_Request_init_split(MPI_Status *req_status, const MPI_Comm comm, const int com_rank, can_node *node, int *wait_for) {
	int count, main_loop_buffer_int;
	MPI_Status status;
	land new_land;
	unsigned int land_buffer[MAX_SIZE_BUFFER];
	list temp_voisins, data_to_send;
	init_list(&temp_voisins, sizeof(neighbour));
	init_list(&data_to_send, sizeof(chunk));

	MPI_Get_count (req_status, MPI_INT, &count);
	MPI_Recv(&main_loop_buffer_int ,count, MPI_INT, req_status->MPI_SOURCE, req_status->MPI_TAG, comm, &status);
	split_land_update_neighbour(&new_land, &(node->land_id), &temp_voisins, &(node->voisins), req_status->MPI_SOURCE , com_rank);
	distribute_data_after_split(&(node->land_id), &(node->data_storage), &data_to_send);
	land_buffer[0] = new_land.x; land_buffer[1] = new_land.y;land_buffer[2] = new_land.size_x;land_buffer[3] = new_land.size_y;
	MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, req_status->MPI_SOURCE, REQUEST_RECEIVE_LAND, comm);
	CAN_Send_neighbour_list(&temp_voisins,RES_INIT_NEIGHBOUR , req_status->MPI_SOURCE, comm);
	CAN_Send_data_update(&data_to_send, RES_INIT_DATA, req_status->MPI_SOURCE ,comm);

	list_clear(&temp_voisins, free_neighbour_cb);
	list_clear(&data_to_send, free_chunk_cb);
	(*wait_for) = MPI_ANY_SOURCE;
}

int CAN_Node_Job(int com_rank, MPI_Comm comm) {
	can_node node;
	unsigned int main_loop_from;
	UNUSED(main_loop_from);
	int main_loop_tag, flag; // wait_array on attend un message d'une source avec un tag
	MPI_Status main_loop_status;
	int wait_for = MPI_ANY_SOURCE;
	init_can_node(&node);
	/*
	int MPI_Iprobe(int source, int tag, MPI_Comm comm, int *flag,
	MPI_Status *status)
	*/
	while(1) {
		usleep(1000); // nice idle =)
		MPI_Iprobe(wait_for,MPI_ANY_TAG, MPI_COMM_WORLD,&flag , &main_loop_status);
		if(flag) {
			main_loop_tag  = main_loop_status.MPI_TAG;

			if(main_loop_tag == ROOT_TAG_INIT_NODE) {
				//Reception de l'ordre d'insertion
				CAN_REQ_Root_init(&main_loop_status, comm, com_rank, &node);
			}

			else if(main_loop_tag == REQUEST_TO_JOIN) {
				//Reception d'une demande pour joindre le réseau
				//réponse possible :faire apsser ou accepter
				CAN_REQ_Request_to_join(&main_loop_status,comm,com_rank, &node ,&wait_for);
			}

			else if(main_loop_tag == SEND_LAND_ORDER) {
				//Reception d'une demande e journalisation de la zone par le proc 0
				CAN_REQ_Send_Land_order(&main_loop_status , comm, &node);
			}

			else if(main_loop_tag == SEND_DATA_ORDER) {
				// traitement
				//void CAN_REQ_Fetch_data(MPI_Status req_status ,MPI_Comm comm,int com_rank ,const land *land_id, const list * voisins) {
				CAN_REQ_Send_data_order(&main_loop_status , comm, com_rank ,&node);
			}

			else if(main_loop_tag == RES_INIT_NEIGHBOUR) {
				//Reception des nouveau voisin
				CAN_REQ_Rec_Neighbours(&main_loop_status , comm, com_rank, &node);
			}

			else if(main_loop_tag == RES_INIT_DATA) {
				//Reception des nouveau voisin
				CAN_REQ_Rec_data(&main_loop_status , comm, com_rank, &node, &wait_for);
			}

			else if(main_loop_tag == UPDATE_NEIGBOUR) {
				//Receptiond un nouveau voisin
				CAN_REQ_Update_Neighbours(&main_loop_status  , comm, com_rank ,&node);
			}

			else if(main_loop_tag == RES_REQUEST_TO_JOIN) {
				//Apres un,e deamdne pour joindre le réseau, un noued répon positivement
				CAN_REQ_Res_Request_to_join(&main_loop_status, comm, com_rank , &node, &wait_for);
			}

			else if(main_loop_tag == SEND_NEIGBOUR_ORDER) {
				//Reception d'une demand de journalisation des voisisn du proc 0
				CAN_REQ_Send_Neighbour_order(&main_loop_status, comm, &node);
			}

			else if(main_loop_tag == REQUEST_INIT_SPLIT) {
				//Split la zone et répartie le voisins
				CAN_REQ_Request_init_split(&main_loop_status , comm, com_rank , &node, &wait_for);
			}

			else if(main_loop_tag == ATTACH_NEW_DATA) {
				// traitement
				CAN_REQ_Attach_new_data(&main_loop_status, comm, &node);
			}

			else if(main_loop_tag == RM_DATA) {
				// traitement
				printf("else if(main_loop_tag == RM_DATA) {\n");
				CAN_REQ_Remove_data(&main_loop_status, comm,&node);
			}

			else if(main_loop_tag == FETCH_DATA) {
				// traitement
				//void CAN_REQ_Fetch_data(MPI_Status req_status ,MPI_Comm comm,int com_rank ,const land *land_id, const list * voisins) {
				CAN_REQ_Fetch_data(&main_loop_status , comm, com_rank ,&node);
			} else if(main_loop_tag == SEND_ORDER_QUIT) {
				// traitement
				//void CAN_REQ_Fetch_data(MPI_Status req_status ,MPI_Comm comm,int com_rank ,const land *land_id, const list * voisins) {
				break;
			}


			else   {
				printf("unknow tag \n");
			}
		}
	}
	list_clear(&(node.data_storage), free_chunk_cb);
	list_clear(&(node.voisins), free_neighbour_cb);
	printf("[ %d ] quit \n", com_rank);
	return 1;
}

void CAN_Quitt_overlay( int root,int nb_proc, MPI_Comm comm) {
	UNUSED(root);
	int ack;
	for(int i = 0 ; i < nb_proc; i++) {
		if(i == ROOT_PROCESS ) continue;
		MPI_Send(&ack, 1, MPI_INT, i, SEND_ORDER_QUIT, comm);
	}
}

void free_dummy(void *elem) {UNUSED(elem);}

// je pars du principe que tout ce qui n'est pas dans le land 1 doit etre mis dans la seconde liste.
void distribute_data_after_split(const land *land, list *old_list, list *new_list) {
	chunk temp;
	list parcours_list;
	pair temp_pair;
	list_cp_revert(old_list, NULL, &parcours_list);
	init_list(new_list , old_list->element_size);

	list_clear(old_list, free_dummy);
	list_clear(new_list, free_dummy);

	for(int i = 0 ; i < parcours_list.nb_elem; i++) {
		list_get_index(&parcours_list, i, &temp);
		init_pair(&temp_pair, temp.x, temp.y);
		if(is_land_contains_pair(land, &temp_pair)) {
			list_add_front(old_list, &temp);
		} else {
			list_add_front(new_list, &temp);
		}
	}
}

void chunks_to_buffer(const list *list, char buffer[MAX_SIZE_BUFFER_CHAR],unsigned int *char_size) {
	chunk temp;unsigned int idx = 0;
	unsigned int data_size;
	memcpy(&buffer[0], &(list->nb_elem) , sizeof(unsigned int)); //nb total de chunk obligé
	idx = sizeof(unsigned int);

	for(int i = 0; i < list->nb_elem; i++) {
		list_get_index(list, i, &temp);
		memcpy(&buffer[idx], &temp.x, sizeof(unsigned int));idx += sizeof(unsigned int);
		memcpy(&buffer[idx], &temp.y, sizeof(unsigned int));idx += sizeof(unsigned int);
		memcpy(&buffer[idx], &(temp.data_wrapper.element_size), sizeof(unsigned int));idx += sizeof(unsigned int);
		memcpy(&buffer[idx], &(temp.data_wrapper.data_type), sizeof(unsigned int));idx += sizeof(unsigned int);
		data_size = temp.data_wrapper.element_size;
		memcpy(&buffer[idx], temp.data_wrapper.data, data_size); idx += data_size;
	}
	*char_size = idx;
}

void CAN_shuffle_insert(int data_type, int nb_elem, int self_rank, MPI_Comm comm) {
	UNUSED(data_type);
	pair temp ;
	can_data data;
	int i_rand;
	for(int i = 0; i < nb_elem; i ++) {
		get_random_id(&temp, SIZE_X, SIZE_Y);
		i_rand = rand() % 1000;
		init_data(&data, sizeof(unsigned int), DATA_INT, &i_rand );
		CAN_Attach_new_data(self_rank, 1,comm,&temp ,&i_rand , DATA_INT, sizeof(unsigned int) );
		free_can_data_(&data);
	}
}

void buffer_to_chunk(list *list, const char buffer[MAX_SIZE_BUFFER_CHAR]) {
	chunk temp;
	unsigned int x, y, size, elem_type;
	can_data temp_data;
	unsigned int nb_total_chunk, idx = 0;
	memcpy(&nb_total_chunk, &buffer[0], sizeof(unsigned int));
	idx = sizeof(unsigned int);
	for(unsigned int i = 0 ; i < nb_total_chunk; i++) {
		memcpy(&x, &buffer[idx], sizeof(unsigned int)); idx += sizeof(unsigned int);
		memcpy(&y, &buffer[idx], sizeof(unsigned int)); idx += sizeof(unsigned int);
		memcpy(&size, &buffer[idx], sizeof(unsigned int)); idx += sizeof(unsigned int);
		memcpy(&elem_type, &buffer[idx], sizeof(unsigned int)); idx += sizeof(unsigned int);
		init_data(&temp_data, size, elem_type, &buffer[idx]); idx += size;
		init_chunk(&temp,x, y, &temp_data);
		list_add_front(list, &temp);
	}
}

/*
* only x and y
*/
void chunk_to_buffer(const list* ch, unsigned int buffer[MAX_SIZE_BUFFER]) {
	chunk temp; int idx = 0;
	for(int i = 0; i < ch->nb_elem; i++) {
		list_get_index(ch, i, &temp);
		buffer[idx]     = temp.x;
		buffer[idx + 1] = temp.y;
		idx += 2;
	}
}


int CAN_Send_data_update(const list *list, int mpi_tag, int mpi_destinataire,  MPI_Comm comm ) {
	char buffer[MAX_SIZE_BUFFER_CHAR];
	unsigned int size;
	chunks_to_buffer(list, buffer, &size);
	MPI_Send(&buffer[0],size, MPI_CHAR, mpi_destinataire, mpi_tag, comm);
	MPI_Recv(&size, 4, MPI_UNSIGNED, mpi_destinataire, ACK, comm, MPI_STATUS_IGNORE);
	return 1;
}
