#ifndef LINKED_LIST_H
#define LINKED_LIST_H

// Structure list definition
typedef struct{
	int type;
	double _time;
	double _arrival_time; // time scheduled when the event arrives into the PC
	double _predicted_time; // predicted time that the event spends in the PC buffer
	int buffer_elements; // number of elements that are in the PC buffer when this event arrives (excluding himself)
	struct list * next;
} list;

list * rem (list * pointer);
list * add (list * pointer, int n_type, double n_time, double n_arrival_time, double n_predicted_time, int n_buffer_elements);
void print_elems (list * pointer);

#endif
