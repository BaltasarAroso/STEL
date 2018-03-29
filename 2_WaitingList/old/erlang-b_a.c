#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "linked_list.h"

#define ARRIVALS 100000

// type 1 - ARRIVAL | type 2 - DEPARTURE
#define ARRIVAL 1
#define DEPARTURE 2

#define lambda 0.00015 // Arrival rate of each free customer
#define dm 2 // Average service time in minutes
#define m 8 // Number of concurrent service channels / servers
#define L 0 // Number of system buffer positions
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
  fprintf(CSV, "Indice, Tempo Central, Número de Chegadas (total %d)\n", ARRIVALS);
  for (i = 0; i < hist_size; i++) {
    fprintf(CSV, "%d, %lf, %d\n", i, (2*i+1)/(float)(hist_size*2), histogram[i]);
  }

  return 0;
}

float calc_mean_time(double lt, int type) {
  float u, C, S;

  u = (float)(rand()+1)/RAND_MAX; // uniformal distribuition

  if (type == ARRIVAL) {
    // mean time between consecutive arrivals
    C = -(log(u)/(float)lt);
    //printf("*****Arrival time: %f\n",C);
    return C;

  } else {
    // service time
    S = -(log(u)*(float)dm);
    //printf("----Departure time: %f\n", S);
    return S;
  }
}

int main(int argc, char* argv[]) {

   float lambda_total, curr_time = 0, depart_time;
   int i, buffer_size = L, channels = 0, arrival_event = 0, losses = 0;
   //int hist_size = 1000, *histogram;
   //char filename[50];
   list *events = NULL;

   srand(time(NULL)); // initialization of rand
   lambda_total = (K-L)*lambda; // arrival rate of the system

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
  for(i=0; i < ARRIVALS; i++) {

    if (events->type == ARRIVAL) {
      if (channels < m) { // if the channels aren't full
        // The arrival event passes to an departure event
        depart_time = events->_time + calc_mean_time(lambda_total, DEPARTURE);
        // and another arrival event is generated (in order to the system keep working)
        curr_time = events->_time + calc_mean_time(lambda_total, ARRIVAL);

        // So we remove the event from the arrival events
        events = rem(events);
        // and we set it as a departure event
        events = add(events, DEPARTURE, depart_time);
        channels++; // This event is served by one channel available
        // and then we add the new arrival event
        events = add(events, ARRIVAL, curr_time); // The new arrival event

        //print_elems(events);
        //printf("//// Channels: %d ////\n", channels);
        arrival_event++;  // counting the arrival events

      } else if (buffer_size > 0) { // if the buffer have events on it
        // We calculate the time
        curr_time += calc_mean_time(lambda_total, ARRIVAL);
        // and transform that event on the buffer in an arrival event
        events = add(events, 0, curr_time);
        // so the buffer releases one position
        buffer_size--;

      } else { // if the channels and buffer are occupied we get a lost event
        losses++;
        // But we need to atualize the time and in order that
        curr_time = events->_time + calc_mean_time(lambda_total, ARRIVAL);
        // we remove the event from the arrival events
        events = rem(events); // That call is rejected
        // and we add a new arrival event with the current time
        events = add(events, ARRIVAL, curr_time);
        //printf("\t... Client lost: %d ...\n", losses);
        arrival_event++;  // counting the arrival events
      }
    } else { // Departure event
      events = rem(events);
      channels--; // The channel that served that client is now free
      //printf("\t<<< Channels after departure: %d >>>\n", channels);
    }
  }

  /*
  // find the index where the corresponding value should be counted
  index = (curr_time-old_time)/(1 / (float)(HIST_JUMP*lambda));

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
  printf("\nProbability of blocking (loss = %d) of customers (total arrivals = %d): B = %.3f\n", losses, arrival_event, (float)losses / (float)arrival_event);
  //printf("\nProbability of customer service delay: Pa = %.3f\n", );
  //printf("\nAverage customer service delay: Am = %.3f\n", );
  //printf("\nEnter the value of seconds (t) to compute the Pa under certain value: ");
  //scanf("%d", t);

  return 0;
}
