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

#ifndef FLOAT_PROTO
#define FLOAT_PROTO

//-------------------------- Float Protocol -----------------------------------

/*static void floatFeedOblivFloat(OblivBit* dest, int party, float a)
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
    __obliv_c__setPlainAddF(dest, a, b, __bitsize(dest))
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
}*/

#endif
