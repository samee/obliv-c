// Samuel Havron <havron@virginia.edu>
// https://www.github.com/samuelhavron
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "linReg.h"

// Fixed point arithmetic
#define SCALE (1 << 16) // 2^16
#define DESCALE(x) (double) x / SCALE

void check_mem(int *x, int *y) {
 if(x == NULL || y == NULL) {
    printf("ERROR: Memory allocation failed\n");
    exit(1);
  }
}

DataSet load_data(FILE *inputFile) {
  DataSet data;
  data.x = malloc(sizeof(int));
  data.y = malloc(sizeof(int));
  data.n = 0;
  check_mem(data.x, data.y);
  
  double a, b;
  while (!feof(inputFile)) {
    int dataPoints = fscanf(inputFile, "%lf %lf", &a, &b);
    
    if (dataPoints != 2) {
      if (dataPoints < 0 && feof(inputFile)) {
	break;
      } else {
	fprintf(stderr, "ERROR: Input does not match file format. Check input file.\n");
	printf("File format exception found at Line %d in file.\n", data.n + 1);
	exit(1);
      }
    }

    // Update count and resize arrays
    data.n += 1;
    data.x = realloc(data.x, sizeof(int) * data.n);
    data.y = realloc(data.y, sizeof(int) * data.n);
    check_mem(data.x, data.y);
    
    data.x[data.n - 1] = a*SCALE; // -1 for index
    data.y[data.n - 1] = b*SCALE; // -1 for index
  }
 
  printf("Loading %d data points ...\n", data.n); 
  return data;
}

LinRegResult linear_regression(DataSet theData) {
  
  LinRegResult result;
  int n = theData.n; // number of data points
  double sumx = DESCALE(sum(theData.x, n)); // sum of x
  double sumxx = DESCALE(dotProd(theData.x, theData.x, n)); // sum of each x squared
  double sumy = DESCALE(sum(theData.y, n)); // sum of y
  double sumyy = DESCALE(dotProd(theData.y, theData.y, n)); // sum of each y squared
  double sumxy = DESCALE(dotProd(theData.x, theData.y, n)); // sum of each x * y
  
  double m, b, r;
  // Compute least-squares best fit straight line
  m = (n * sumxy - sumx * sumy) / (n * sumxx - sqr(sumx)); // slope
  b = (sumy * sumxx - (sumx * sumxy)) / (n * sumxx - sqr(sumx)); // y-intercept
  r = (sumxy - sumx * sumy / n) / sqrt((sumxx - sqr(sumx) / n) * (sumyy - sqr(sumy)/ n)); // correlation

  result.m = m * SCALE;
  result.b = b * SCALE;
  result.r = r * SCALE;
  return result;
}

int main(int argc, char *argv[]) {
  printf("Linear Regression\n");
  printf("=================\n\n");

  // Check for a filename
  if (argc == 2) {
    FILE *input = fopen(argv[1], "r");
    
    if (input == NULL) {
      perror(argv[1]);
      exit(1);

    } else {
      DataSet theData = load_data(input);
      fclose(input);
     
      LinRegResult linReg = linear_regression(theData);
      clean(theData);
      printf("\nSlope   \tm = %15.6e\n", DESCALE(linReg.m)); // print slope
      printf("y-intercept\tb = %15.6e\n", DESCALE(linReg.b)); // print y-intercept
      printf("Correlation\tr = %15.6e\n", DESCALE(linReg.r)); // print correlation
    }

  } else {
    printf("ERROR: Must enter filename after ./linReg\n");

  }
  return 0;
}

int sqr(int x) {
  return x * x;
}

int dotProd(int *a, int *b, int n) {
  double dotProd = 0;
  int result = 0;
  int i;
  for (i = 0; i < n; i++) {
    dotProd += DESCALE(a[i]) * DESCALE(b[i]);
  }
  result = dotProd * SCALE;
  return result;
}

int sum(int *a, int n) {
  int sum = 0;
  int i;
  for (i = 0; i < n; i++) {
    sum += a[i];
  }
  return sum;
}

void clean(DataSet data) {
  free(data.x);
  free(data.y);
}
