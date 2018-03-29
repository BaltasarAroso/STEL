#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define ARRIVALS 100000
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
  int hist_size = HIST_JUMP*lambda;
  int index = 0, *histogram;
  double u, C, sum = 0;
  char filename[50];

  if (!strcpy(filename, argv[1])){
    perror("strcpy");
    return -1;
  }

  histogram = (int*)calloc(hist_size, sizeof(int));

  srand(time(NULL));

  //cycle where the jumps are calculated and added in the histogram
  for (int i = 0; i < ARRIVALS; i++) {
    u = (rand()%RAND_MAX + 1) / (float)RAND_MAX;

    //jumps size / intervals
    C = -(log(u)/(float)lambda);
    sum += C;

    //find the index where that jump is added to in the histogram
    index = (int)(C / (1 / (float)(hist_size)));

    //if the jump exceeds the maximum index added it in the last index
    histogram[index > (hist_size - 1) ? hist_size - 1 : index]++;

  }

  fprintf(stderr, "Theoretical Value of lambda = %lf\n", (float)lambda);
  fprintf(stderr, "Estimator of lambda = %lf\n\n", (float)ARRIVALS / sum);

  if(saveInCSV(filename, histogram, hist_size) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stderr, "File %s saved successfully\n\n", filename);

  return 0;
}
