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
list * add (list * pointer, int n_type, double n_time, /*double n_arrival_time,*/ double n_predicted_time){
	list * lap = pointer;
	list * ap_aux, * ap_next;
	if(pointer == NULL) {
		pointer = (list *) malloc(sizeof (list));
		pointer -> next = NULL;
		pointer -> type = n_type;
		pointer -> _time = n_time;
		//pointer -> _arrival_time = n_arrival_time;
		pointer -> _predicted_time = n_predicted_time;
		return pointer;
	} else {
		if (pointer->_time > n_time) {
	        ap_aux = (list *) malloc(sizeof (list));
	        ap_aux -> type = n_type;
          ap_aux -> _time = n_time;
					//ap_aux -> _arrival_time = n_arrival_time;
					ap_aux -> _predicted_time = n_predicted_time;
          ap_aux -> next = (struct list *) pointer;
          return ap_aux;
	  }

		ap_next = (list *)pointer -> next;
		while(pointer != NULL) {
			if((ap_next == NULL) || ((ap_next -> _time) > n_time)) {
				break;
			}
			pointer = (list *)pointer -> next;
			ap_next = (list *)pointer -> next;
		}
		ap_aux = (list *)pointer -> next;
		pointer -> next = (struct list *) malloc(sizeof (list));
		pointer = (list *)pointer -> next;
		if(ap_aux != NULL) {
			pointer -> next = (struct list *)ap_aux;
		}else{
			pointer -> next = NULL;
		}
		pointer -> type = n_type;
		pointer -> _time = n_time;
		//pointer -> _arrival_time = n_arrival_time;
		pointer -> _predicted_time = n_predicted_time;
		return lap;
	}
}

// Function that print in the terminal all the list elements
void print_elems (list * pointer){
	if(pointer == NULL) {
		printf("empty list!\n");
	} else {
		while(pointer != NULL) {
			printf("type=%d\t_time=%lf\n", pointer -> type, pointer -> _time);
			pointer = (list *)pointer -> next;
		}
	}
}
