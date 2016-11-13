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

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


#ifndef CURRENT_PROTO
#define CURRENT_PROTO
// Right now, we do not support multiple protocols at the same time
static __thread ProtocolDesc *currentProto;
static inline bool known(const OblivBit* o) { return !o->unknown; }
#endif

#ifndef FLOAT_PROTO
#define FLOAT_PROTO

//-------------------------- Float Protocol -----------------------------------

static void floatFeedOblivFloat(OblivBit* dest, int party, float a)
{ 
    int curparty = ocCurrentParty();
    dest->unknown=true;
    if (party == 1) { 
        if(curparty == 1) dest->floatValue = a; 
    } else if (party == 2 && curparty == 1) {
        orecv(currentProto, 2, &dest->floatValue, sizeof(float));
    } else if (party == 2 && curparty == 2) {
        osend(currentProto, 1, &a, sizeof(float));
    } else {
        fprintf(stderr,"Error: This is a 2 party protocol\n");
    }
}

void floatProtoFeedOblivInputs(ProtocolDesc* pd,
    OblivInputs* spec,size_t count,int party)
{ 
    while(count--) { 
        int i;
        widest_t v = spec->src;
        for(i=0;i<spec->size;++i) { 
            floatFeedOblivFloat(spec->dest+i,party,v&1);
            v >>= 1;
        }
        spec++;
    }
}

bool floatProtoRevealOblivBits(float *dest, __obliv_c__float src, int party)
{
    *dest = src.bits.floatValue;
    return true;
}
  
void floatProtoAdd(ProtocolDesc* pd, 
    OblivBit* dest,const OblivBit* a,const OblivBit* b) 
{
    int curparty = ocCurrentParty();
    int pid;
    int status;
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Creating pipe failed");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    switch (pid) {
        case 0:
            close(fd[0]);
            dup2(fd[1], 1);
            char* const argAlice[] = {"--alice --input", (char*) &a->floatValue};
            char* const argBob[] = {"--bob --input", (char*) &b->floatValue};
            if (curparty == 1) {
                execv("../scd/bin/garbled_circuit/TinyGarble", argAlice);
            } else {
                execv("../scd/bin/garbled_circuit/TinyGarble", argBob);
            }
            perror("Exec failed!");
            exit(0);
            break;
        case -1:
            perror("Fork failed");
            exit(-1);
        default:
            close(fd[1]);
            break;
    }
    float res;
    char buffer[sizeof(float)];
    while(1) {
        int bytesRead = read(fd[0], buffer, sizeof(buffer));
        if (bytesRead <= 0) break;
    }
    memcpy(&res, &buffer, sizeof(buffer));
    waitpid(pid, &status, 0);
    dest->floatValue = res;
    dest->unknown = false;
    currentProto->debug.mulCount++;
}

void execFloatProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
    pd->currentParty = ocCurrentPartyDefault;
    pd->error = 0;
    pd->feedOblivInputs = floatProtoFeedOblivInputs;
    pd->revealOblivBits = floatProtoRevealOblivBits;
    pd->partyCount = 2;
    pd->extra = NULL;
    currentProto = pd;
    currentProto->debug.mulCount = currentProto->debug.xorCount = 0;
    start(arg);
}

#endif
