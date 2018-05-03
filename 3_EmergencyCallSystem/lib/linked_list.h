#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// Structure list definition
typedef struct{
	int type;
	double _time;
	double _predicted_time;
	struct list * next;
} list;

list * rem (list * pointer);
list * add (list * pointer, int n_type, double n_time, double n_predicted_time);
void print_elems (list * pointer);

#endif
