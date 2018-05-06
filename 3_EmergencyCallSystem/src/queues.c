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
#define DELAY 1
#define PREVISION 2
#define POSITIVE 7
#define NEGATIVE -7
#define WINDOW 100

// defines for list type events
#define ARRIVAL_PC 1
#define DEPARTURE_PC 2

#define ARRIVAL_INEM 10
#define DEPARTURE_INEM 20

// functions

int saveInCSV(char* filename, int* histogram, int hist_size, int hist_type, int value_type) {
  FILE *CSV;
  int i;
  fprintf(stdout, "\n\n\tSaving the histogram in \"%s\"\n", filename);

  CSV = fopen(filename,"a");
  if(CSV == NULL) {
    perror("fopen");
    return -1;
  }

  // delay histogram
  if (hist_type == DELAY) {
    fprintf(CSV, "Index, Delay\n");
    for (i = 0; i < hist_size; i++) {
      fprintf(CSV, "%d, %d\n", i, histogram[i]);
    }

  // prevision histogram
  } else if (value_type == NEGATIVE){
    fprintf(CSV, "Index, Prevision\n");
    for (i = hist_size - 1; i > 0; i--) {
      fprintf(CSV, "%d, %d\n", -i, histogram[i]);
    }
  } else if (value_type == POSITIVE){
    //fprintf(CSV, "Index, Prevision\n");
    for (i = 0; i < hist_size; i++) {
      fprintf(CSV, "%d, %d\n", i, histogram[i]);
    }
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

list * addNewEvent(double t, double arrival_time, int type, list* event_list) {
  event_list = add(event_list, type, (t + calcTime(type)), arrival_time, 0, 0);
  return event_list;
}

// Function to tell the user the mean time he will have to wait based on system load
double tellDelay(double avg_delay, int buffer_positions, int buffer_events) {
  /*if (buffer_positions != 0 && buffer_events != 0 && buffer_events > buffer_positions) {
    fprintf(stderr, "\t\t\t\tFIRST\n");
    return (avg_delay * (buffer_events - buffer_positions) + (buffer_positions * 0.5 * DM) * buffer_positions) / (float) buffer_events;
  } else if (buffer_positions != 0 && buffer_events != 0) {
    fprintf(stderr, "\t\t\t\tSECOND\n");
    return buffer_positions * 0.5 * DM;
  }*/
  /*if (buffer_events == buffer_positions) {
    return buffer_positions * 0.5 * DM;
  } else {*/
    return avg_delay * buffer_positions;
  //}
  //return 0;
}

// Function that generates a random event that could be an emergency or not (60% prob of emeregency)
int arrivalOrEmergency () {
  if ((rand() % 100) < 60) {
  	return ARRIVAL_INEM;
  } else {
  	return ARRIVAL_PC;
  }
}

int* insertValuesInHistogram (int index, int* hist_size, int* histogram) {
  int i = 0, aux_hist = 0;
  index = abs(index);
  if (index + 1 > *hist_size) {
    aux_hist = *hist_size;
    *hist_size = index + 1;
    histogram = (int*)realloc(histogram, (*hist_size) * sizeof(int));
    for (i = aux_hist; i < *hist_size; i++) {
      histogram[i] = 0;
    }
    if (histogram == NULL) {
      perror("realloc");
      return NULL;
    }
  }
  histogram[index]++;
  return histogram;
}

int main(int argc, char* argv[]) {

  // useful variables
  int i = 0, aux_type = 0, losses = 0;
  double aux_time = 0, arrival_time = 0, calc_time = 0, new_time = 0, delay_time = 0, delay_pc_inem = 0;

  // histogram_delay
  int *histogram_delay = NULL, index_delay = 0, hist_size_delay = 0;
  char filename_delay[50] = "";

  // histogram_prevision
  int index_prevision = 0, hist_size_prevision_negative = 0, hist_size_prevision_positive = 0;
  int *histogram_prevision_positive = NULL, *histogram_prevision_negative = NULL;
  double avg_delay = 0;
  char filename_prevision[50] = "";

  // PC variables
  int arrival_events = 0, channels_pc = 0, buffer_size = L, buffer_events = 0;
  list *events = NULL, *buffer = NULL;

  // INEM variables
  int inem_events = 0, channels_inem = 0;
  list *buffer_inem = NULL;

  srand(time(NULL));

  // associate filename to histogram_delay
  strcat(filename_delay, argv[1]);
  histogram_delay = (int*)calloc(1, sizeof(int));

  // associate filename to histogram_prevision
  //strcat(filename_prevision, argv[2]);
  //histogram_prevision_positive = (int*)calloc(1, sizeof(int));
  //histogram_prevision_negative = (int*)calloc(1, sizeof(int));

  events = add(events, ARRIVAL_PC, aux_time, arrival_time, 0, 0);

  // We process the list of events
  for(i = 1; i < EVENTS_LIST; i++) {
    // saving event values and remove that event after
  	aux_time = events->_time;
    arrival_time = events->_arrival_time;
    aux_type = events->type;
    events = rem(events);

    if (aux_type == ARRIVAL_PC) { // PC arrival event
      fprintf(stdout, "ARRIVAL_PC\n"); //DEBUG
      arrival_time = aux_time;
      arrival_events++;  // counting the arrival events
      if (channels_pc < M) { // if the channels_pc aren't full
        channels_pc++; // A channel serves the call
        if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
          events = addNewEvent(aux_time, arrival_time, DEPARTURE_PC, events);
        } else { // Change to an ARRIVAL_INEM
          events = addNewEvent(aux_time, arrival_time, ARRIVAL_INEM, events);
        }
      } else if (buffer_size > 0) { // if the buffer has space
          buffer = add(buffer, ARRIVAL_PC, aux_time, arrival_time, avg_delay*(L-buffer_size), L-buffer_size);
          buffer_size--;
          buffer_events++;
          fprintf(stderr, "\tbuffer_events = %d\n", buffer_events); //DEBUG
          fprintf(stderr, "\tbuffer_positions = %d\n", L-buffer_size); //DEBUG
          fprintf(stderr, "\tavg_delay = %lf\n", avg_delay*60); //DEBUG
          fprintf(stderr, "\n\n\t\t\t\t************** BUFFER **************\t\t\t\t\n\n");
          print_elems(buffer);
          fprintf(stderr, "\n\n\t\t\t\t************** END **************\t\t\t\t\n\n");
      } else { // if the channels_pc and buffer are occupied we get a lost event
        losses++;
      }
      // the new event need to have the arrival time calculated as a new time
      calc_time = calcTime(ARRIVAL_PC);
      events = add(events, ARRIVAL_PC, (aux_time + calc_time), (aux_time + calc_time), 0, 0);

    } else if (aux_type == DEPARTURE_PC) { // Departure event
      fprintf(stdout, "DEPARTURE_PC\n"); //DEBUG
      channels_pc--;
      if (buffer != NULL) { // As a channel is free now it can serve an event waiting in the buffer
        channels_pc++; // A channel serves the call
        if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
          events = addNewEvent(buffer->_time, buffer->_arrival_time, DEPARTURE_PC, events);
        } else { // Change to an ARRIVAL_EMERGENCY
          events = addNewEvent(buffer->_time, buffer->_arrival_time, ARRIVAL_INEM, events);
        }

        /* add values to histogram_delay */
        index_delay = (int) 60 * (aux_time - buffer->_time);
        if ((histogram_delay = insertValuesInHistogram(index_delay, &hist_size_delay, histogram_delay)) == NULL) {
          fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Delay !!!\n\n");
          return -1;
        }
        /*****************************/
        delay_time += aux_time - buffer->_time;

        /* add values to histogram_prevision *
        new_time = (aux_time - buffer->_time) / (float) (buffer->buffer_elements + 0.5);
        avg_delay = avg_delay - avg_delay / (float) WINDOW + new_time / (float) WINDOW;
        index_prevision = (int) (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
        fprintf(stderr, "\tindex_prevision = %d\n", index_prevision); //DEBUG
        //index_prevision += ZERO;
        if (index_prevision < 0) {
          if ((histogram_prevision_negative = insertValuesInHistogram(index_prevision, &hist_size_prevision_negative, histogram_prevision_negative)) == NULL) {
            fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Prevision Negative !!!\n\n");
            return -1;
          }
        } else {
          if ((histogram_prevision_positive = insertValuesInHistogram(index_prevision, &hist_size_prevision_positive, histogram_prevision_positive)) == NULL) {
            fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Prevision Positive !!!\n\n");
            return -1;
          }
        }
        *************************************/

        buffer = rem(buffer);
        buffer_size++;
      }

    } else if (aux_type == ARRIVAL_INEM) {
      fprintf(stderr, "ARRIVAL_INEM\n"); //DEBUG
      inem_events++;  // Counting the arrival events
      //fprintf(stderr, "delay_pc_inem = %f\n", delay_pc_inem); //DEBUG
  		if (channels_inem < M_INEM) { // If the channels_inem aren't full
        events = addNewEvent(aux_time, arrival_time, DEPARTURE_INEM, events);
        channels_inem++; // A channel serves the call
        channels_pc--; // One channel from PC becomes free, because passes the call to INEM
        if (buffer != NULL) {
          channels_pc++; // A channel serves the call
          if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
            events = addNewEvent(buffer->_time, buffer->_arrival_time, DEPARTURE_PC, events);
          } else { // Change to an ARRIVAL_INEM
            events = addNewEvent(buffer->_time, buffer->_arrival_time, ARRIVAL_INEM, events);
          }

          /* add values to histogram_delay */
          index_delay = (int) 60 * (aux_time - buffer->_time);
          if ((histogram_delay = insertValuesInHistogram(index_delay, &hist_size_delay, histogram_delay)) == NULL) {
            fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Delay !!!\n\n");
            return -1;
          }
          /*****************************/
          delay_time += aux_time - buffer->_time;

          /* add values to histogram_prevision *
          new_time = (aux_time - buffer->_time) / (float) (buffer->buffer_elements + 0.5);
          avg_delay = avg_delay - avg_delay / (float) WINDOW + new_time / (float) WINDOW;
          index_prevision = (int) (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
          fprintf(stderr, "\tindex_prevision = %d\n", index_prevision); //DEBUG
          //index_prevision += ZERO;
          if (index_prevision < 0) {
            if ((histogram_prevision_negative = insertValuesInHistogram(index_prevision, &hist_size_prevision_negative, histogram_prevision_negative)) == NULL) {
              fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Prevision Negative !!!\n\n");
              return -1;
            }
          } else {
            if ((histogram_prevision_positive = insertValuesInHistogram(index_prevision, &hist_size_prevision_positive, histogram_prevision_positive)) == NULL) {
              fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Prevision Positive !!!\n\n");
              return -1;
            }
          }
          *************************************/

          buffer = rem(buffer);
          buffer_size++;
        }
        delay_pc_inem += aux_time - arrival_time;
  		} else { // Otherwise, the event needs to wait on buffer
        buffer_inem = add(buffer_inem, ARRIVAL_INEM, aux_time, arrival_time, 0, 0);
        //fprintf(stderr, "\n\nENTROU NO BUFFER INEM\n"); //DEBUG
      }

    } else if (aux_type == DEPARTURE_INEM) { // It's a DEPARTURE_INEM, what means that the call is transfered to INEM
      fprintf(stdout, "DEPARTURE_INEM\n"); //DEBUG
      if (buffer_inem != NULL) { // As a channel is free now, it can serve an event waiting in the buffer_inem
        events = addNewEvent(aux_time, buffer_inem->_arrival_time, DEPARTURE_INEM, events);
        delay_pc_inem += aux_time - buffer_inem->_arrival_time;
        channels_pc--;
        if (buffer != NULL) {
          channels_pc++; // A channel serves the call
          if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
            events = addNewEvent(buffer->_time, buffer->_arrival_time, DEPARTURE_PC, events);
          } else { // Change to an ARRIVAL_INEM
            events = addNewEvent(buffer->_time, buffer->_arrival_time, ARRIVAL_INEM, events);
          }

          /* add values to histogram_delay */
          index_delay = (int) 60 * (aux_time - buffer->_time);
          if ((histogram_delay = insertValuesInHistogram(index_delay, &hist_size_delay, histogram_delay)) == NULL) {
            fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Delay !!!\n\n");
            return -1;
          }
          /*****************************/
          delay_time += aux_time - buffer->_time;

          /* add values to histogram_prevision *
          new_time = (aux_time - buffer->_time) / (float) (buffer->buffer_elements + 0.5);
          avg_delay = avg_delay - avg_delay / (float) WINDOW + new_time / (float) WINDOW;
          index_prevision = (int) (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
          fprintf(stderr, "\tindex_prevision = %d\n", index_prevision); //DEBUG
          //index_prevision += ZERO;
          if (index_prevision < 0) {
            if ((histogram_prevision_negative = insertValuesInHistogram(index_prevision, &hist_size_prevision_negative, histogram_prevision_negative)) == NULL) {
              fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Prevision Negative !!!\n\n");
              return -1;
            }
          } else {
            if ((histogram_prevision_positive = insertValuesInHistogram(index_prevision, &hist_size_prevision_positive, histogram_prevision_positive)) == NULL) {
              fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Prevision Positive !!!\n\n");
              return -1;
            }
          }
          *************************************/

          buffer = rem(buffer);
          buffer_size++;
        }
        buffer_inem = rem(buffer_inem);
      } else { // If the buffer is empty
        channels_inem--;
      }
    }
  }

  /* save histogram_delay in file.csv */
  if(saveInCSV(filename_delay, histogram_delay, hist_size_delay, DELAY, 0) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stdout, "\tFile \"%s\" saved successfully\n\n", filename_delay);
  free(histogram_delay);
  /******************************/

  /* save histogram_prevision in file.csv *
  if(saveInCSV(filename_prevision, histogram_prevision_negative, hist_size_prevision_negative, PREVISION, NEGATIVE) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stdout, "\tFile \"%s\" with negative values saved successfully\n\n", filename_prevision);
  free(histogram_prevision_negative);
  ******************************/

  /* save histogram_prevision in file.csv *
  if(saveInCSV(filename_prevision, histogram_prevision_positive, hist_size_prevision_positive, PREVISION, POSITIVE) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stdout, "\tFile \"%s\" with positive values saved successfully\n\n", filename_prevision);
  free(histogram_prevision_positive);
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
