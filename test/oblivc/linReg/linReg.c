// Samuel Havron <havron@virginia.edu>
// https://www.github.com/samuelhavron
// MIT License
// WARNING: CURRENTLY IN DEVELOPMENT, WILL NOT SUCCESSFULLY COMPUTE DATA

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "linReg.h"
#include <obliv.h>
#include "../common/util.h"

int main(int argc, char *argv[]) {
  printf("Linear Regression\n");
  printf("=================\n\n");

  ProtocolDesc pd;
  protocolIO io;

  // Check args
  if (argc == 4) {
    ocTestUtilTcpOrDie(&pd, argv[2][0]=='1', argv[1]);
    setCurrentParty(&pd, (argv[2][0]=='1'?1:2)); // only checks for a '1'
    
    io.src = argv[3]; // filename
    execYaoProtocol(&pd, linReg, &io); // start linReg.oc

    fprintf(stderr, "\nSlope   \tm = %15.6e\n", DESCALE(io.m)); // print slope
    fprintf(stderr, "y-intercept\tb = %15.6e\n", DESCALE(io.b)); // print y-intercept
    fprintf(stderr, "Correlation\tr = %15.6e\n", DESCALE(io.r)); // print correlation
    cleanupProtocol(&pd);
  } else {
    printf("Usage: %s <port> <1|2> <filename>\n", argv[0]);

  }
  return 0;
}

void load_data(protocolIO *io, int x[MAXN], int y[MAXN], int party) {
  FILE *inputFile = fopen(io->src, "r");

  if (inputFile == NULL) {
   perror(io->src);
   exit(1);
  }
  
  io->n = 0;
  int i;
  double a;
  for (i = 0; i < MAXN; i++) {
    int dataPoints = fscanf(inputFile, "%lf", &a);
    
    if (dataPoints != 1) {
      if (dataPoints < 0 && feof(inputFile)) {
      	io->n = i;
	break;
      } else {
	fprintf(stderr, "ERROR: Input does not match file format. Check input file.\n");
	printf("File format exception found at Line %d or Line %d in file.\n", i, i + 1);
	exit(1);
      }
    }
    
    if (party == 1) {
      x[i] = a * SCALE;
      io->x[i] = x[i]; // REMOVE AFTER NON-OBLIV TEST
    } else if (party == 2) {
      y[i] = a * SCALE;
      io->y[i] = y[i]; // REMOVE AFTER NON-OBLIV TEST
    }
  }

  printf("Loading %d data points ...\n", io->n);
  fclose(inputFile);
}
