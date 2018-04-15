#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include "lib/linked_list.h"

#define EVENTS_LIST 1000000
#define LAMBDA 10 // Calls per minute (600/hour)
#define L 1000 // Finite buffer size of PC
#define DM 1.5 // Mean time
#define DM_TRANSF 0.75
#define M 8 // service positions in PC and INEM
#define K 20000 // Population size of potential clients

#define ARRIVAL 1
#define DEPARTURE 2
#define EMERGENCY 3

/*
void collectData (double* p_lambda, double* p_dm, int* p_m, int* p_L, int* p_K) {
  fprintf(stdout, "\n\tLet's begin the Simulation of an emergency call system!\n\n");
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
  double u, C, S, E;
  u = (rand()%RAND_MAX + 1) / (double)RAND_MAX;

  if (type == ARRIVAL) {
    // mean time between consecutive arrivals
    C = -(log(u)/(double)lt);
    return C;
  } else if (type == DEPARTURE) {
    // service time
    S = 1-(log(u)*DM);
    if (S < 60) {
      calcTime(lt, type);
    }
    if (S > 4*60) {
      calcTime(lt, type);
    }
    return S;
  }
  E = -(log(u)*DM_TRANSF);
  if (E < 35) {
    calcTime(lt, type);
  }
  if (E > 75) {
    calcTime(lt, type);
  }
  return E;

  /* Box-Muller */
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;

  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }

  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = U1*U1 + U2*U2;
    }
  while (W >= 1 || W == 0);

  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;

  call = !call;

  return (mu + sigma * (double) X1);
  /**************/
  
}

list * addNewEvent(double t, double lt, int type, list* event_list) {
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
  // useful variables
  int i = 0, index = 0, losses = 0;
  int *histogram, hist_size = 0, aux_hist = 0;
  double lambda_total = 0, aux_time = 0, delay_time = 0;
  char filename[50];

  // PC variables
  int arrival_events = 0, channels = 0, buffer_size = L, buffer_events = 0;
  list *events = NULL, *buffer = NULL;

  // INEM variables
  int inem_events = 0, channels_inem = M, buffer_inem_size = 0, losses_inem = 0;
  list *events_inem = NULL, *buffer_inem = NULL;

  //int emergency = 0, delay_count = 0;;
  //double t = -1;

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
          aux_hist = hist_size;
          hist_size = index + 1;
          histogram = (int*)realloc(histogram, hist_size * sizeof(int));
          for (i = aux_hist; i < hist_size; i++) {
            histogram[i] = 0;
          }
          if (histogram == NULL) {
            perror("realloc");
            return -1;
          }
        }
        histogram[index]++;

        buffer = rem(buffer);
        buffer_size++;
      } else { // If the buffer is empty
        channels--;
      }

    } else { // It's an emergency
      inem_events++;  // counting the arrival events
      if (channels_inem < M) { // If the channels aren't full
        events_inem = addNewEvent(events->_time, lambda_total, DEPARTURE, events_inem);
        channels_inem++;
        events = rem(events);
      } else if (buffer_inem_size < L) {
        buffer_inem = add(buffer_inem, ARRIVAL, events->_time);
        buffer_inem_size++;
      } else { // If the inem's buffer has more events that PC can support the event is lost
        losses_inem++;
        events = rem(events);
      }
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

  fprintf(stdout, "\nProbability of a call being delayed at the entry to the PC system (buffer events = %d): %.3f%%\n",
                     buffer_events, buffer_events / (float)(arrival_events+inem_events) * 100);

  fprintf(stdout, "\nProbability of a call being lost (losses = %d) at the entry to the PC system (total arrivals = %d): %.3f%%\n",
                     losses, arrival_events, losses / (float)(arrival_events+inem_events) * 100);

  fprintf(stdout, "\nAverage delay time of calls in the PC system entry: %.3fmin = %.3fsec\n",
                     delay_time / (float)(arrival_events+inem_events), delay_time / (float)(arrival_events+inem_events) * 60);

  fprintf(stdout, "\nAverage delay time of calls from PC to INEM: %.3fmin = %.3fsec\n\n",
                     delay_time / (float)inem_events, delay_time / (float)(inem_events) * 60);

  //free(histogram);

  return 0;
}
