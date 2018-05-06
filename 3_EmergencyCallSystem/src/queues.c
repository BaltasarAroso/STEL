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
#define LAMBDA 600 // 600 calls/hour
#define L 6 // Finite buffer size of PC
#define DM 1.5 // Mean time of exponential distribution (PC calls)
#define M 18 // service positions at PC
#define M_INEM 18 // service positions at INEM

// defines for gaussian function
#define MU 0.75 // Mean = 45s
#define SIGMA 0.25 // Standard deviation = 15s

// defines for auxiliar code
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

// Function that generates a random event that could be an emergency or not (60% prob of emeregency)
int arrivalOrEmergency () {
  if ((rand() % 100) < 60) {
  	return ARRIVAL_INEM;
  } else {
  	return ARRIVAL_PC;
  }
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

double calcTime(int type, double lambda) {
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
    C = -(log(u)/lambda);
    return C;
  }
}

list * addNewEvent(double t, double arrival_time, int type, list* event_list, double lambda) {
  event_list = add(event_list, type, (t + calcTime(type, lambda)), arrival_time, 0, 0);
  return event_list;
}

double calculatePredictedTime(double avg_delay, int buffer_positions, int buffer_events) {
  if (buffer_events == buffer_positions) {
    return buffer_positions * 0.5 * DM;
  } else {
    return avg_delay * buffer_positions;
  }
  return 0;
}

