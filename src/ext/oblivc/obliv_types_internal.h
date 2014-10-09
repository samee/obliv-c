#pragma once
#include<obliv_types.h>

typedef struct OblivBit {
  bool unknown; // Will be default initialized with memset(0), 
                //   so this field is 'unknown' rather than 'known'
                //   so that the default 0 means 'known'
  union {
    // a struct for each protocol we support goes here
    bool knownValue;
    struct {
      // FIXME Couldn't generator just XOR R with this on a NOT?
      // generator: w is label for 0 value if inverted == false, 1 otherwise
      // evaluator: w is current label
      yao_key_t w;
      union {
        bool value;    // used only by the prover in np protocol
        bool inverted; // inverted: generator use only
      };
      // npSetBitXor, npFlipBit uses assumes the two bools to be aliased
    } yao;
  };
} OblivBit;

// Dev warning: name clashes likely. Fix when it becomes a problem
// Java-style iterator over bits in OblivInput array, assumes all sizes > 0
typedef struct { int i,j; OblivInputs* oi; size_t n; } OIBitSrc;
static inline OIBitSrc oiBitSrc(OblivInputs* oi,size_t n) 
  { return (OIBitSrc){.i = 0, .j = 0, .oi = oi, .n = n}; }
static inline bool hasBit (OIBitSrc* s) { return s->i<s->n; }
static inline bool curBit (OIBitSrc* s) { return s->oi[s->i].src & (1<<s->j); }
static inline OblivBit* curDestBit(OIBitSrc* s) { return s->oi[s->i].dest+s->j; }
static inline void nextBit(OIBitSrc* s) 
  { if(++(s->j)>=s->oi[s->i].size) { s->j=0; ++(s->i); } }
static inline int bitCount(OIBitSrc* s) 
{ int res=0,i;
  for(i=0;i<s->n;++i) res+=s->oi[i].size;
  return res;
}

