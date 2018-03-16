#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "linked_list.h"

#define EVENTS_LIST 1000000

// type 1 - ARRIVAL | type 2 - DEPARTURE
#define ARRIVAL 1
#define DEPARTURE 2

#define lambda 0.2 // Arrival rate of each free customer
#define dm 2 // Average service time in minutes
#define m 8 // Number of concurrent service channels / servers
#define L 4 // Number of system buffer positions
#define K 20 // Population size of potential clients

int saveInCSV(char* filename, int* histogram, int hist_size) {
  FILE *CSV;
  int i;
  fprintf(stderr, "Saving the histogram in the %s\n", filename);

  CSV = fopen(filename,"w+");
  if(CSV == NULL) {
    perror("fopen");
    return -1;
  }
  fprintf(CSV, "Indice, Tempo Central, Número de Chegadas (total %d)\n", EVENTS_LIST);
  for (i = 0; i < hist_size; i++) {
    fprintf(CSV, "%d, %lf, %d\n", i, (2*i+1)/(float)(hist_size*2), histogram[i]);
  }

  return 0;
}

double calc_time(double lt, int type) {
  double u, C, S;

  u = (rand()%RAND_MAX + 1) / (double)RAND_MAX;

  if (type == ARRIVAL) {
    // mean time between consecutive arrivals
    C = -(log(u)/(float)lt);
    return C;

  } else {
    // service time
    S = -(log(u)*(float)dm);
    return S;
  }
}

list * add_new_event(double t, float lt, int type, list* event_list){
  double event_time = 0;
  event_time = t + calc_time(lt, type);
  event_list = add(event_list, type, event_time);
  return event_list;
}

int main(int argc, char* argv[]) {

   double lambda_total, depart_time = 0, aux_time = 0, delay_time = 0;
   int i, buffer_size = L, channels = 0, arrival_events = 0, buffer_events = 0, delay_count = 0, losses = 0;
   //int hist_size = 1000, *histogram;
   //char filename[50];
   list *events = NULL, *buffer = NULL;

   srand(time(NULL));

   /*if (!strcpy(filename, argv[1])){
     perror("strcpy");
     return -1;
   }*/

   //histogram = (int*)calloc(hist_size, sizeof(int));

   events = add(events, ARRIVAL, 0);

  // We process the list of events
  for(i = 1; i < EVENTS_LIST; i++) {

    if (events->type == ARRIVAL) { // Arrival event

      arrival_events++;  // counting the arrival events
      lambda_total = (K - channels - (L-buffer_size)) * lambda; // arrival rate of the system
      aux_time = events->_time;

      if (channels < m) { // if the channels aren't full
        events = add_new_event(events->_time, lambda_total, DEPARTURE, events);
        channels++;

        //print_elems(events);
        //fprintf(stderr, "//// Channels: %d ////\n", channels);

      } else if (buffer_size > 0) { // if the buffer has space
          buffer = add(buffer, ARRIVAL, events->_time);
          buffer_size--;
          buffer_events++;

          //fprint_elems(buffer);
          //fprintf(stderr, "ºººº Event added in the BUFFER: %d ºººº\n", L - buffer_size);

      } else { // if the channels and buffer are occupied we get a lost event
        losses++;
        //fprintf(stderr, "\t... Client lost: %d ...\n", losses);
      }
      // substitute the actual event to a new arrival event
      events = rem(events);
      events = add_new_event(aux_time, lambda_total, ARRIVAL, events);

    } else { // Departure event
      aux_time = events->_time;
      events = rem(events);
      //fprintf(stderr, "\t<<< Channels after departure: %d >>>\n", channels);
      if (buffer != NULL) {
        depart_time = aux_time + calc_time(lambda_total, DEPARTURE);
        events = add(events, DEPARTURE, depart_time);

        delay_time += aux_time - buffer->_time;
        delay_count++;

        buffer = rem(buffer);
        buffer_size++;
      } else {
        channels--;
      }
    }
  }

  /*
  index = (arrival_time-old_time)/(1 / (float)(HIST_JUMP*lambda));
  histogram[index > (hist_size - 1) ? hist_size - 1 : index]++;

  if(saveInCSV(filename, histogram, hist_size) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stderr, "File %s saved successfully\n\n", filename);

  */

  fprintf(stdout, "\nProbability of blocking (loss = %d) of customers (total arrivals = %d): B = %.3f %%\n", losses, arrival_events, losses / (float)arrival_events * 100);
  fprintf(stdout, "\nProbability of customer service delay (buffer_events = %d): Pa = %.3f %%\n", buffer_events, buffer_events / (float)arrival_events * 100);
  fprintf(stdout, "\nAverage customer service delay (delay_time = %.3f): Am = %.3f min = %.3f sec\n", delay_time, delay_time / (float)(arrival_events), delay_time / (float)(arrival_events) * 60);
  //printf("\nEnter the value of seconds (t) to compute the Pa under certain value: ");
  //scanf("%d", t);

  return 0;
}
