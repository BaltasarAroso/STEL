#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// Structure list definition
typedef struct{
	int type;
	int buffer_positions;
	double _time;
	double _arrival_time;
	struct list * next;
} list;

list * rem (list * pointer);
list * add (list * pointer, int n_type, double n_time, double n_arrival_time,  int buffer_positions);
void print_elems (list * pointer);

#endif
