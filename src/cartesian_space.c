#include "cartesian_space.h"

int CAN_Send_neighbour(const neighbour *neig, int mpi_tag, int mpi_destinataire,  MPI_Comm comm) {
  unsigned int buffer[5],  size = (int) (sizeof(neighbour) / sizeof(unsigned int));
  buffer[0] = neig->x;
  buffer[1] = neig->y;
  buffer[2] = neig->size;
  buffer[3] = neig->orientation;
  buffer[4] = neig->com_rank;
  MPI_Send(&buffer[0],  size, MPI_UNSIGNED, mpi_destinataire,  mpi_tag,  comm);
  return 1;
}

int CAN_Send_neighbour_list(const list *l, int mpi_tag, int mpi_destinataire,  MPI_Comm comm) {
  unsigned int buffer[MAX_SIZE_BUFFER],  size = l->nb_elem * (sizeof(neighbour) / sizeof(unsigned int));
  neighbour_to_buffer(l,  buffer);
  MPI_Send(&buffer[0],  size, MPI_UNSIGNED, mpi_destinataire,  mpi_tag,  comm);
  return 1;
}

//MPI_Recv(&main_loop_buffer_int[0], count,  MPI_INT,  main_loop_from,  main_loop_tag,  MPI_COMM_WORLD,  &main_loop_status);
int CAN_Receive_neighbour(neighbour *neig, int mpi_tag, int mpi_src,  MPI_Comm comm ) {
  unsigned int buffer[5],  size =  (sizeof(neighbour) / sizeof(unsigned int)) ;
  printf("reception de %d ui \n", (int) size);
  MPI_Recv(&buffer[0],  size,  MPI_UNSIGNED, mpi_src,  mpi_tag,  comm,  MPI_STATUS_IGNORE);
  init_neighbour(neig,  buffer[0],  buffer[1],  buffer[2], buffer[3],  buffer[4]);
  return 1;
}

void split_land_update_neighbour(land *new_land,  land *old_land,  list *new_n, list *old_n,  int new_rank,  int old_rank) {
  neighbour temp, old, new;
  split_land(new_land,  old_land);
  land_extract_neighbourg_after_split(new_land,  old_land,  &old,  &new);
  old.com_rank = new_rank;
  new.com_rank = old_rank;
  for(int i = 0; i < old_n->nb_elem; i++) {
    list_get_index(old_n, i, &temp);
    if(is_neighbour(new_land,  &temp)) {
      adjust_neighbour(new_land,  &temp);
      list_add_front(new_n,  &temp);
    }
    list_get_index(old_n, i,  &temp);

    if(is_neighbour(old_land,  &temp)) {
      if(adjust_neighbour(old_land,  &temp)) {
        list_replace_index(old_n,  i,  &temp);
      }
    }
  }
  list_add_front(new_n,  &new);
  list_add_front(old_n,  &old);
}

int adjust_neighbour(land *land,  neighbour *n) {
  int changed = 0;
  if(n->orientation == VOISIN_H) {
    if(n->x < land->x) {
      n->size = n->size - (land->x - n->x);
      n->x = land->x;
      changed = 1;
    }
    if(n->x + n->size < land->x + land->size_x) {
      n->size = n->size - (land->x - n->x);
      changed = 1;
    }
    if(n->x + n->size > land->x + land->size_x) {
      n->size = n->size - ((n->x + n->size)  - (land->x + land->size_x));
      changed = 1;
    }
  } else if(n->orientation == VOISIN_V) {
    if(n->y < land->y) {
      n->size = n->size - (land->y - n->y);
      n->y = land->y;
      changed = 1;
    }
    if(n->y + n->size > land->y + land->size_y) {
      n->size = n->size - ((n->y + n->size) - (land->y + land->size_y));
      changed = 1;
    }
  }
  return changed;
}

