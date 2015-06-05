#include "mpi_can.h"

//root process
#define ROOT_PROCESS 0

// world size
#define SIZE_X 1000
#define SIZE_Y 1000

int main(int argc, char**argv) {
  FILE *file;
  file = fopen ("log.txt", "w+");

  if(!file) {
    printf(" erreur lors de l'ouverture du fichier de log \n");
    return 0;
  }
  fprintf(file, "log \n");
  int nb_proc, com_rank,node_number , i, corresp, localise;
  unsigned int land_buffer[4] , buffer_ui[MAX_SIZE_BUFFER], buffer_simple_int = 0;
  int wait_for = -1;
  int main_loop_tag, main_loop_from, count, main_loop_buffer_int[MAX_SIZE_BUFFER], send_int_buffer[MAX_SIZE_BUFFER], wait_array[2]; // wait_array on attend un message d'une source avec un tag
  bool bootstrap = false, active = false, is_waiting = false;
  list voisins, temp_voisins;
  neighbour neighbour_temp_find, temp_voisin;
  pair pair_id, pair_join_request;
  land land_id, new_land, temp_land;
  MPI_Status main_loop_status;
  MPI_Request main_loop_request;
  MPI_Init (&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &com_rank);
  init_list(&voisins, sizeof(neighbour));
  init_list(&temp_voisins, sizeof(neighbour));
  srand(com_rank * nb_proc);

  if(com_rank == ROOT_PROCESS) {
    for(i = 0; i < nb_proc; i++) {
      if(i != ROOT_PROCESS) {
        MPI_Send(&i, 1, MPI_INT, i, ROOT_TAG_INIT_NODE, MPI_COMM_WORLD);
        MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
        main_loop_from = main_loop_status.MPI_SOURCE;
        main_loop_tag  = main_loop_status.MPI_TAG;
        if(main_loop_tag == ACK_TAG_BOOTSTRAP) {
          printf("bootstraping node did it well \n");
        } else if(main_loop_tag == GET_ENTRY_POINT) {
          main_loop_buffer_int[0] = 1;
          MPI_Send(&main_loop_buffer_int[0] ,1 , MPI_INT, i, SEND_ENTRY_POINT, MPI_COMM_WORLD );
        }
      }
    }

    int dummy = 0;
    for(int i = 0; i < 10000000; i++) {
      dummy = i * i;
    }

    for(i = 0; i < nb_proc; i++) {
      if(i != ROOT_PROCESS) {
          MPI_Send(&buffer_simple_int ,1 , MPI_INT, i, SEND_LAND_ORDER, MPI_COMM_WORLD);
          MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, i, ACK, MPI_COMM_WORLD, &main_loop_status);
          init_land(&temp_land, land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
          log_factory(file, &temp_land, LAND_LOG, i);
          MPI_Send(&buffer_simple_int ,1 , MPI_INT, i, SEND_NEIGBOUR_ORDER, MPI_COMM_WORLD);
          MPI_Probe(i, ACK, MPI_COMM_WORLD, &main_loop_status);
          MPI_Get_count (&main_loop_status, MPI_UNSIGNED, &count);
          MPI_Recv(&buffer_ui[0] ,count, MPI_UNSIGNED, i, ACK, MPI_COMM_WORLD, &main_loop_status);
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

  } else {
    get_random_id(&pair_id, SIZE_X, SIZE_Y);
    while(1) {
      list_clear(&temp_voisins, free_neighbour_cb);
      if(wait_for != -1) {
        printf("je suis [%d] en attente d'un message de %d \n", com_rank, wait_for);
      }
      printf("[ %d ] en attente de message \n", com_rank);
      MPI_Probe(wait_for, MPI_ANY_TAG, MPI_COMM_WORLD, &main_loop_status);
      main_loop_from = main_loop_status.MPI_SOURCE;
      main_loop_tag  = main_loop_status.MPI_TAG;
      printf("[ %d ],  recu %d, from %d \n", com_rank, main_loop_tag ,main_loop_from );
      if(main_loop_tag == ROOT_TAG_INIT_NODE) {
        MPI_Get_count (&main_loop_status, MPI_INT, &count);
        MPI_Recv(&main_loop_buffer_int[0] ,count , MPI_INT, main_loop_from,
          main_loop_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(bootstrap == false && *main_loop_buffer_int == 1) {
          bootstrap   = true;
          main_loop_buffer_int [0] = 0;
          init_land(&land_id, 0, 0, SIZE_X, SIZE_Y);
          MPI_Send(&main_loop_buffer_int[0] ,1 ,MPI_INT ,ROOT_PROCESS ,ACK_TAG_BOOTSTRAP ,MPI_COMM_WORLD);
        } else {
          main_loop_buffer_int[0] = com_rank;
          MPI_Send(&main_loop_buffer_int[0] ,1 , MPI_INT ,ROOT_PROCESS ,GET_ENTRY_POINT ,MPI_COMM_WORLD);
          MPI_Recv(&main_loop_buffer_int[0] ,1 , MPI_INT, ROOT_PROCESS, SEND_ENTRY_POINT, MPI_COMM_WORLD, &main_loop_status);
          send_int_buffer[0] = com_rank; send_int_buffer[1] = pair_id.x; send_int_buffer[2] = pair_id.y;
          MPI_Send((&send_int_buffer[0]), 3, MPI_INT, main_loop_buffer_int[0], REQUEST_TO_JOIN, MPI_COMM_WORLD);
        }
      }
      else if(main_loop_tag == REQUEST_TO_JOIN) {
      // Request de localisation, le noeud contenant la paire transporté répondra au noeud de la requete RES_REQUEST_TO_JOIN
        MPI_Recv(&main_loop_buffer_int[0] ,3, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        init_pair(&pair_join_request, main_loop_buffer_int[1], main_loop_buffer_int[2]);
        if(is_land_contains_pair(&land_id, &pair_join_request)) {
            MPI_Send(&com_rank, 1, MPI_INT, main_loop_buffer_int[0], RES_REQUEST_TO_JOIN,  MPI_COMM_WORLD);
            wait_for = main_loop_buffer_int[0];
        } else {
          if(find_neighbour(&voisins, &pair_join_request, &neighbour_temp_find)) {
            print_pair(&pair_join_request);
            MPI_Send(&main_loop_buffer_int[0], 3, MPI_INT, neighbour_temp_find.com_rank, REQUEST_TO_JOIN,  MPI_COMM_WORLD);
          } else {
            printf("ERROR lors de la recherche de voisins \n");
          }
        }
      }
      else if(main_loop_tag == SEND_LAND_ORDER) {
        MPI_Get_count (&main_loop_status, MPI_INT, &count);
        MPI_Recv(&buffer_simple_int , count , MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(main_loop_from == ROOT_PROCESS ) {
          printf("recu SEND_LAND_ORDER \n");
          land_buffer[0] = land_id.x; land_buffer[1] = land_id.y;land_buffer[2] = land_id.size_x;land_buffer[3] = land_id.size_y;
          MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, main_loop_from, ACK, MPI_COMM_WORLD);
        }
      }
      else if(main_loop_tag == RES_INIT_NEIGHBOUR) {
        int nb_voisins, idx = 0;
        MPI_Get_count (&main_loop_status, MPI_UNSIGNED, &count);
        MPI_Recv(&buffer_ui[0] ,count, MPI_UNSIGNED, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        nb_voisins = count / (sizeof(neighbour) / sizeof(unsigned int));
        for(int i = 0; i < nb_voisins; i++) {
          init_neighbour(&temp_voisin, buffer_ui[idx], buffer_ui[idx+1], buffer_ui[idx+2],buffer_ui[idx+3], buffer_ui[idx+4]);
          idx += 5;
          list_add_front(&voisins, &temp_voisin);
          print_neighbour(&temp_voisin);
          if(main_loop_from == temp_voisin.com_rank ) {
            continue;
          }
          CAN_Send_neighbour(&temp_voisin,UPDATE_NEIGBOUR, temp_voisin.com_rank, MPI_COMM_WORLD );
          print_neighbour(&temp_voisin);
        }
        printf("voisins recu \n");
        list_apply(&voisins, print_neighbour_cb);
        printf("\n");
      }

      else if(main_loop_tag == UPDATE_NEIGBOUR) {
        CAN_Receive_neighbour(&temp_voisin, main_loop_tag, main_loop_from, MPI_COMM_WORLD);
        printf("Je suis %d, Mes amis je suis heureux de vous annoncer que nous acceuilons à présent un nouveau voisins [%d]! \n ", com_rank, main_loop_from);
        print_neighbour(&temp_voisin);
        update_neighbours(&voisins, &land_id ,&temp_voisin);
        temp_voisin.com_rank = main_loop_from;
        list_add_front(&voisins, &temp_voisin);
      }

      else if(main_loop_tag == RES_REQUEST_TO_JOIN) {
        MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        MPI_Send(&(main_loop_buffer_int[0]),1 , MPI_INT , main_loop_from , REQUEST_INIT_SPLIT, MPI_COMM_WORLD);
        MPI_Recv(&(land_buffer[0]), 4, MPI_UNSIGNED, main_loop_from, REQUEST_RECEIVE_LAND, MPI_COMM_WORLD, &main_loop_status);
        init_land(&land_id, land_buffer[0], land_buffer[1] , land_buffer[2], land_buffer[3]);
        print_land(&land_id);
      }

      else if(main_loop_tag == SEND_NEIGBOUR_ORDER) {
        neighbour_to_buffer(&voisins, buffer_ui);
        MPI_Recv(&main_loop_buffer_int[0] ,1, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        MPI_Send((&buffer_ui[0]), voisins.nb_elem * (sizeof(neighbour)/sizeof(unsigned int)), MPI_UNSIGNED, main_loop_from, ACK, MPI_COMM_WORLD);
      }

      else if(main_loop_tag == REQUEST_INIT_SPLIT) {
        MPI_Get_count (&main_loop_status, MPI_INT, &count);
        MPI_Recv(&main_loop_buffer_int[0] ,count, MPI_INT, main_loop_from, main_loop_tag, MPI_COMM_WORLD, &main_loop_status);
        split_land_update_neighbour(&new_land, &land_id, &temp_voisins, &voisins, main_loop_from , com_rank);
        land_buffer[0] = new_land.x; land_buffer[1] = new_land.y;land_buffer[2] = new_land.size_x;land_buffer[3] = new_land.size_y;
        MPI_Send(&land_buffer[0], 4, MPI_UNSIGNED, main_loop_from, REQUEST_RECEIVE_LAND, MPI_COMM_WORLD );
        CAN_Send_neighbour_list(&temp_voisins,RES_INIT_NEIGHBOUR , main_loop_from, MPI_COMM_WORLD);
        printf("envoie des voisins \n");
        list_apply(&temp_voisins, print_neighbour_cb);
        list_clear(&temp_voisins, free_neighbour_cb);
        printf("je suis %d , transaction fini avec %d \n",com_rank, wait_for);
        wait_for = -1;
      }

      else if(main_loop_tag == LOCALIZE) {
        corresp = main_loop_buffer_int[0];
        init_pair(&pair_join_request,  main_loop_buffer_int[1], main_loop_buffer_int[2]);
        if(is_land_contains_pair(&land_id, &pair_join_request)) {
          MPI_Send(&com_rank, 1, MPI_INT, corresp, LOCALIZE_RESP, MPI_COMM_WORLD);
        } else {
        }
      }

      else   {
        printf("unknow tag \n");
      }
    }
  }

  MPI_Finalize();
  fclose(file);
  return 0;
}
