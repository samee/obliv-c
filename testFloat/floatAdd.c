#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "floatAdd.h"
#include <obliv.h>
#include "../test/oblivc/common/util.h"
#include "dbg.h"

double lap;
int currentParty;

const char* mySide() {
  if (currentParty == 1) return "Generator";
  else return "Evaluator";
}

int main(int argc, char *argv[]) {
  printf("Floating Point Addition\n");
  printf("=================\n\n");

  // Check args
  if (argc == 3) {
 
    // Initialize protocols and obtain connection information
    const char *remote_host = strtok(argv[1], ":");
    const char *port = strtok(NULL, ":");
    ProtocolDesc pd;
    protocolIO io;
    
    // Make connection between two shells
    // Modified ocTestUtilTcpOrDie() function from obliv-c/test/oblivc/common/util.c
    log_info("Connecting to %s on port %s ...\n", remote_host, port);
    if(argv[2][0] == '1') {
        if(protocolAcceptTcp2P(&pd,port)!=0) {
            log_err("TCP accept from %s failed\n", remote_host);
	        exit(1);
        }
    } else {
        if(protocolConnectTcp2P(&pd,remote_host,port)!=0) {
            log_err("TCP connect to %s failed\n", remote_host);
	        exit(1);
	    }
    }

    // Final initializations before entering protogol
    currentParty = (argv[2][0]=='1'?1:2);
    setCurrentParty(&pd, currentParty); // only checks for a '1'
    io.v = currentParty; // Val?
    lap = wallClock();

    // Execute Float protocol and cleanup
    execFloatProtocol(&pd, floatAddi, &io); // starts 'floatAddi()'
    cleanupProtocol(&pd);
    double runtime = wallClock() - lap; // stop clock here 

    // Print results and store runtime data
    log_info("%s total time: %lf seconds\n", mySide(), runtime);
    printf("\n");
    log_info("Value   \tv = %f\n", io.v); // print val
  } else {
    log_info("Usage: %s <hostname:port> <1|2> <filename>\n" 
	     "\tHostname usage:\n" 
	     "\tlocal -> 'localhost' remote -> IP address or DNS name\n", argv[0]);
  }
  return 0;
}

void load_data(protocolIO *io, float* x, float* y, int party) {
    if (party == 1) {
        *x = io->v;
    } else if (party == 2) {
        *y = io->v;
    }
}