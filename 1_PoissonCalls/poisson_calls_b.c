#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRIVALS 100000000
#define deltaTIME 100
#define HIST_JUMP 10
#define lambda 8

int saveInCSV(char* filename, int* histogram, int hist_size){
  FILE *CSV;
  int i;
  fprintf(stderr, "Saving the histogram in the %s\n", filename);

  CSV = fopen(filename,"w+");
  if(CSV == NULL) {
    perror("fopen");
    return -1;
  }
  fprintf(CSV, "Index, Central Time, Arrival Calls (of %d)\n", ARRIVALS);
  for (i = 0; i < hist_size; i++) {
    fprintf(CSV, "%d, %lf, %d\n", i, (2*i+1)/(float)(hist_size*2), histogram[i]);
  }

  return 0;
}

int main(int argc, char* argv[]){
  int i, j, index = 0, hist_size = HIST_JUMP*lambda, count = 0, *histogram;
  double u, delta, p, curr_time = 0, old_time = 0;
	char filename[20];

	histogram = (int*) calloc(hist_size, sizeof(int));
  delta = 1/(double)(deltaTIME*lambda);
	if(!strcpy(filename, argv[1])){
		perror("strcpy");
		return -1;
	}

	//probability of the event occur in delta
  p = lambda*delta;

  srand(time(NULL));

  for(i = 0; i < ARRIVALS; i++){
    u = (float)(rand()+1)/RAND_MAX;

		if(u < p) {
      //increment of the current time according to the arrival number (i)
      curr_time = i*delta;

      //increase in expected number of arrivals
			count++;

      //find the index where that jump is added to in the histogram
			index = (curr_time-old_time)/(1 / (float)(HIST_JUMP*lambda));

      //if the jump exceeds the maximum index added it in the last index
			histogram[index > (hist_size - 1) ? hist_size - 1 : index]++;
			old_time = curr_time;
		}
  }

	fprintf(stderr, "Theoretical Value of lambda = %lf\n", (float)lambda);
	fprintf(stderr, "Estimator of lambda = %lf\n\n", count*deltaTIME*lambda / (float)(ARRIVALS));

	if(saveInCSV(filename, histogram, hist_size) < 0) {
		perror("saveInCSV");
		return -1;
	}
	fprintf(stderr, "File %s saved successfully\n\n", filename);

  return 0;
}
