#include <stdio.h>
#include <stdlib.h>
#include <obliv.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../test/oblivc/common/util.h"
#include "dbg.h"

#include "testFloatAdd.h"

/*
#ifndef CURRENT_PROTO
#define CURRENT_PROTO
// Right now, we do not support multiple protocols at the same time
static __thread ProtocolDesc *currentProto;
static inline bool known(const OblivBit* o) { return !o->unknown; }
#endif
*/

/*int ocCurrentParty2() 
{
    return currentProto->currentParty(currentProto);
}*/


void printAsBinary(float x) 
{
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    unsigned char* float_bytes = (unsigned char*) &x;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        int bit_number = i % byte_size;
        int this_bit = (*float_bytes >> bit_number) & 1;
        printf("%i", this_bit); 
        if ( bit_number == byte_size - 1 ) {
            float_bytes++;
        }
    }
    printf("\n");
}


void printOblivBits(__obliv_c__float n)
{
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        printf("%i", n.bits[i].knownValue);
    }
    printf("\n");
}

/*
void printOblivInput(OblivInputs n)
{
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        printf("%i", n.dest[i].knownValue);
    }
    printf("\n");
}
*/

void load_data(protocolIO *io, float* x, float* y, int party) 
{
    if (party == 1) {
        *x = io->v;
    } else if (party == 2) {
        *y = io->v;
    }
}

int main(int argc, char *argv[]) 
{
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

    // Final initializations before entering protocol
    cp = (argv[2][0]=='1'? 1 : 2);
    setCurrentParty(&pd, cp); // only checks for a '1'
    pd.thisParty = cp;
    if (cp == 1) {
        io.v = 1.2345;
    } else {
        io.v = 2.3456;
    }
    lap = wallClock();

    // Execute Float protocol and cleanup
    execFloatProtocol(&pd, floatAddi, &io); // starts 'floatAddi()'
    cleanupProtocol(&pd);
    double runtime = wallClock() - lap; // stop clock here 

    // Print results and store runtime data
    log_info("Total time: %lf seconds\n", runtime);
    printf("\n");
    log_info("Value   \tv = %f\n", io.ores); // print val
  } else {
    log_info("Usage: %s <hostname:port> <1|2> <filename>\n" 
         "\tHostname usage:\n" 
         "\tlocal -> 'localhost' remote -> IP address or DNS name\n", argv[0]);
  }
  return 0;
}