void land_extract_neighbourg_after_split(land *land1,  land * land2,  neighbour *n1,  neighbour *n2) {
  if(land1->x == land2->x) // alors ils sont l'un au dessus de l'autre
  {
    n1->size = land1->size_x;
    n2->size = land1->size_x;
    n2->x = land2->x;
    n1->x = land1->x;
    n1->orientation = VOISIN_H;
    n2->orientation = VOISIN_H;
    if(land1->y < land2->y) {
      n1->y = land2->y;
      n2->y = land1->y + land1->size_y;
    } else {
      n2->y = land1->y;
      n1->y = land2->y + land2->size_y;
    }
  } else if(land1->y == land2->y) {
    n1->size = land1->size_y;
    n2->size = land1->size_y;
    n2->y = land2->y;
    n1->y = land1->y;
    n1->orientation = VOISIN_V;
    n2->orientation = VOISIN_V;
    if(land1->x < land2->x) {
      n1->x = land2->x;
      n2->x = land1->x + land1->size_x;
    } else {
      n2->x = land1->x;
      n1->x = land2->x + land2->size_x;
    }
  } else {
    printf("impossible \n");
  }
}

int find_neighbour(const list *l, const pair *pair, neighbour *res) {
  neighbour temp;
  int better;
  double min, val_temp;
  if(l->nb_elem == 0) {
    return 0;
  }
  min = 10000000; better = 0;
  for(int i = 0 ; i < l->nb_elem; i++) {
    list_get_index(l,  i,  &temp);
    val_temp = entire_dist_neigbourg(pair->x,  pair->y,  &temp);
    if(val_temp < min) {
      min = val_temp;
      better = i;
    }
  }
  list_get_index(l,  better,  res);
  return 1;
}


double entire_dist_neigbourg(int x1,  int y1,  const neighbour *neighbour) {
    int x,  y;
    if(neighbour->orientation == VOISIN_H) {
      y = neighbour->y;
      x = neighbour->x + (neighbour->size / 2);
    } else {
      x = neighbour->x;
      y = neighbour->y + (neighbour->size / 2);
    }
    return sqrt(pow((double)(x - x1),  2) + pow((double)(y - y1), 2 ));
}


void print_land(const land *l) {
  printf("((%u,  %u),  (%u,  %u))\n",  l->x,   l->size_x,  l->y,  l->size_y);
}


void init_neighbour(neighbour *n,  unsigned int x,  unsigned int y,  unsigned int size,  unsigned int or,  unsigned int com_rank) {
  n->size = size;
  n->x = x;
  n->y = y;
  n->orientation = or;
  n->com_rank = com_rank;
}

int is_neighbour(const land *land,  const neighbour *n) {
    printf(" is neigh ");
  if(is_neighbour_top(land,  n)) {
    printf(" is TOP \n");
    return VOISIN_TOP;
  } else if (is_neighbour_bot(land,  n)) {
    printf(" is BOT \n");
    return VOISIN_BOT;
  } else if(is_neighbour_left(land,  n)) {
    printf(" is LEFT \n");
    return VOISIN_LEFT;
  } else if(is_neighbour_right(land,  n)) {
    printf(" is RIGHT \n");
    return VOISIN_RIGHT;
  } else {
    printf(" is NONE \n");
    return VOISIN_NONE;
  }
}

void neighbour_to_buffer(const list *l,  unsigned int buffer[MAX_SIZE_BUFFER]) {
  neighbour temp;
  int idx = 0;
  for(int i = 0; i < l->nb_elem; i++) {
    list_get_index(l, i, &temp);
    buffer[idx]   = temp.x;
    buffer[idx+1] = temp.y;
    buffer[idx+2] = temp.size;
    buffer[idx+3] = temp.orientation;
    buffer[idx+4] = temp.com_rank;
    idx += 5;
  }
}

bool is_neighbour_top(const land *land, const neighbour *n) {
    if(n->orientation == VOISIN_V)
      return false;

    bool temp = (land->x <= n->x && n->x <= land->x + land->size_x) || (land->x <= n->x + n->size && n->x + n->size <= land->x + land->size_x);
    return temp && (land->y == n->y);
}

bool is_neighbour_bot(const land *land,  const neighbour *n) {
    if(n->orientation == VOISIN_V)
      return false;

    bool temp = (land->x <= n->x && n->x <= land->x + land->size_x) || (land->x <= n->x + n->size && n->x + n->size <= land->x + land->size_x);
    return temp && (n->y == land->y + land->size_y);
}

bool is_neighbour_left(const land *land,  const neighbour *n) {
    if(n->orientation == VOISIN_H)
      return false;

    bool temp = (land->y <= n->y && n->y <= land->y + land->size_y) || (land->y <= n->y + n->size && n->y + n->size <= land->y + land->size_y);
    return temp && (n->x == land->x);
}

