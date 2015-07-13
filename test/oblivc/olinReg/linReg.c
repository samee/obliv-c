// Samuel Havron <havron@virginia.edu>
// https://www.github.com/samuelhavron
// MIT License

#include <stdio.h>
#include <math.h>
#include "linReg.h"
#include <obliv.h>
#include "../common/util.h"

double lap;
int currentParty;

const char* mySide() {
  if (currentParty == 1) return "Generator";
  else return "Evaluator";
}

int main(int argc, char *argv[]) {
  printf("Linear Regression\n");
  printf("=================\n\n");

  ProtocolDesc pd;
  protocolIO io;

  // Check args
  if (argc == 4) {
    ocTestUtilTcpOrDie(&pd, argv[2][0]=='1', argv[1]);
    currentParty = (argv[2][0]=='1'?1:2);
    setCurrentParty(&pd, currentParty); // only checks for a '1'
    
    io.src = argv[3]; // filename
    lap = wallClock();
    execYaoProtocol(&pd, linReg, &io); // start linReg.oc
    cleanupProtocol(&pd);
    double runtime = wallClock() - lap; // stop clock here 

    fprintf(stderr, "%s total time: %lf seconds\n", mySide(), runtime);
    fprintf(stderr, "Yao Gate Count: %u\n", yaoGateCount());
    write_runtime(io.n, runtime, currentParty, "runtime.dat");

    fprintf(stderr, "\nSlope   \tm = %15.6e\n", (double) DESCALE(io.m)); // print slope
    fprintf(stderr, "y-intercept\tb = %15.6e\n", (double) DESCALE(io.b)); // print y-intercept
    fprintf(stderr, "Correlation\tr = %15.6e\n", sqrt((double) DESCALE(io.rsqr))); // print correlation
  } else {
    printf("Usage: %s <port> <1|2> <filename>\n", argv[0]);

  }
  return 0;
}

void load_data(protocolIO *io, int x[MAXN], int y[MAXN], int party) {
  FILE *inputFile = fopen(io->src, "r");

  if (inputFile == NULL) {
   perror(io->src);
   exit(1); // causes TCP error for non-null party
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
    } else if (party == 2) {
      y[i] = a * SCALE;
    }
  }

  printf("Loading %d data points ...\n", io->n);
  fclose(inputFile);
}

void write_runtime(int n, double time, int party, const char* dest) {
  FILE *file = fopen(dest, "a");
  
  if (file == NULL) {
    perror(dest);
    exit(1);
  }

  fprintf(file, "[party %d] %d points, %lf seconds\n", party, n, time);
  printf("Write to file %s successful\n", dest);
}
