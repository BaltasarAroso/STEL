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
  fprintf(CSV, "Indice, Tempo Central, Número de Chegadas (total %d)\n", ARRIVALS);
  for (i = 0; i < hist_size; i++) {
    fprintf(CSV, "%d, %lf, %d\n", i, (2*i+1)/(float)(hist_size*2), histogram[i]);
  }

  return 0;
}

int main(int argc, char* argv[]){
  //definir o tamanho do histogram que é dado pelo número de saltos*lambda
  int hist_size = HIST_JUMP*lambda;

  //índice e vetor dos valores do histograma
  int index = 0, *histogram;

  //variáveis para cálculo dos valores obtidos
  double u, C, sum = 0;

  //nome do ficheiro.csv
  char filename[50];

  //valor escrito na consola depois do ./tp1 correspondente ao nome do ficheiro
  if (!strcpy(filename, argv[1])){
    perror("strcpy");
    return -1;
  }

  //alocar memória dinâmica do tamanho do histograma (hist_size)
  //iniciar a 0 o numero de chegadas
  histogram = (int*)calloc(hist_size, sizeof(int));

  //semente do rand()
  srand(time(NULL));

  //ciclo onde são obtidos os saltos e colocados no histograma
  for (int i = 0; i < ARRIVALS; i++) {

    //debug
    //fprintf(stderr, "%d\n", i);

    // u pertence ao intervalo ]0,1]
    u = (rand()%RAND_MAX + 1) / (float)RAND_MAX;
    //debug
    //fprintf(stderr, "u = %lf\n", u);

    //valor do "salto"/intervalos
    C = -(log(u)/(float)lambda);
    //debug
    //fprintf(stderr,"C = %lf\n\n", C);

    //soma de todos os valores
    sum += C;

    //cálculo do índice onde colocar o valor no histograma
    index = (int)(C / (1 / (float)(hist_size)));

    //se o valor de C exceder o valor máximo, colocar esses valores no indice maximo
    histogram[index > (hist_size - 1) ? hist_size - 1 : index]++;

  }

  //calcular a média esperada (ponto 2 da alínea a)
  fprintf(stderr, "Theoretical Value of lambda = %lf\n", (float)lambda);
  fprintf(stderr, "Estimator of lambda = %lf\n\n", (float)ARRIVALS / sum);

  //guardar os valores no ficheiro.csv
  if(saveInCSV(filename, histogram, hist_size) < 0) {
    perror("saveInCSV");
    return -1;
  }
  fprintf(stderr, "File %s saved successfully\n\n", filename);

  return 0;
}