bool is_neighbour_right(const land *land,  const neighbour *n) {
    if(n->orientation == VOISIN_H)
      return false;

      bool temp = (land->y <= n->y && n->y <= land->y + land->size_y) || (land->y <= n->y + n->size && n->y + n->size <= land->y + land->size_y);
      return temp && (n->x == land->x + land->size_x);
}

void print_neighbour(const neighbour *n) {
  if(n->orientation == VOISIN_V) {
    printf("|");
    printf(" [%u] (%u,  %u),  (%u) \n", n->com_rank, n->x, n->y,  n->size);
  } else if(n->orientation == VOISIN_H) {
    printf("--");
    printf(" [%u] (%u,  %u),  (%u) \n", n->com_rank, n->x, n->y,  n->size);
  } else {
    printf("invalid neighbour \n");
  }
}

int update_neighbours(list *list, const land*land,   const neighbour *new_n) {
  neighbour temp;
  for(int i = 0; i < list->nb_elem; i++) {
    list_get_index(list,  i,  &temp);
    printf("job ici \n");
    print_neighbour(new_n);
    print_neighbour(&temp);
    if(are_over_neighbour(new_n,  &temp)) {
      update_border(&temp,  new_n);
      if(!is_neighbour(land,  &temp)) {
        printf("nous ne somme plus voisins :'( \n");
      }
      list_replace_index(list,  i,  &temp);
    }
  }
  return 0;
}

int update_border(neighbour *n1,  const neighbour *n2) {
  int ind_max1,  ind_max2,  ind_beg1,  ind_beg2;
  if(n1->orientation != n2->orientation)
    return false;
  if(n1->orientation ==  VOISIN_V) {
    ind_max2 = n2->size + n2->y;
    ind_max1 = n1->size + n1->y;
    ind_beg1 = n1->y;
    ind_beg2 = n2->y;
  } else if(n1->orientation ==  VOISIN_H) {
    ind_max2 = n2->size + n2->x;
    ind_max1 = n1->size + n1->x;
    ind_beg1 = n1->x;
    ind_beg2 = n2->x;
  }
  if(is_over_neighbour(n1,  n2)) {
    if(is_contains_neighbour(n1,  n2)) {
      printf("seems imporssible \n");
    } else if(is_over_neighbour_end(n1,  n2)) {
      n1->size = n1->size - (ind_max1 - ind_beg2);
    } else if(is_over_neighbour_begin(n2,  n1)) {
      n1->size = n1->size - (ind_max2 - ind_beg1);
      n1->x = ind_beg2;
    }
  } else {
    printf("on tombe dans ce cas \n");
  }
  return 1;
}

int is_over_neighbour(const neighbour *n1,  const neighbour *n2) {
  if(n1->orientation != n2->orientation)
    return false;
  return (is_contains_neighbour(n1,  n2) || is_over_neighbour_begin(n1,  n2) || is_over_neighbour_end(n1,  n2));
}

int are_over_neighbour(const neighbour *n1, const neighbour *n2) {
  if(n1->orientation != n2->orientation)
    return false;
  return (is_over_neighbour(n1,  n2) || is_over_neighbour(n2,  n1));
}

// voisin 1 contien voisin 2
// n1 x--------------x
// n2    x-------x
int is_contains_neighbour(const neighbour *n1, const neighbour *n2) {
  int ind_max1,  ind_max2,  ind_beg1,  ind_beg2;
  if(n1->orientation != n2->orientation)
    return false;
  if(n1->orientation ==  VOISIN_V) {
    ind_max2 = n2->size + n2->y;
    ind_max1 = n1->size + n1->y;
    ind_beg1 = n1->y;
    ind_beg2 = n2->y;
  } else if(n1->orientation ==  VOISIN_H) {
    ind_max2 = n2->size + n2->x;
    ind_max1 = n1->size + n1->x;
    ind_beg1 = n1->x;
    ind_beg2 = n2->x;
  }
  return (ind_beg1 < ind_beg2) && (ind_max1 > ind_max2);
}

