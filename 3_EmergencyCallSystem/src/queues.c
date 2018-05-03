#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <signal.h>
#include "lib/linked_list.h"

// defines for input values
#define EVENTS_LIST 1000000
#define LAMBDA 10 // Calls per minute (600/hour)
#define L 6 // Finite buffer size of PC
#define DM 1.5 // Mean time of exponential distribution (PC calls)
#define M 18 // service positions at PC
#define M_INEM 18 // service positions at INEM

// defines for gaussian function
#define MU 0.75 // Mean = 45s
#define SIGMA 0.25 // Standard deviation = 15s

// defines for auxiliar code
#define ZERO 100

// defines for list type events
#define ARRIVAL_PC 1
#define DEPARTURE_PC 2

#define ARRIVAL_INEM 10
#define DEPARTURE_INEM 20

// functions

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
    fprintf(CSV, "%d, %d\n", i-ZERO, histogram[i]);
  }
  fclose(CSV);

  return 0;
}

double boxMuller(void) {
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;

  if (call == 1) {
    call = !call;
    return (MU + SIGMA * (double) X2);
  }

  do {
    U1 = -1.0 + ((double) rand () / RAND_MAX) * 2.0;
    U2 = -1.0 + ((double) rand () / RAND_MAX) * 2.0;
    W = U1*U1 + U2*U2;
  } while (W >= 1 || W == 0);

  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;

  call = !call;

  return (MU + SIGMA * (double) X1);
}

double calcTime(int type) {
  double u = 0, C = 0, S = 0, E = 0;

  if (type == DEPARTURE_PC) { // service time
    do {
    	u = (rand()%RAND_MAX+1) / (double)RAND_MAX;
    	S = -(log(u)*DM);
  	} while ((S < 1) || (S > 4));// Min time=1min, Max time=4min
  	return S;

  } else if (type == DEPARTURE_INEM) { // service time
      do {
      	u = (rand()%RAND_MAX+1) / (double)RAND_MAX;
      	S = -(log(u)*DM);
    	} while (S < 1);// Min time=1min, Max time=4min
    	return S;

  } else if (type == ARRIVAL_INEM) {
  	do {
  		E = boxMuller();
  	} while ((E < 0.5) || (E > 1.25));// Min time=30s, Max time=0.75s
    return E;

  } else { // It's an arrival
  	u = (rand()%RAND_MAX+1) / (double)RAND_MAX;
    C = -(log(u)/(double)LAMBDA);
    return C;
  }
}

list * addNewEvent(double t, int type, list* event_list) {
  double event_time = 0;
  event_time = t + calcTime(type);
  event_list = add(event_list, type, event_time, 0);
  return event_list;
}

// Function to tell the user the mean time he will have to wait based on system load
double tellDelay(double avg_error, int buffer_positions, int buffer_events) {
  if (buffer_positions != 0 && buffer_events != 0 && buffer_positions != buffer_events) {
    return (avg_error) + (buffer_positions * 0.5 * DM) / (float)(buffer_events);
  } else if (buffer_positions != 0 && buffer_events != 0) {
    return (buffer_positions * 0.5 * DM) / (float)(buffer_events);
  }
  return 0;
}

// Function that generates a random event that could be an emergency or not (60% prob of emeregency)
int arrivalOrEmergency () {
  if ((rand() % 100) < 60) {
  	return ARRIVAL_INEM;
  } else {
  	return ARRIVAL_PC;
  }
}

