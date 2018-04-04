#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include "linked_list.h"

#define EVENTS_LIST 1000000
#define LAMBDA 10 // Calls per minute (600/hour)
#define L 1000 // Finite buffer
#define DM 1.5 // Mean time
#define DM_TRANSF 0.75
#define M 8
#define K 20000 // Population size of potential clients
// type 1 - ARRIVAL | type 2 - DEPARTURE | type 3 - EMERGENCY
#define ARRIVAL 1
#define DEPARTURE 2
#define EMERGENCY 3

/*
void collectData (double* p_lambda, double* p_dm, int* p_m, int* p_L, int* p_K) {
  fprintf(stdout, "\n\tLet's begin the Simulation of a waitiling list!\n\n");
  fprintf(stdout, "\tBut first I need some data to calculate the results. So please tell me the values of:\n");
  fprintf(stdout, "\t!!! Attention, in decimal numbers please use dot ('.') instead of commas (',') !!!\n");

  fprintf(stdout, "\n\tCalls per hour (lambda): ");
  scanf("%lf", p_lambda);

  fprintf(stdout, "\n\tAverage service time per minutes (dm): ");
  scanf("%lf", p_dm);

  fprintf(stdout, "\n\tNumber of concurrent service channels (m): ");
  scanf("%d", p_m);

  fprintf(stdout, "\n\tNumber of system buffer positions (L): ");
  scanf("%d", p_L);

  fprintf(stdout, "\n\tPopulation size of potential clients (K): ");
  scanf("%d", p_K);

  fprintf(stdout, "\n\tOk, that's it! So here are the results:\n");
}

int saveInCSV(char* filename, int* histogram, int hist_size) {
  FILE *CSV;
  int i;
  fprintf(stdout, "\n\tSaving the histogram in \"%s\"\n", filename);

  CSV = fopen(filename,"w+");
  if(CSV == NULL) {
    perror("fopen");
    return -1;
  }
  fprintf(CSV, "Index, Delay\n");
  for (i = 0; i < hist_size; i++) {
    fprintf(CSV, "%d, %d\n", i, histogram[i]);
  }
  fclose(CSV);

  return 0;
}
*/

double calcTime(double lt, int type) {
  double u, C, S;

  u = (rand()%RAND_MAX + 1) / (double)RAND_MAX;

  if (type == ARRIVAL) {
    // mean time between consecutive arrivals
    C = -(log(u)/(double)lt);
    return C;

  } else if (type == DEPARTURE) {
    // service time
    S = -(log(u)*DM);
    return S;
  }



}

list * addNewEvent(double t, double lt, int type, list* event_list){
  double event_time = 0;
  event_time = t + calcTime(lt, type);
  event_list = add(event_list, type, event_time);
  return event_list;
}

void calcDelayProb(double t, int tot_delay, int* hist, int h_size) {
  int num_delay = 0;
  for (int i = 0; i < (int)t; i++) {
    num_delay += hist[i];
  }
  printf("\tDelay: %d\n", num_delay);
  printf("\tTotal delay: %d\n", tot_delay);
  fprintf(stdout, "\tThe probability is Pa(a<%.3f) = %.3f%%\n", t, ((float)num_delay) / tot_delay * 100);

}

// Function to tell the user the mean time he will have to wait based on system load
void tellDelay(float tot_delay, int arrivals) {
  printf("Delay: %.3f\n", tot_delay);
  printf("Arrival events: %d\n", arrivals);
  printf("Mean time of delay predicted: %.3f seconds\n", tot_delay / (float)(arrivals) * 60);
}

// Function that generates a random event that could be an emergency or not (60% prob of emeregency)
int arrivalOrEmergency () {
  srand(time(NULL));
  if ((rand() % 100) < 60) return EMERGENCY;
  else return ARRIVAL;
}

int main(int argc, char* argv[]) {
  int index = 0, emergency = 0, arrival_events = 0;
  double lambda_total, aux_time = 0, t = -1, delay_time = 0;;
  int i, channels = 0, buffer_size = L, buffer_events = 0, delay_count = 0, losses = 0, channels_inem = 0;
  int *histogram, hist_size = 0, aux = 0;
  char filename[50];
  list *events = NULL, *events_inem = NULL, *buffer = NULL, *buffer_inem = NULL;

  srand(time(NULL));

  strcat(filename, argv[1]);

  histogram = (int*)calloc(1, sizeof(int));

  events = add(events, arrivalOrEmergency(), 0);

  // We process the list of events
  for(i = 1; i < EVENTS_LIST; i++) {
    aux_time = events->_time;
    if (events->type == ARRIVAL) { // Arrival event
      arrival_events++;  // counting the arrival events
      lambda_total = (K - channels - (L-buffer_size)) * LAMBDA; // arrival rate of the system

      // If isn't an emergency, call is processed by PC
      if (channels < M) { // if the channels aren't full
        events = addNewEvent(events->_time, lambda_total, DEPARTURE, events);
        channels++; // A channel serves the call
      } else if (buffer_size > 0) { // if the buffer has space
          buffer = add(buffer, ARRIVAL, events->_time);
          buffer_size--;
          buffer_events++;
          tellDelay(delay_time, arrival_events);
      } else { // if the channels and buffer are occupied we get a lost event
        losses++;
      }
      // Remove the actual arrival event and add a new one
      events = rem(events);
      events = addNewEvent(aux_time, lambda_total, arrivalOrEmergency(), events);

    } else if (events->type == DEPARTURE) { // Departure event
      events = rem(events);
      if (buffer != NULL) { // As a channel is free now it can serve an event waiting in the buffer
        events = addNewEvent(aux_time, lambda_total, DEPARTURE, events);

        delay_time += aux_time - buffer->_time;
        index = (int)((aux_time - buffer->_time)*60);
        if (index + 1 > hist_size) {
          aux = hist_size;
          hist_size = index + 1;
          histogram = (int*)realloc(histogram, hist_size * sizeof(int));
          for (i = aux; i < hist_size; i++) {
            histogram[i] = 0;
          }
          if (histogram == NULL) {
            perror("realloc");
            return -1;
          }
        }
        histogram[index]++;
        delay_count++;

        buffer = rem(buffer);
        buffer_size++;
      } else { // If the buffer is empty
        channels--;
      }

    } else { // It's an emergency
      arrival_events++;  // counting the arrival events
      if (channels_inem < M) { // If the channels aren't full
        events_inem = addNewEvent(events->_time, lambda_total, DEPARTURE, events_inem);
        channels_inem--;
        events = rem(events);
      } else {
        buffer_inem = add(buffer_inem, ARRIVAL, events->_time);
      }
      
      /* verify buffer_inem and send that call to INEM */

      // The arrival of that call originates a new arrival event
      events = addNewEvent(aux_time, lambda_total, arrivalOrEmergency(), events);
    }
  }
/*
  if(saveInCSV(filename, histogram, hist_size) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stdout, "\tFile \"%s\" saved successfully\n\n", filename);
*/

  fprintf(stdout, "\nProbability of blocking (losses = %d) of customers (total arrivals = %d): B = %.3f%%\n", losses, arrival_events, losses / (float)arrival_events * 100);
  /*fprintf(stdout, "\nProbability of customer service delay (buffer events = %d): Pa = %.3f%%\n", buffer_events, buffer_events / (float)arrival_events * 100);
  fprintf(stdout, "\nAverage customer service delay: Am = %.3fmin = %.3fsec\n\n", delay_time / (float)arrival_events, delay_time / (float)(arrival_events) * 60);

  free(histogram);
*/

  return 0;
}
