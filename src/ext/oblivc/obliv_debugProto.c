#include <obliv_common.h>
#include <obliv_bits.h>
#include <commitReveal.h>
#include <nnob.h>
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

//-------------------------- Debug Protocol -----------------------------------

static void dbgFeedOblivBool(OblivBit* dest,int party,bool a)
{ 
  int curparty = ocCurrentParty();
  
  dest->unknown=true;
  if(party==1) { if(curparty==1) dest->knownValue=a; }
  else if(party==2 && curparty == 1) 
    orecv(currentProto,2,&dest->knownValue,sizeof(bool));
  else if(party==2 && curparty == 2) osend(currentProto,1,&a,sizeof(bool));
  else fprintf(stderr,"Error: This is a 2 party protocol\n");
}
  /*
void __obliv_c__feedOblivBits(OblivBit* dest, int party
                             ,const bool* src,size_t size)
  { while(size--) __obliv_c__feedOblivBool(dest++,party,*(src++)); }
*/

void dbgProtoFeedOblivInputs(ProtocolDesc* pd,
    OblivInputs* spec,size_t count,int party)
{ while(count--)
  { int i;
    widest_t v = spec->src;
    for(i=0;i<spec->size;++i) 
    { dbgFeedOblivBool(spec->dest+i,party,v&1);
      v>>=1;
    }
    spec++;
  }
}

bool dbgProtoRevealOblivBits
  (ProtocolDesc* pd,widest_t* dest,const OblivBit* src,size_t size,int party)
{ widest_t rv=0;
  if(currentProto->thisParty==1)
  { src+=size;
    while(size-->0) rv = (rv<<1)+!!(--src)->knownValue;
    if(party==0 || party==2) osend(pd,2,&rv,sizeof(rv));
    if(party==2) return false;
    else { *dest=rv; return true; }
  }else // assuming thisParty==2
  { if(party==0 || party==2) { orecv(pd,1,dest,sizeof(*dest)); return true; }
    else return false;
  }
}

// TODO all sorts of identical parameter optimizations
// Implementation note: remember that all these pointers may alias each other
void dbgProtoSetBitAnd(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (a->knownValue&& b->knownValue);
  dest->unknown = true;
  currentProto->debug.mulCount++;
}

void dbgProtoSetBitOr(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (a->knownValue|| b->knownValue);
  dest->unknown = true;
  currentProto->debug.mulCount++;
}
void dbgProtoSetBitXor(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (!!a->knownValue != !!b->knownValue);
  dest->unknown = true;
  currentProto->debug.xorCount++;
}
void dbgProtoSetBitNot(ProtocolDesc* pd,OblivBit* dest,const OblivBit* a)
{
  dest->knownValue= !a->knownValue;
  dest->unknown = a->unknown;
}
void dbgProtoFlipBit(ProtocolDesc* pd,OblivBit* dest) 
  { dest->knownValue = !dest->knownValue; }

void execDebugProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
    pd->currentParty = ocCurrentPartyDefault;
    pd->error = 0;
    pd->feedOblivInputs = dbgProtoFeedOblivInputs;
    pd->revealOblivBits = dbgProtoRevealOblivBits;
    pd->setBitAnd = dbgProtoSetBitAnd;
    pd->setBitOr  = dbgProtoSetBitOr;
    pd->setBitXor = dbgProtoSetBitXor;
    pd->setBitNot = dbgProtoSetBitNot;
    pd->flipBit   = dbgProtoFlipBit;
    pd->partyCount= 2;
    pd->extra = NULL;
    currentProto = pd;
    currentProto->debug.mulCount = currentProto->debug.xorCount = 0;
    start(arg);
}
  