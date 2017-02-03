#include <stdio.h>
#include <stdlib.h>
#include <obliv.h>
#include <obliv.oh>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../test/oblivc/common/util.h"
#include "dbg.h"

double lap;
int currentParty;

typedef struct {
  float v; // Value
  int party;
  float ores;
} protocolIO;

void load_data(protocolIO *io, float* x, float* y, int party) 
{
    if (party == 1) {
        *x = io->v;
    } else if (party == 2) {
        *y = io->v;
    }
}


void floatFeedOblivFloat(OblivBit* dest, int party, bool a) 
{
    int curparty = 1;
    dest->unknown=true;
    if(party==1) { if(curparty==1) dest->knownValue=a; }
    else if(party==2 && curparty == 1)  {
        // orecv(currentProto,2,&dest->knownValue,sizeof(bool));
        __obliv_c__assignBitKnown(dest, a);
    }
    else if(party==2 && curparty == 2) {
        // osend(currentProto,1,&a,sizeof(bool));
    }
    else fprintf(stderr,"Error: This is a 2 party protocol\n");
}

void floatProtoFeedOblivInputs(ProtocolDesc* pd,
    OblivInputs* spec, size_t count, int party)
{
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    int bit_number;
    int this_bit;
    while(count--){
        unsigned char* float_bytes = (unsigned char*) &(spec->src);
        for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
            bit_number = i % byte_size;
            this_bit = (*float_bytes >> bit_number) & 1;
            floatFeedOblivFloat((spec->dest)+i, party, this_bit);
            if ( bit_number == byte_size - 1 ) {
                float_bytes++;
            }
        }
        spec++;
    }
}

bool floatProtoRevealOblivBits(ProtocolDesc* pd,widest_t* dest,
        const OblivBit* src,size_t size,int party) 
{
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    int tmp = 0;
    unsigned char floatBytes[float_byte_size];
    memcpy(floatBytes, (unsigned char*) &tmp, float_byte_size);
    int j = 0;
    unsigned char currentByte = floatBytes[j];
    int bit_number;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        bit_number = i % byte_size;
        currentByte |= (src[i].knownValue << bit_number);
        if (bit_number == byte_size - 1) {
            floatBytes[j] = currentByte;
            j++;
            currentByte = floatBytes[j];
        }
    }
    float rv = 0;
    memcpy(&rv, floatBytes, float_byte_size);
    if(2==1)
    {
        if(party==0 || party==2) {
            // osend(pd,2,&rv,sizeof(rv));
        }
        if(party==2) return false;
        else { 
            *dest=rv; return true; 
        }
    } else { 
        if(party==0 || party==2) { 
            // orecv(pd,1,dest,sizeof(*dest)); 
            memcpy(dest, &rv, float_byte_size);
            return true; 
        }
        else return false;
    }
}

/*
void genOblivFloat(OblivBit* dest, float x, int size) {
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    unsigned char* float_bytes = (unsigned char*) &x;
    int bit_number;
    int this_bit;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        bit_number = i % byte_size;
        this_bit = (*float_bytes >> bit_number) & 1;
        __obliv_c__assignBitKnown(dest+(i), this_bit);
        if ( bit_number == byte_size - 1 ) {
            float_bytes++;
        }
    }
}

void revOblivFloat(float* dest, OblivBit* bits, int size) {
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    int tmp = 0;
    unsigned char floatBytes[float_byte_size];
    memcpy(floatBytes, (unsigned char*) &tmp, float_byte_size);
    int j = 0;
    unsigned char currentByte = floatBytes[j];
    int bit_number;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        bit_number = i % byte_size;
        currentByte |= (bits[i].knownValue << bit_number);
        if (bit_number == byte_size - 1) {
            floatBytes[j] = currentByte;
            j++;
            currentByte = floatBytes[j];
        }
    }
    memcpy(dest, floatBytes, float_byte_size);
}
*/

void printAsBinary(float x) {
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

void printOblivBits(__obliv_c__float n) {
    int float_byte_size = sizeof(float);
    int byte_size = sizeof(char) * 8;
    for ( int i = 0; i < float_byte_size * byte_size; i++ ) {
        printf("%i", n.bits[i].knownValue);
    }
    printf("\n");
}

void floatAddi(protocolIO* io) {
    float x;
    float y;

    load_data(io, &x, &y, io->party);

    __obliv_c__float obliv_x;
    __obliv_c__float obliv_y;
    __obliv_c__float obliv_z;
    
    
    printf("%f\n", x);
    printAsBinary(x);
    
    OblivInputs spec_x;
    setupOblivFloat(&spec_x,&obliv_x,x);
    spec_x.src_f = x;
    floatProtoFeedOblivInputs(NULL, &spec_x, 1, 2);
    printOblivBits(obliv_x);
    
    
    printf("%f\n", y);
    printAsBinary(y);

    OblivInputs spec_y;
    setupOblivFloat(&spec_y,&obliv_y,y);
    spec_y.src_f = y;
    floatProtoFeedOblivInputs(NULL, &spec_y, 1, 2);
    printOblivBits(obliv_y);
    
    float z = 0;

    OblivInputs spec_z;
    setupOblivFloat(&spec_z,&obliv_z,z);
    floatProtoFeedOblivInputs(NULL, &spec_z, 1, 2);

    __obliv_c__setPlainAddF(spec_z.dest, spec_x.dest, spec_y.dest, 32);
    floatProtoRevealOblivBits(NULL, &z, spec_z.dest, 32, 0);
    printf("Res: %f\n", z);
    printAsBinary(z);
    printOblivBits(obliv_z);

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
    currentParty = (argv[2][0]=='1'? 1 : 2);
    setCurrentParty(&pd, currentParty); // only checks for a '1'
    if (currentParty == 1) {
        io.v = 1.2345;
        io.party = 1;
    } else {
        io.v = 2.3456;
        io.party = 2;
    }
    lap = wallClock();

    // Execute Float protocol and cleanup
    // execFloatProtocol(&pd, floatAddi, &io); // starts 'floatAddi()'
    floatAddi(&io);
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