// return true if
// n1 : x--------------x
// n2 :         x-------------x
int is_over_neighbour_end(const neighbour *n1, const neighbour *n2) {
  int ind_max1,  ind_max2,  ind_beg1,  ind_beg2;
  if(n1->orientation != n2->orientation)
    return false;
  if(n1->orientation ==  VOISIN_V) {
    ind_max2 = n2->size + n2->y;
    ind_max1 = n1->size + n1->y;
    ind_beg1 = n1->y;
    ind_beg2 = n2->y;
  } else if(n1->orientation ==  VOISIN_H) {
    ind_max2 = n2->size + n2->x;
    ind_max1 = n1->size + n1->x;
    ind_beg1 = n1->x;
    ind_beg2 = n2->x;
  }
  return (ind_beg1 <= ind_beg2) && (ind_max2 > ind_max1);
}

// return true if
// n1 :         x--------------x
// n2 : x--------------x
int is_over_neighbour_begin(const neighbour *n1, const neighbour *n2) {
  int ind_max1,  ind_max2,  ind_beg1,  ind_beg2;
  if(n1->orientation != n2->orientation)
    return false;
  if(n1->orientation ==  VOISIN_V) {
    ind_max2 = n2->size + n2->y;
    ind_max1 = n1->size + n1->y;
    ind_beg1 = n1->y;
    ind_beg2 = n2->y;
  } else if(n1->orientation ==  VOISIN_H) {
    ind_max2 = n2->size + n2->x;
    ind_max1 = n1->size + n1->x;
    ind_beg1 = n1->x;
    ind_beg2 = n2->x;
  }
  return (ind_beg1 >= ind_beg2) && (ind_max2 < ind_max1);
}

void free_neighbour_cb(void *n) {
  neighbour elem = *((neighbour *) n);
}

void print_neighbour_cb(void *n) {
  neighbour elem = *((neighbour *) n);
  print_neighbour(&elem);
}


void init_land(land *l, unsigned  int x, unsigned  int y, unsigned int s_x, unsigned int s_y) {
  l->x = x;
  l->y = y;
  l->size_x = s_x;
  l->size_y = s_y;
}

bool is_land_contains(const land *l, unsigned int x, unsigned int y) {
  return ((l->x <= x && l->x + l->size_x > x) && (l->y <= y && l->y + l->size_y > y));
}

bool is_land_contains_pair(const land *l, const pair *p) {
  return ((l->x <= p->x && l->x + l->size_x > p->x) && (l->y <= p->y && l->y + l->size_y > p->y));
}

void split_land(land *new_land, land *old_land) {
  unsigned int new_x,  new_y,  new_size_x,  new_size_y;
  if(old_land->size_x >= old_land->size_y) {
    new_x = old_land->x + (old_land->size_x / 2);
    new_y = old_land->y;
    new_size_y = old_land->size_y;
    new_size_x = old_land->size_x / 2;
    old_land->size_x = new_size_x ;
  } else {
    new_x = old_land->x;
    new_y = old_land->y + (old_land->size_y / 2 );
    new_size_y = (old_land->size_y / 2);
    new_size_x = old_land->size_x;
    old_land->size_y = new_size_y;
  }
  init_land(new_land,  new_x,  new_y,  new_size_x,  new_size_y);
}

void log_factory(FILE *f,  const void *data,  int CODE,  int from) {
  if(from != -1) {
    fprintf(f, " -> %d ",  from);
  }
  land temp_land;
  neighbour temp_neighbour;
  switch(CODE) {
    case LAND_LOG :
      memcpy(&temp_land, data,  sizeof(land));
      fprintf(f,  "((%u,  %u),  (%u,  %u))\n",  temp_land.x,   temp_land.size_x,  temp_land.y,  temp_land.size_y);
      break;
    case NEIGHBOUR_LOG :
      memcpy(&temp_neighbour,  data,  sizeof(neighbour));
      if(temp_neighbour.orientation == VOISIN_V) {
        fprintf(f,  "|");
        fprintf(f,  " [%u] (%u,  %u),  (%u) \n", temp_neighbour.com_rank, temp_neighbour.x, temp_neighbour.y,  temp_neighbour.size);
      } else if(temp_neighbour.orientation == VOISIN_H) {
        fprintf(f, "--");
        fprintf(f, " [%u] (%u,  %u),  (%u) \n", temp_neighbour.com_rank, temp_neighbour.x, temp_neighbour.y,  temp_neighbour.size);
      } else {
        fprintf(f,  "invalid neighbour \n");
      }
      break;
  }
  fflush(f);
}
