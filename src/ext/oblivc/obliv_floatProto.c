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

void floatFeedOblivFloat(OblivBit* dest, int party, bool a) 
{
    /**************************/
    int curparty = ocCurrentParty();
    /**************************/

    dest->unknown = false; // true;
    if(party == 1) {
        if (curparty == 1) {
            dest->knownValue = a;
        }
    } else if(party == 2 && curparty == 1)  {
        orecv(currentProto, 2, &dest->knownValue, sizeof(bool));
    } else if(party == 2 && curparty == 2) {
        osend(currentProto, 1, &a, sizeof(bool));
    } else {
        fprintf(stderr, "Error: This is a 2 party protocol\n");
    }
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
    widest_t rv = 0;
    memcpy(&rv, floatBytes, float_byte_size);

    if (currentProto->thisParty == 1) {
        if(party == 0 || party == 2) {
            osend(pd, 2, &rv, sizeof(rv));
        }
        if(party == 2) {
            return false;
        } else {
            *dest = rv;
            return true; 
        }
    } else { 
        if(party == 0 || party == 2) {
            orecv(pd, 1, dest, sizeof(*dest)); 
            return true; 
        } else {
            return false;
        }
    }
}

void floatProtoSetBitAnd(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (a->knownValue&& b->knownValue);
  dest->unknown = true;
}

void floatProtoSetBitOr(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (a->knownValue|| b->knownValue);
  dest->unknown = true;
}
void floatProtoSetBitXor(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (!!a->knownValue != !!b->knownValue);
  dest->unknown = true;
}

void floatProtoSetBitNot(ProtocolDesc* pd,OblivBit* dest,const OblivBit* a)
{
  dest->knownValue= !a->knownValue;
  dest->unknown = a->unknown;
}

void floatProtoFlipBit(ProtocolDesc* pd,OblivBit* dest) 
{ 
    dest->knownValue = !dest->knownValue; 
}

void execFloatProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
    pd->currentParty = ocCurrentPartyDefault;
    pd->error = 0;
    pd->partyCount = 2;
    pd->extra = NULL;
    pd->feedOblivInputs = floatProtoFeedOblivInputs;
    pd->revealOblivBits = floatProtoRevealOblivBits;
    pd->setBitAnd = floatProtoSetBitAnd;
    pd->setBitOr  = floatProtoSetBitOr;
    pd->setBitXor = floatProtoSetBitXor;
    pd->setBitNot = floatProtoSetBitNot;
    pd->flipBit   = floatProtoFlipBit;
    currentProto = pd;
    start(arg);
}

#endif
