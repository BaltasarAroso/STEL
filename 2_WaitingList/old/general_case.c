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

#define lambda 0.00015 // Arrival rate of each free customer
#define dm 2 // Average service time in minutes
#define m 8 // Number of concurrent service channels / servers
#define L 1000 // Number of system buffer positions
#define K 20000 // Population size of potential clients

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

float calc_time(double lt, int type) {
  float u, C, S;

  u = (float)(rand()+1)/RAND_MAX;

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

int main(int argc, char* argv[]) {

   float lambda_total, arrival_time = 0, depart_time = 0, aux_time = 0, delay_time = 0;
   int i, buffer_size = L, channels = 0, arrival_events = 0, buffer_events = 0, delay_count = 0, losses = 0;
   //int hist_size = 1000, *histogram;
   //char filename[50];
   list *events = NULL, *buffer = NULL;

   srand(time(NULL)); // initialization of rand

   //lambda_total = (K - L) * lambda; // arrival rate of the system

   // name of the file .csv written after the ./<exe> file and where the data will be written
   /*if (!strcpy(filename, argv[1])){
     perror("strcpy");
     return -1;
   }*/

   // allocate memory dynamically with the histogram size (hist_size)
   // initialize with 0 all the histogram elements
   // histogram = (int*)calloc(hist_size, sizeof(int));

   // we add the first arrival to the list of events
   events = add(events, ARRIVAL, 0);

  // We process the list of events
  for(i = 1; i < EVENTS_LIST; i++) {

    if (events->type == ARRIVAL) {

      arrival_events++;  // counting the arrival events
      lambda_total = (K - channels - (L-buffer_size)) * lambda; // arrival rate of the system

      if (channels < m) { // if the channels aren't full
        // The arrival event passes to an departure event
        depart_time = events->_time + calc_time(lambda_total, DEPARTURE);
        // and another arrival event is generated (in order to the system keep working)
        arrival_time = events->_time + calc_time(lambda_total, ARRIVAL);
        //print_elems(events);
        // So we remove the event from the arrival events
        events = rem(events);
        // and we set it as a departure event
        events = add(events, DEPARTURE, depart_time);
        channels++; // This event is served by one channel available
        // and then we add the new arrival event
        events = add(events, ARRIVAL, arrival_time); // The new arrival event

        print_elems(events);
        printf("//// Channels: %d ////\n", channels);

      } else if (buffer_size > 0) { // if the buffer has space
        //arrival_time = events->_time + calc_time(lambda_total, ARRIVAL);
        // transform that event in a buffer event
        buffer = add(buffer, ARRIVAL, events->_time);
        // so the buffer releases one position
        buffer_size--;
        // count the number of events that passed through the buffer (for Pa)
        buffer_events++;

        arrival_time = events->_time + calc_time(lambda_total, ARRIVAL);
        // we remove the event from the arrival events
        events = rem(events); // That call is rejected
        // and we add a new arrival event with the current time
        events = add(events, ARRIVAL, arrival_time);
        //print_elems(buffer);
        printf("ºººº Event added in the BUFFER: %d ºººº\n", L - buffer_size);

      } else { // if the channels and buffer are occupied we get a lost event
        losses++;
        // But we need to atualize the time and in order that
        arrival_time = events->_time + calc_time(lambda_total, ARRIVAL);
        // we remove the event from the arrival events
        events = rem(events); // That call is rejected
        // and we add a new arrival event with the current time
        events = add(events, ARRIVAL, arrival_time);
        printf("\t... Client lost: %d ...\n", losses);
      }

    } else { // Departure event
      aux_time = events->_time;
      events = rem(events);
      channels--; // The channel that served that client is now free
      printf("\t<<< Channels after departure: %d >>>\n", channels);
      // Add the events that was waiting in the buffer into the channel now available
      if (buffer != NULL) {
        depart_time = aux_time + calc_time(lambda_total, DEPARTURE);
        // Sum the time between arrival to buffer and being served
        delay_time += depart_time - aux_time;
        delay_count++;
        // Remove it from the buffer
        buffer = rem(buffer);
        buffer_size++;
        // Add the event as a departure event
        events = add(events, DEPARTURE, depart_time);
        channels++; // The channel has been occupied
      }
    }
  }

  /*
  // find the index where the corresponding value should be counted
  index = (arrival_time-old_time)/(1 / (float)(HIST_JUMP*lambda));

  // if the C value exceeds the max value of the index, put it in the last index
  histogram[index > (hist_size - 1) ? hist_size - 1 : index]++;

  // save histogram values in the file.csv
  if(saveInCSV(filename, histogram, hist_size) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stderr, "File %s saved successfully\n\n", filename);

  */

  /*printf("Time of arrival: %f\n", events->_time);
  events = rem(events);
  printf("Time of 2 arrival: %f\n", events->_time);*/

  // Computes the probability of blocking (loss) of customers (B)
  printf("\nProbability of blocking (loss = %d) of customers (total arrivals = %d): B = %.3f\n", losses, arrival_events, losses / (float)arrival_events);
  printf("\nProbability of customer service delay (buffer_events = %d): Pa = %.3f\n", buffer_events, buffer_events / (float)arrival_events);
  printf("\nAverage customer service delay (delay_time = %.3f): Am = %.3f\n", delay_time, delay_time / (float)(delay_count));
  //printf("\nEnter the value of seconds (t) to compute the Pa under certain value: ");
  //scanf("%d", t);

  return 0;
}
