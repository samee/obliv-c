#pragma once
#include<obliv_types_internal.h>
#include<string.h>
// These are yao-related functions from obliv_bits.c that later became useful 
// in other files as well
void yaoKeyNewPair(YaoProtocolDesc* pd,yao_key_t w0,yao_key_t w1);
static inline void yaoKeyCopy(yao_key_t d, const yao_key_t s) 
  { memcpy(d,s,YAO_KEY_BYTES); }
static inline void yaoKeyZero(yao_key_t d) { memset(d,0,YAO_KEY_BYTES); }
static inline bool yaoKeyLsb(const yao_key_t k) { return k[0]&1; }
static inline void yaoKeyXor(yao_key_t d, const yao_key_t s)
  { memxor(d,s,YAO_KEY_BYTES); }
void yaoSetHalfMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,uint64_t k);
void yaoSetHashMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,const yao_key_t b,
                    uint64_t k,int i);

// Assumes b is unknown
const char* yaoKeyOfBit(const OblivBit* b);

extern void setupYaoProtocol(ProtocolDesc* pd,bool halfgates);
extern void mainYaoProtocol(ProtocolDesc* pd, bool point_and_permute,
                            protocol_run start, void* arg);
extern void cleanupYaoProtocol(ProtocolDesc* pd);
extern bool yaoGenrRevealOblivBits(ProtocolDesc* pd,
                widest_t* dest,const OblivBit* o,size_t n,int party);
extern bool yaoEvalRevealOblivBits(ProtocolDesc* pd,
                widest_t* dest,const OblivBit* o,size_t n,int party);
extern void yaoGenrFeedOblivInputs(ProtocolDesc* pd
               ,OblivInputs* oi,size_t n,int src);
extern void yaoEvalFeedOblivInputs(ProtocolDesc* pd
               ,OblivInputs* oi,size_t n,int src);

extern void yaoHalfSwapGate(ProtocolDesc* pd,
    OblivBit a[],OblivBit b[],int n,bool c);

// Uses private value from party 'party's x to initialize
// a special "fatBit" OblivBit object. These can only be used by
// __obliv_c__fatDecode() later. Useful with yaoHalfSwapGate.
static inline OblivBit __obliv_c__fatBit(bool x)
  { return (OblivBit){.unknown=false,{.knownValue=x}}; }
static inline bool __obliv_c__fatDecode(const OblivBit* b) 
  { return b->knownValue; }

