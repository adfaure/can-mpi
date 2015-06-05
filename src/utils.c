#include "utils.h"

void get_random_id(pair *p, int min, int max) {
  p->x = rand() % (max - min + 1) + min;
  p->y = rand() % (max - min + 1) + min;
}

void init_pair(pair *p, int x, int y) {
  p->x = x;
  p->y = y;
}

void print_pair(const pair *p) {
  printf("paire : (%d ,%d) \n", p->x, p->y);
}

double entire_dist_betwen_points(int x1, int y1, int x2, int y2) {
    return sqrt(pow((double)(x2 - x1), 2) + pow((double)(y2 - y1),2 ));
}


float **alloc_2d_float(int rows, int cols) {
    float *data = (float *)malloc(rows*cols*sizeof(float));
    float **array= (float **)malloc(rows*sizeof(float*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

int **alloc_2d_int(int rows, int cols) {
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i < rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

long long now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long millisecondsSinceEpoch =
            (unsigned long long)(tv.tv_sec) * 1000 +
            (unsigned long long)(tv.tv_usec) / 1000;
    return millisecondsSinceEpoch;
}


void list_apply(const list *l, void(*cb)(void * data)) {
  cell *current = l->first;
  for(int i = 0 ; i < l->nb_elem; i++) {
    cb(current->data);
    current = current->next;
  }
}

void init_list(list *l, unsigned int element_size) {
  l->nb_elem = 0;
  l->first = NULL;
  l->element_size = element_size;
}

void list_add_front(list * l, void *elem) {
  cell *temp,*new_cell = (cell*) malloc(sizeof(cell));
  new_cell->data = malloc(l->element_size);
  new_cell->next = NULL;
  memcpy(new_cell->data, elem, l->element_size);
  temp = l->first;
  l->first = new_cell;
  new_cell->next = temp;
  l->nb_elem++;
}

int list_get_index(const list *l,int i, void *data) {
  if (i >= l->nb_elem || i < 0 ) return 0;
  cell *current = l->first;
  int acc = 0;
  while(acc != i){
    current = current->next;
    acc++;
  }
  memcpy(data, current->data, l->element_size);
  return 1;
}

int list_replace_index(list * l, int i ,const void *data) {
  if (i >= l->nb_elem || i < 0 ) return 0;
  cell *current = l->first;
  int acc = 0;
  while(acc != i){
    current = current->next;
    acc++;
  }
  free(current->data);
  current->data = malloc(l->element_size);
  memcpy(current->data, data, l->element_size);
  return 1;
}

void list_clear(list *l, void(*free_function)(void *data)) {
  cell *current = l->first, *temp;
  if(l->first == NULL)
    return;
  for(int i = 0 ; i < l->nb_elem; i++) {
    free_function(current->data);
    temp = current;
    current = current->next;
    free(temp);
  }
  l->first = NULL;
  l->nb_elem = 0;
}
