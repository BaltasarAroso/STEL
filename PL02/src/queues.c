#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include "linked_list.h"

#define EVENTS_LIST 1000000

// type 1 - ARRIVAL | type 2 - DEPARTURE
#define ARRIVAL 1
#define DEPARTURE 2
#define CSVFolder "DotCSV/"

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

double calcTime(double lt, int type, double dm) {
  double u, C, S;

  u = (rand()%RAND_MAX + 1) / (double)RAND_MAX;

  if (type == ARRIVAL) {
    // mean time between consecutive arrivals
    C = -(log(u)/(double)lt);
    return C;

  } else {
    // service time
    S = -(log(u)*dm);
    return S;
  }
}

list * addNewEvent(double t, double lt, int type, list* event_list, double dm){
  double event_time = 0;
  event_time = t + calcTime(lt, type, dm);
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

int main(int argc, char* argv[]) {
  double lambda = 0, dm = 0;
  int m = 0, L = 0, K = 0, index = 0;

  collectData(&lambda, &dm, &m, &L, &K);
  lambda = lambda / 60; // passing calls per hour to calls per minutes

  double lambda_total, aux_time = 0, delay_time = 0, t = -1;
  int i, buffer_size = L, channels = 0, arrival_events = 0, buffer_events = 0, delay_count = 0, losses = 0;
  int *histogram, hist_size = 0;
  char filename[50];
  list *events = NULL, *buffer = NULL;

  srand(time(NULL));

  if (stat(CSVFolder, NULL) == -1) {
   mkdir(CSVFolder, 0777);
  }

  if (!strcpy(filename, CSVFolder)){
   perror("strcpy");
   return -1;
  }

  strcat(filename, argv[1]);

  histogram = (int*)calloc(1, sizeof(int));

  events = add(events, ARRIVAL, 0);

  // We process the list of events
  for(i = 1; i < EVENTS_LIST; i++) {
    aux_time = events->_time;
    if (events->type == ARRIVAL) { // Arrival event
      arrival_events++;  // counting the arrival events
      lambda_total = (K - channels - (L-buffer_size)) * lambda; // arrival rate of the system

      if (channels < m) { // if the channels aren't full
        events = addNewEvent(events->_time, lambda_total, DEPARTURE, events, dm);
        channels++; // A channel serves the call
      } else if (buffer_size > 0) { // if the buffer has space
          buffer = add(buffer, ARRIVAL, events->_time);
          buffer_size--;
          buffer_events++;
      } else { // if the channels and buffer are occupied we get a lost event
        losses++;
      }
      // Remove the actual arrival event and add a new one
      events = rem(events);
      events = addNewEvent(aux_time, lambda_total, ARRIVAL, events, dm);

    } else { // Departure event
      events = rem(events);
      if (buffer != NULL) { // As a channel is free now it can serve an event waiting in the buffer
        events = addNewEvent(aux_time, lambda_total, DEPARTURE, events, dm);

        delay_time += aux_time - buffer->_time;
        index = (int)((aux_time - buffer->_time)*60);
        if (index + 1 > hist_size) {
          hist_size = index + 1;
          histogram = (int*)realloc(histogram, hist_size * sizeof(int));
          for (i = 0; i < hist_size; i++) {histogram[i] = 0;}
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
    }
  }

  hist_size++;
  fprintf(stderr, "index = %d | hist_size = %d\n\n", index, hist_size);
  if(saveInCSV(filename, histogram, hist_size) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stdout, "\tFile \"%s\" saved successfully\n\n", filename);

  fprintf(stdout, "\nProbability of blocking (losses = %d) of customers (total arrivals = %d): B = %.3f%%\n", losses, arrival_events, losses / (float)arrival_events * 100);
  fprintf(stdout, "\nProbability of customer service delay (buffer events = %d): Pa = %.3f%%\n", buffer_events, buffer_events / (float)arrival_events * 100);
  fprintf(stdout, "\nAverage customer service delay: Am = %.3fmin = %.3fsec\n\n", delay_time / (float)arrival_events, delay_time / (float)(arrival_events) * 60);

  while(1){
    fprintf(stdout, "\nPlease enter the value of seconds (t) to compute the probability of the delay being less than t seconds Pa(a<t) or '0' to end: ");
    scanf("%lf", &t);
    if (t == 0) {
      break;
    }
    calcDelayProb(t, delay_count, histogram, hist_size);
  }

  free(histogram);

  return 0;
}
