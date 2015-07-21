// Samuel Havron <havron@virginia.edu>
// https://www.github.com/samuelhavron
// MIT License

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
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

 

  // Check args
  if (argc >= 4) {
 
    const char *remoteHost = argv[4];

    ProtocolDesc pd;
    protocolIO io;
    
    if(argv[2][0] == '1') { 
      if(protocolAcceptTcp2P(&pd,argv[1])!=0) { 
	fprintf(stderr,"TCP accept failed\n");
	exit(1);
      }
    }
    else 
      if(protocolConnectTcp2P(&pd,remoteHost,argv[1])!=0) {
	fprintf(stderr,"TCP connect failed\n");
	exit(1);
      }



    //ocTestUtilTcpOrDie(&pd, argv[2][0]=='1', argv[1]);
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

void load_data(protocolIO *io, int** x, int** y, int party) {
  FILE *inputFile = fopen(io->src, "r");

  if (inputFile == NULL) {
   perror(io->src);
   exit(1); // causes TCP error for non-null party
  }
  
  io->n = 0;
  int memsize = ALLOC;
  
  double a;
  while (!feof(inputFile)) {
    int dataPoints = fscanf(inputFile, "%lf", &a);
    
    if (dataPoints != 1) {
      if (dataPoints < 0 && feof(inputFile)) {
	break;
      } else {
	fprintf(stderr, "ERROR: Input does not match file format. Check input file.\n");
	printf("File format exception found at Line %d or Line %d in file.\n", io->n, io->n + 1);
	exit(1);
      }
    }
        
    io->n += 1;
    if (io->n > memsize) {
      printf ("Data is now %d points in size. Changing memsize from %d bytes to ", io->n, memsize);
      memsize *= 2;
      printf ("%d bytes\n", memsize);
      *x = realloc(*x, sizeof(int) * memsize);
      *y = realloc(*y, sizeof(int) * memsize);
      check_mem(*x, *y, party);
    }
    
    int aint = a * SCALE;
    assert(APPROX((double) DESCALE(aint), a));
    if (party == 1) {
      *(*x + io->n - 1) =  aint; // messy, but needed for dereferencing 
    } else if (party == 2) {
      *(*y + io->n - 1) =  aint;
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

void check_mem(int* x, int* y, int party) {
  if((party == 1 && x == NULL) || (party == 2 && y == NULL)) {
    printf("ERROR: Memory allocation failed\n");
    exit(1);
  }
}
