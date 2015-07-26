#pragma once
#include<string.h>
// These are yao-related functions from obliv_bits.c that later became useful 
// in other files as well
void yaoKeyNewPair(YaoProtocolDesc* pd,yao_key_t w0,yao_key_t w1);
static inline void yaoKeyCopy(yao_key_t d, const yao_key_t s) 
  { memcpy(d,s,YAO_KEY_BYTES); }
static inline void yaoKeyZero(yao_key_t d) { memset(d,0,YAO_KEY_BYTES); }
static inline bool yaoKeyLsb(const yao_key_t k) { return k[0]&1; }
static inline void yaoKeyXor(yao_key_t d, const yao_key_t s)
{
#if YAO_KEY_BYTES==10
  ((int64_t *)d)[0] ^= ((int64_t *)s)[0];
  ((int16_t *)d)[4] ^= ((int16_t *)s)[4];
#else
 int i;
  for(i=0;i<YAO_KEY_BYTES;++i) d[i]^=s[i];
#endif
}
void yaoSetHalfMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,uint64_t k);
void yaoSetHashMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,const yao_key_t b,
                    uint64_t k,int i);

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