double calculateStandardDeviation(double hist_prevision_values[], int delay_count) {
  int i = 0;
  double sum = 0.0, avg = 0.0, standard_deviation = 0.0;

  for (i = 0; i < delay_count; i++) {
      sum += hist_prevision_values[i];
  }
  avg = sum / delay_count;
  for (i = 0; i < delay_count; i++) {
      standard_deviation += pow(hist_prevision_values[i] - avg, 2);
  }

  return sqrt(standard_deviation / delay_count);
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

int main(int argc, char* argv[]) {
  int min = 0, max = 0, jump = 0, size = 0, k = 0, j = 0, flag_sensitivity = 0;
  int* lambda_values = NULL;
  double* delays = NULL;
  char c = '\0';

  fprintf(stderr, "\n\tDo you pretend to get the sensitivity analysis in a .csv file? (yes = 'y' or no = 'n')\n");
  fprintf(stderr, "\tA 'no' answer will give, by default, 600 calls/hour to the lambda value. Answer: ");
  scanf("%c", &c);
  if (c == 'y') {
    flag_sensitivity = 1;
    fprintf(stderr, "\n\tDefine the values of lambda for the sensitivity analysis:\n");
    do {
      fprintf(stderr, "\tmin: ");
      scanf("%d", &min);
    } while (min < 0);
    do {
      fprintf(stderr, "\tmax: ");
      scanf("%d", &max);
    } while (max <= 0);
    do {
      fprintf(stderr, "\tjump: ");
      scanf("%d", &jump);
    } while (jump <= 0);
    fprintf(stderr, "\n\t*********************************************************\n");
    size = (max - min) / (float)jump;
    lambda_values = (int*)calloc(size + 1, sizeof(int));
    delays = (double*)calloc(size + 1, sizeof(double));
  } else {
    flag_sensitivity = 0;
    min = LAMBDA;
    max = LAMBDA;
    jump = 0;
  }


  for (k = min; k <= max; k += jump) {
    if (flag_sensitivity) {
      lambda_values[j] = k;
    }

    // useful variables
    int i = 0, aux_type = 0, losses = 0, delay_count = 0;
    double lambda = k / 60.0, aux_time = 0, arrival_time = 0, calc_time = 0, delay_time = 0, delay_pc_inem = 0;

    // histogram_delay
    int *histogram_delay = NULL, index_delay = 0, hist_size_delay = 0;
    char filename_delay[50] = "";

    // histogram_prevision
    int index_prevision = 0, hist_size_prevision_negative = 0, hist_size_prevision_positive = 0;
    int *histogram_prevision_positive = NULL, *histogram_prevision_negative = NULL;
    double avg_delay = 0, new_time = 0, absolute_error = 0, relative_error = 0;
    double* hist_prevision_values = NULL;
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
    strcat(filename_prevision, argv[2]);
    histogram_prevision_positive = (int*)calloc(1, sizeof(int));
    histogram_prevision_negative = (int*)calloc(1, sizeof(int));
    hist_prevision_values = (double*)calloc(1, sizeof(double));

    events = add(events, ARRIVAL_PC, aux_time, arrival_time, 0, 0);

    // We process the list of events
    for(i = 1; i < EVENTS_LIST; i++) {
      // saving event values and remove that event after
    	aux_time = events->_time;
      arrival_time = events->_arrival_time;
      aux_type = events->type;
      events = rem(events);

      if (aux_type == ARRIVAL_PC) { // PC arrival event
        //fprintf(stdout, "ARRIVAL_PC\n"); //DEBUG
        arrival_time = aux_time;
        arrival_events++;  // counting the arrival events
        if (channels_pc < M) { // if the channels_pc aren't full
          channels_pc++; // A channel serves the call
          if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
            events = addNewEvent(aux_time, arrival_time, DEPARTURE_PC, events, lambda);
          } else { // Change to an ARRIVAL_INEM
            events = addNewEvent(aux_time, arrival_time, ARRIVAL_INEM, events, lambda);
          }
        } else if (buffer_size > 0) { // if the buffer has space
            buffer = add(buffer, ARRIVAL_PC, aux_time, arrival_time, calculatePredictedTime(avg_delay, L-buffer_size, buffer_events), L-buffer_size);
            buffer_size--;
            buffer_events++;
        } else { // if the channels_pc and buffer are occupied we get a lost event
          losses++;
        }
        // the new event need to have the arrival time calculated as a new time
        calc_time = calcTime(ARRIVAL_PC, lambda);
        events = add(events, ARRIVAL_PC, (aux_time + calc_time), (aux_time + calc_time), 0, 0);

      } else if (aux_type == DEPARTURE_PC) { // Departure event
        //fprintf(stdout, "DEPARTURE_PC\n"); //DEBUG
        channels_pc--;
        if (buffer != NULL) { // As a channel is free now it can serve an event waiting in the buffer
          channels_pc++; // A channel serves the call
          if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
            events = addNewEvent(buffer->_time, buffer->_arrival_time, DEPARTURE_PC, events, lambda);
          } else { // Change to an ARRIVAL_EMERGENCY
            events = addNewEvent(buffer->_time, buffer->_arrival_time, ARRIVAL_INEM, events, lambda);
          }

          /* add values to histogram_delay */
          index_delay = (int) 60 * (aux_time - buffer->_time);
          if ((histogram_delay = insertValuesInHistogram(index_delay, &hist_size_delay, histogram_delay)) == NULL) {
            fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Delay !!!\n\n");
            return -1;
          }
          /*****************************/
          delay_time += aux_time - buffer->_time;

          /* add values to histogram_prevision */
          new_time = (aux_time - buffer->_time) / (float) (buffer->buffer_elements + 0.5);
          avg_delay = avg_delay - avg_delay / (float) WINDOW + new_time / (float) WINDOW;

          // for the calculateStandardDeviation function
          delay_count++;
          hist_prevision_values = (double*)realloc(hist_prevision_values, (delay_count) * sizeof(double));
          hist_prevision_values[delay_count-1] = 60 * (buffer->_predicted_time - (aux_time - buffer->_time));
          if (hist_prevision_values == NULL) {
            perror("realloc");
            return -1;
          }

          index_prevision = (int) (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
          absolute_error += fabs(60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
          relative_error += (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
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
          /*************************************/

          buffer = rem(buffer);
          buffer_size++;
        }

      } else if (aux_type == ARRIVAL_INEM) {
        //fprintf(stderr, "ARRIVAL_INEM\n"); //DEBUG
        inem_events++;  // Counting the arrival events
    		if (channels_inem < M_INEM) { // If the channels_inem aren't full
          events = addNewEvent(aux_time, arrival_time, DEPARTURE_INEM, events, lambda);
          channels_inem++; // A channel serves the call
          channels_pc--; // One channel from PC becomes free, because passes the call to INEM
          if (buffer != NULL) {
            channels_pc++; // A channel serves the call
            if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
              events = addNewEvent(buffer->_time, buffer->_arrival_time, DEPARTURE_PC, events, lambda);
            } else { // Change to an ARRIVAL_INEM
              events = addNewEvent(buffer->_time, buffer->_arrival_time, ARRIVAL_INEM, events, lambda);
            }

            /* add values to histogram_delay */
            index_delay = (int) 60 * (aux_time - buffer->_time);
            if ((histogram_delay = insertValuesInHistogram(index_delay, &hist_size_delay, histogram_delay)) == NULL) {
              fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Delay !!!\n\n");
              return -1;
            }
            /*****************************/
            delay_time += aux_time - buffer->_time;

            /* add values to histogram_prevision */
            new_time = (aux_time - buffer->_time) / (float) (buffer->buffer_elements + 0.5);
            avg_delay = avg_delay - avg_delay / (float) WINDOW + new_time / (float) WINDOW;

            // for the calculateStandardDeviation function
            delay_count++;
            hist_prevision_values = (double*)realloc(hist_prevision_values, (delay_count) * sizeof(double));
            hist_prevision_values[delay_count-1] = 60 * (buffer->_predicted_time - (aux_time - buffer->_time));
            if (hist_prevision_values == NULL) {
              perror("realloc");
              return -1;
            }

            index_prevision = (int) (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
            absolute_error += fabs(60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
            relative_error += (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
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
            /*************************************/

            buffer = rem(buffer);
            buffer_size++;
          }
          delay_pc_inem += aux_time - arrival_time;
    		} else { // Otherwise, the event needs to wait on buffer
          buffer_inem = add(buffer_inem, ARRIVAL_INEM, aux_time, arrival_time, 0, 0);
        }

      } else if (aux_type == DEPARTURE_INEM) { // It's a DEPARTURE_INEM, what means that the call is transfered to INEM
        //fprintf(stdout, "DEPARTURE_INEM\n"); //DEBUG
        if (buffer_inem != NULL) { // As a channel is free now, it can serve an event waiting in the buffer_inem
          events = addNewEvent(aux_time, buffer_inem->_arrival_time, DEPARTURE_INEM, events, lambda);
          delay_pc_inem += aux_time - buffer_inem->_arrival_time;
          channels_pc--;
          if (buffer != NULL) {
            channels_pc++; // A channel serves the call
            if (arrivalOrEmergency() == ARRIVAL_PC) { // Stay as an ARRIVAL from PC
              events = addNewEvent(buffer->_time, buffer->_arrival_time, DEPARTURE_PC, events, lambda);
            } else { // Change to an ARRIVAL_INEM
              events = addNewEvent(buffer->_time, buffer->_arrival_time, ARRIVAL_INEM, events, lambda);
            }

            /* add values to histogram_delay */
            index_delay = (int) 60 * (aux_time - buffer->_time);
            if ((histogram_delay = insertValuesInHistogram(index_delay, &hist_size_delay, histogram_delay)) == NULL) {
              fprintf(stderr, "\n\n\t\t!!! ERROR Inserting Values In Histogram Delay !!!\n\n");
              return -1;
            }
            /*****************************/
            delay_time += aux_time - buffer->_time;

            /* add values to histogram_prevision */
            new_time = (aux_time - buffer->_time) / (float) (buffer->buffer_elements + 0.5);
            avg_delay = avg_delay - avg_delay / (float) WINDOW + new_time / (float) WINDOW;

            // for the calculateStandardDeviation function
            delay_count++;
            hist_prevision_values = (double*)realloc(hist_prevision_values, (delay_count) * sizeof(double));
            hist_prevision_values[delay_count-1] = 60 * (buffer->_predicted_time - (aux_time - buffer->_time));
            if (hist_prevision_values == NULL) {
              perror("realloc");
              return -1;
            }

            index_prevision = (int) (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
            absolute_error += fabs(60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
            relative_error += (60 * (buffer->_predicted_time - (aux_time - buffer->_time)));
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
            /*************************************/

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

    /* save histogram_prevision in file.csv */
    if(saveInCSV(filename_prevision, histogram_prevision_negative, hist_size_prevision_negative, PREVISION, NEGATIVE) < 0) {
      perror("saveInCSV");
      return -1;
    }
    fprintf(stdout, "\tFile \"%s\" with negative values saved successfully\n\n", filename_prevision);
    free(histogram_prevision_negative);
    /******************************/

    /* save histogram_prevision in file.csv */
    if(saveInCSV(filename_prevision, histogram_prevision_positive, hist_size_prevision_positive, PREVISION, POSITIVE) < 0) {
      perror("saveInCSV");
      return -1;
    }
    fprintf(stdout, "\tFile \"%s\" with positive values saved successfully\n\n", filename_prevision);
    free(histogram_prevision_positive);
    /******************************/

    fprintf(stdout, "\nProbability of a call being delayed at the entry of the PC system (buffer events = %d): %.3f%%\n",
                       buffer_events, buffer_events / (float)(arrival_events) * 100);

    fprintf(stdout, "\nProbability of a call being lost (losses = %d) at the entry to the PC system (total arrivals = %d): %.3f%%\n",
                       losses, arrival_events, losses / (float)(arrival_events) * 100);

    fprintf(stdout, "\nAverage delay time of calls in the PC system entry: %.3f min = %.3f sec\n",
                       delay_time / (float)buffer_events, delay_time / (float)buffer_events * 60);

    fprintf(stdout, "\nAverage delay time of calls from PC to INEM: %.3f min = %.3f sec\n\n",
                       delay_pc_inem / (float)inem_events, delay_pc_inem / (float)(inem_events) * 60);

    /*********************************************************************************************************************************/

    fprintf(stderr, "\n\t***** Waiting Time Prediction Error *****\n");

    fprintf(stdout, "\nAverage of the absolute error on the waiting prediction time in the input buffer: %.3f min = %.3f sec\n",
                       absolute_error / (float)(buffer_events * 60), absolute_error / (float)(buffer_events));

    fprintf(stdout, "\nAverage of the relative error on the waiting prediction time in the input buffer: %.3f min = %.3f sec\n",
                      relative_error / (float)(buffer_events * 60), relative_error / (float)(buffer_events));

    fprintf(stdout, "\nStandard deviation of the waiting prediction time in the input buffer: %.3f min = %.3f sec\n",
                      calculateStandardDeviation(hist_prevision_values, delay_count) / 60, calculateStandardDeviation(hist_prevision_values, delay_count));

    fprintf(stderr, "\n\t*****************************************\n");

    if (flag_sensitivity) {
      delays[j] = delay_pc_inem / (float)(inem_events) * 60;
      j++;
    } else {
      break;
    }

  }

  if (flag_sensitivity) {
    // Saving in a .csv file the Sensitivy Analysis
    FILE *sensitivity;
    char filename_sensitivity[50] = "sensitivity.csv";
    fprintf(stdout, "\n\n\tSaving the histogram in \"%s\"\n", filename_sensitivity);
    sensitivity = fopen(filename_sensitivity, "w");
    if(sensitivity == NULL) {
      perror("fopen");
      return -1;
    }
    fprintf(sensitivity, "Lambda, Delays\n");
    for (k = 0; k <= size; k++) {
      fprintf(sensitivity, "%d, %lf\n", lambda_values[k], delays[k]);
    }
    fclose(sensitivity);
    fprintf(stdout, "\tFile \"%s\" saved successfully\n\n", filename_sensitivity);
    free(lambda_values);
    free(delays);
  }

  return 0;
}