int main(int argc, char* argv[]) {

  // useful variables
  int i = 0, losses = 0;
  double aux_time = 0, delay_time = 0, delay_pc_inem = 0, event_time = 0, delay = 0, avg_error = 0;
  //int *histogram = NULL, index = 0, hist_size = 0, aux_hist = 0;
  //char filename[50] = "";

  // PC variables
  int arrival_events = 0, channels = 0, buffer_size = L, buffer_events = 0;
  list *events = NULL, *buffer = NULL;

  // INEM variables
  int inem_events = 0, channels_inem = 0;
  double aux_time_inem = 0, aux_time_pc_inem = 0;
  list *events_inem = NULL, *buffer_inem = NULL;

  srand(time(NULL));

  //strcat(filename, argv[1]);
  //histogram = (int*)calloc(1, sizeof(int));

  events = add(events, ARRIVAL_PC, 0, 0);

  // We process the list of events
  for(i = 1; i < EVENTS_LIST; i++) {
  	aux_time = events->_time;
    if (events->type == ARRIVAL_PC) { // PC arrival event
      fprintf(stdout, "ARRIVAL_PC\n"); //DEBUG
      arrival_events++;  // counting the arrival events
      if (channels < M) { // if the channels aren't full
        if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
          events = addNewEvent(aux_time, DEPARTURE_PC, events);
          channels++; // A channel serves the call
        } else { // Change to an ARRIVAL_EMERGENCY
          events = rem(events);
          events = addNewEvent(aux_time, ARRIVAL_INEM, events);
        }
      } else if (buffer_size > 0) { // if the buffer has space
          buffer_size--;
          buffer_events++;
          buffer = add(buffer, ARRIVAL_PC, aux_time, tellDelay(avg_error, L-buffer_size, buffer_events));
      } else { // if the channels and buffer are occupied we get a lost event
        losses++;
      }

      // Remove the actual arrival event and add a new one
      events = rem(events);
      events = addNewEvent(aux_time, ARRIVAL_PC, events);

    } else if (events->type == DEPARTURE_PC) { // Departure event
      fprintf(stdout, "DEPARTURE_PC\n"); //DEBUG
      events = rem(events);
      if (buffer != NULL) { // As a channel is free now it can serve an event waiting in the buffer
        if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
          events = addNewEvent(events->_time, DEPARTURE_PC, events);
          channels++; // A channel serves the call
        } else { // Change to an ARRIVAL_EMERGENCY
          events = rem(events);
          events = addNewEvent(aux_time, ARRIVAL_INEM, events);
        }
        delay_time += aux_time - buffer->_time;

        /* add values to histogram

        // the purpose is to do a slide average, so we need to update the average
        if (buffer_events > (L-buffer_size)){
          avg_error += (aux_time - buffer->_time) / (float) (buffer_events - (L-buffer_size));
        }

        if (avg_error != 0) {
          //fprintf(stderr, "\t avg_error = %f\n", avg_error); //DEBUG
          //fprintf(stderr, "\t buffer->_predicted_time = %f\n", buffer->_predicted_time); //DEBUG
          index = (int) (60 * (avg_error - buffer->_predicted_time));
          //fprintf(stderr, "\t index = %d\n", index); //DEBUG
        }
        index += ZERO;
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
        ***************************/
        buffer = rem(buffer);
        buffer_size++;
      } else { // If the buffer is empty
        channels--;
      }

    } else if (events->type == ARRIVAL_INEM) {
      fprintf(stderr, "ARRIVAL_INEM\n"); //DEBUG
      inem_events++;  // Counting the arrival events
      events = rem(events);
  		if (channels_inem < M_INEM) { // If the channels aren't full
        events = addNewEvent(aux_time, DEPARTURE_INEM, events);
        channels_inem++; // A channel serves the call
        channels--; // One channel from PC becomes free, because passes the call to INEM
        if (buffer != NULL) {
          if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
            events = addNewEvent(aux_time, DEPARTURE_PC, events);
            channels++; // A channel serves the call
          } else { // Change to an ARRIVAL_EMERGENCY
            events = addNewEvent(aux_time, ARRIVAL_INEM, events);
          }
          buffer = rem(buffer);
          buffer_size++;
        }
  		} else { // Otherwise, the event needs to wait on buffer
        buffer_inem = add(buffer_inem, ARRIVAL_INEM, aux_time, 0);
        fprintf(stderr, "\n\nENTROU NO BUFFER INEM\n");
      }

    } else if (events->type == DEPARTURE_INEM) { // It's a DEPARTURE_INEM, what means that the call is transfered to INEM
      fprintf(stdout, "DEPARTURE_INEM\n"); //DEBUG
      events = rem(events);
      if (buffer_inem != NULL) { // As a channel is free now, it can serve an event waiting in the buffer_inem
        events = addNewEvent(aux_time, DEPARTURE_INEM, events);
        channels--; // One channel from PC becomes free, because passes the call to INEM
        if (buffer != NULL) {
          if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
            events = addNewEvent(events->_time, DEPARTURE_PC, events);
            channels++; // A channel serves the call
          } else { // Change to an ARRIVAL_EMERGENCY
            events = addNewEvent(aux_time, ARRIVAL_INEM, events);
          }
          buffer = rem(buffer);
          buffer_size++;
        }
        //delay_pc_inem += events->init - buffer_inem->_time;
        //fprintf(stderr, "delay_pc_inem = %f\n", delay_pc_inem); //DEBUG
        buffer_inem = rem(buffer_inem);
      } else { // If the buffer is empty
        channels_inem--;
      }
    }
  }

  /* save histogram in file.csv
  if(saveInCSV(filename, histogram, hist_size) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stdout, "\tFile \"%s\" saved successfully\n\n", filename);
  free(histogram);
  ******************************/

  fprintf(stdout, "\nProbability of a call being delayed at the entry of the PC system (buffer events = %d): %.3f%%\n",
                     buffer_events, buffer_events / (float)(arrival_events) * 100);

  fprintf(stdout, "\nProbability of a call being lost (losses = %d) at the entry to the PC system (total arrivals = %d): %.3f%%\n",
                     losses, arrival_events, losses / (float)(arrival_events) * 100);

  fprintf(stdout, "\nAverage delay time of calls in the PC system entry: %.3f min = %.3f sec\n",
                     delay_time / (float)buffer_events, delay_time / (float)buffer_events * 60);

  fprintf(stdout, "\nAverage delay time of calls from PC to INEM: %.3f min = %.3f sec\n\n",
                     delay_pc_inem / (float)inem_events, delay_pc_inem / (float)(inem_events) * 60);

  return 0;
}
