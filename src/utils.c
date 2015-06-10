#include "utils.h"

#define UNUSED(x) (void)(x)

void get_random_id(pair *p, int max_x, int max_y) {
    p->x = rand() % max_x;
    p->y = rand() % max_y;
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

void list_add_front(list * l, const void *elem) {
    cell *temp,*new_cell = (cell*) malloc(sizeof(cell));
    new_cell->data = malloc(l->element_size);
    new_cell->next = NULL;
    memcpy(new_cell->data, elem, l->element_size);
    temp = l->first;
    l->first = new_cell;
    new_cell->next = temp;
    l->nb_elem++;
}

int list_get_index(const list *l, int i, void *data) {
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

int list_replace_index(list * l, int i, const void *data) {
    if (i >= l->nb_elem || i < 0) return 0;
    cell *current = l->first;
    int acc = 0;
    while(acc != i) {
        current = current->next;
        acc++;
    }
    free(current->data);
    current->data = malloc(l->element_size);
    memcpy(current->data, data, l->element_size);
    return 1;
}

int list_remove_index(list * l, int i, void(*free_function)(void *data)) {
    if (i >= l->nb_elem || i < 0) return 0;
    if(i == 0 ) return list_remove_front(l, free_function);
    int idx = 0;
    cell *current = l->first, *back_up = NULL;
    while(i != idx) {
        back_up =  current;
        current = current->next;
        idx++;
    }
    back_up->next = current->next;
    free_function(current->data);
    free(current);
    l->nb_elem -= 1;
    return 1;
}

int list_remove_front(list *l, void(*free_function)(void *data)) {
    if(l->nb_elem == 0) return 0;
    cell *temp = NULL;
    temp = l->first;
    l->first = temp->next;
    l->nb_elem--;
    free_function(temp->data);
    free(temp);
    return 1;
}

int list_cp_revert(const list *src, void(*free_function)(void *data), list *dst) {
    UNUSED(free_function);

    void *temp = malloc(sizeof(src->element_size));
    init_list(dst,src->element_size);
    for(int i = 0; i < src->nb_elem ; i++) {
        list_get_index(src, i, temp);
        list_add_front(dst, temp);
    }
    free(temp);
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

void init_land_storage(land_storage *ls, unsigned int size_x,unsigned int size_y) {
	ls->size_x = size_x;
	ls->size_y = size_y;
	ls->data = malloc(sizeof(can_data*) * size_x);
	for(unsigned int i = 0; i < size_x; i++) {
		ls->data[i] = NULL;
	}
}

int land_storage_store_value(land_storage *ls, unsigned int x, unsigned int y,const can_data* data) {
	if(x >= ls->size_x || y >= ls->size_y) return 0;

	if(ls->data[x] == NULL) {
		ls->data[x] = malloc(sizeof(can_data*) * ls->size_y);
		for(unsigned int  i = 0 ; i < ls->size_y; i++) {
			ls->data[x][i] = NULL;
		}
	}

	ls->data[x][y] = malloc(sizeof(can_data));
	memcpy(ls->data[x][y], data, sizeof(can_data));
	return 1;
}

int land_storage_fetch_data(const land_storage *ls, unsigned int x, unsigned int y, can_data *data) {
	if(ls->data[x] == NULL || ls->data[x][y] == NULL) {
		fprintf(stderr, "data does not exist yet \n");
		return 0;
	}
	memcpy(data, ls->data[x][y], sizeof(can_data));
	return 1;
}

void init_data(can_data *data, unsigned int data_size, unsigned int data_type,const void *elem) {
	data->data_type = data_type;
	data->element_size = data_size;
	data->data = malloc(data_size);
	memcpy(data->data, elem, data_size);
}

void free_can_data_(can_data *data) {
	data->data_type = 0;
	data->element_size = 0;
	free(data->data);
}

void can_data_get_element(const can_data *c_data, void *elem) {
	memcpy(elem, c_data->data, c_data->element_size);
}

void print_data(const can_data *data) {
	printf(" data -> type : %d , size : %d , ptr value : %p",
			data->data_type, data->element_size, data->data);
}
