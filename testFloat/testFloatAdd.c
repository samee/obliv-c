#include <stdio.h>
#include <stdlib.h>
#include <obliv.h>
#include <obliv.oh>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../test/oblivc/common/util.h"
#include "dbg.h"

#include <obliv_common.h>
#include <obliv_bits.h>
#include <commitReveal.h>
#include <assert.h>
#include <errno.h>      // libgcrypt needs ENOMEM definition
#include <inttypes.h>
#include <stdio.h>      // for protoUseStdio()
#include <string.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <gcrypt.h>

#ifndef CURRENT_PROTO
#define CURRENT_PROTO
// Right now, we do not support multiple protocols at the same time
static __thread ProtocolDesc *currentProto;
static inline bool known(const OblivBit* o) { return !o->unknown; }
#endif

double lap;
int cp;

typedef struct {
  float v; // Value
  int party;
  float ores;
} protocolIO;

int ocCurrentParty2() 
{
    return currentProto->currentParty(currentProto);
}

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

void printOblivInput(OblivInputs n)
{
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        printf("%i", n.dest[i].knownValue);
    }
    printf("\n");
}

void load_data(protocolIO *io, float* x, float* y, int party) 
{
    if (party == 1) {
        *x = io->v;
    } else if (party == 2) {
        *y = io->v;
    }
}

void floatAddi(protocolIO* io) 
{
    float x;
    float y;
    float z = 0;

    /**************************/
    int curparty = ocCurrentParty2();
    /**************************/
    
    load_data(io, &x, &y, ocCurrentParty2());

    __obliv_c__float obliv_x;
    __obliv_c__float obliv_y;
    __obliv_c__float obliv_z;

    __obliv_c__int obliv_xi;
    
    
    printf("%f\n", x);
    printAsBinary(x);
    
    /*OblivInputs spec_x;
    setupOblivFloat(&spec_x,&obliv_x,x);
    spec_x.src_f = x;
    feedOblivInputs(&spec_x, 1, 1);
    printOblivBits(obliv_x);
    printOblivInput(spec_x);*/
    obliv_x = feedOblivFloat(x, 1);
    
    
    printf("%f\n", y);
    printAsBinary(y);

    /*OblivInputs spec_y;
    setupOblivFloat(&spec_y,&obliv_y,y);
    spec_y.src_f = y;
    feedOblivInputs(&spec_y, 1, 2);
    printOblivBits(obliv_y);
    printOblivInput(spec_y);*/
    obliv_y = feedOblivFloat(y, 2);


    /*OblivInputs spec_z;
    setupOblivFloat(&spec_z,&obliv_z,z);
    spec_z.src_f = z;*/

    __obliv_c__setPlainAddF(obliv_z.bits, obliv_x.bits, obliv_y.bits, 32);
    // revealOblivFloat(&z, obliv_z, 32, 0);
    revealOblivFloat(&z, obliv_z, 0);
    printf("Res: %f\n", z);
    printAsBinary(z);
    printOblivBits(obliv_z);

    io->ores = z;
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