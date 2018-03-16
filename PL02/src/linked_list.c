#include<stdio.h>
#include<stdlib.h>
#include "linked_list.h"

// Function that rems the first element of the list
list * rem (list * pointer){
	list * lap = (list *)pointer -> next;
	free(pointer);
	return lap;
}

// Function that add the new element in the list, ordered by _time
list * add (list * pointer, int n_type, double n_time){
	list * lap = pointer;
	list * ap_aux, * ap_next;
	if(pointer == NULL)
	{
		pointer = (list *) malloc(sizeof (list));
		pointer -> next = NULL;
		pointer -> type = n_type;
		pointer -> _time = n_time;
		return pointer;
	}
	else
	{
		if (pointer->_time > n_time) {
	        ap_aux = (list *) malloc(sizeof (list));
	        ap_aux -> type = n_type;
          ap_aux -> _time = n_time;
          ap_aux -> next = (struct list *) pointer;
          return ap_aux;
	    }

		ap_next = (list *)pointer -> next;
		while(pointer != NULL)
		{
			if((ap_next == NULL) || ((ap_next -> _time) > n_time))
				break;
			pointer = (list *)pointer -> next;
			ap_next = (list *)pointer -> next;
		}
		ap_aux = (list *)pointer -> next;
		pointer -> next = (struct list *) malloc(sizeof (list));
		pointer = (list *)pointer -> next;
		if(ap_aux != NULL)
			pointer -> next = (struct list *)ap_aux;
		else
			pointer -> next = NULL;
		pointer -> type = n_type;
		pointer -> _time = n_time;
		return lap;
	}
}

// Function that print in the terminal all the list elements
void print_elems (list * pointer){
	if(pointer == NULL)
		printf("empty list!\n");
	else
	{
		while(pointer != NULL)
		{
			printf("type=%d\t_time=%lf\n", pointer -> type, pointer -> _time);
			pointer = (list *)pointer -> next;
		}
	}
}

// Little example of this library utility
/*int main(void) {
	list  * list_events;
	int type_ev; double _time_ev;
	list_events = NULL;
	list_events = add(list_events, 1, 0.6);
	list_events = add(list_events, 0, 0.4);
	list_events = add(list_events, 1, 0.3);
	list_events = add(list_events, 2, 0.5);
	list_events = add(list_events, 1, 0.5);
	list_events = add(list_events, 0, 0.2);
	list_events = add(list_events, 1, 0.1);
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	if ( list_events != NULL)
	{
		type_ev = list_events -> type;
		_time_ev = list_events -> _time;
		list_events = rem(list_events);
		printf("\nEVENTO REMOVIDO\n");
		printf("type=%d\t_time=%lf\n", type_ev, _time_ev);
	}
	printf("\nlist ACTUAL\n");
	print_elems(list_events);

	list_events = add(list_events, 2, 0.5);
	list_events = add(list_events, 1, 0.3);
	printf("\nlist ACTUAL\n");
	print_elems(list_events);
}
*/
