// This file is a playground for benchmarking.
// Not to be used in production code
#include<assert.h>
#include<gcrypt.h>
#include<obliv_bits.h>
#include<obliv_common.h>

typedef struct 
{ int bytesPerAnd;
  char* buffer;
  YaoProtocolDesc* ypd; // need the fixed key cipher for crypto test
} NetStressProtocolDesc;

static void netStressFeedOblivBool(ProtocolDesc* pd,
    OblivBit* dest, int party, bool value)
{
  int me = ocCurrentParty();
  if(party==me) osend(pd,3-party,&value,sizeof(value));
  else orecv(pd,3-party,&value,sizeof(value));
  dest->unknown = true;
  dest->knownValue = value;
}

// Refactor with debug protocol TODO
static void netStressFeedOblivInputs(ProtocolDesc* pd,
    OblivInputs* spec,size_t count,int party)
{ while(count--)
  { int i;
    widest_t v = spec->src;
    for(i=0;i<spec->size;++i) 
    { netStressFeedOblivBool(pd,spec->dest+i,party,v&1);
      v>>=1;
    }
    spec++;
  }
}

static bool netStressRevealOblivBits
  (ProtocolDesc* pd,widest_t* dest,const OblivBit* src,size_t size,int party)
{
  if(party!=0 && party!=ocCurrentParty()) return false;
  src+=size;
  widest_t rv = 0;
  while(size-->0) rv = ((rv<<1)|!!(--src)->knownValue);
  *dest = rv;
  return true;
}

// I can't remember why I have all these casts to boolean with knownValue
// Come back and either remove casts or comment reason TODO
static void netStressSetBitXor(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (!!a->knownValue != !!b->knownValue);
  dest->unknown = true;
}

void yaoSetHalfMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,uint64_t k);

static void netStressNonFree(ProtocolDesc* pd,OblivBit* dest)
{
  NetStressProtocolDesc* nspd = pd->extra;
  dest->unknown = true;
  int i;
  bool res = dest->knownValue;
  assert(nspd->bytesPerAnd >=2*YAO_KEY_BYTES);
  yao_key_t mask,mask2;
  if(ocCurrentParty()==1)
  { for(i=0;i<nspd->bytesPerAnd;++i)
      nspd->buffer[i] = (dest->knownValue?0xff:0);
    osend(pd,1,nspd->buffer,nspd->bytesPerAnd);
    yaoSetHalfMask(nspd->ypd,mask,nspd->buffer,0);
    yaoSetHalfMask(nspd->ypd,mask2,nspd->buffer+YAO_KEY_BYTES,0);
    yaoSetHalfMask(nspd->ypd,mask,nspd->buffer,0);
    yaoSetHalfMask(nspd->ypd,mask2,nspd->buffer+YAO_KEY_BYTES,0);
  }else
  { orecv(pd,1,nspd->buffer,nspd->bytesPerAnd);
    yaoSetHalfMask(nspd->ypd,mask,nspd->buffer,0);
    yaoSetHalfMask(nspd->ypd,mask2,nspd->buffer+YAO_KEY_BYTES,0);
  }
  // useless dummy memory work
  for(i=0;i<YAO_KEY_BYTES && i<nspd->bytesPerAnd;++i)
    dest->yao.w[i] ^= nspd->buffer[i];
  dest->knownValue = res;
}

static void netStressSetBitAnd(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue = (a->knownValue && b->knownValue);
  netStressNonFree(pd,dest);
}
static void netStressSetBitOr(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue = (a->knownValue || b->knownValue);
  netStressNonFree(pd,dest);
}
// TODO refactor with debug protocol
static void netStressSetBitNot(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a)
{
  dest->knownValue= !a->knownValue;
  dest->unknown = a->unknown;
}
static void netStressFlipBit(ProtocolDesc* pd,OblivBit* dest) 
  { dest->knownValue = !dest->knownValue; }

void execNetworkStressProtocol(ProtocolDesc* pd, int bytecount,
                               protocol_run start, void* arg)
{
  NetStressProtocolDesc *nspd = malloc(sizeof(NetStressProtocolDesc));
  YaoProtocolDesc* ypd = malloc(sizeof(YaoProtocolDesc));
  nspd->bytesPerAnd = bytecount;
  nspd->buffer = malloc(bytecount);
  nspd->ypd = ypd;
  pd->currentParty = ocCurrentPartyDefault;
  pd->extra = nspd;
  pd->feedOblivInputs = netStressFeedOblivInputs;
  pd->revealOblivBits = netStressRevealOblivBits;
  pd->setBitAnd = netStressSetBitAnd;
  pd->setBitOr  = netStressSetBitOr;
  pd->setBitXor = netStressSetBitXor;
  pd->setBitNot = netStressSetBitNot;
  pd->flipBit   = netStressFlipBit;
  pd->partyCount= 2;
  ocSetCurrentProto(pd);
  gcryDefaultLibInit();
  gcry_cipher_open(&ypd->fixedKeyCipher,GCRY_CIPHER_AES128,
                   GCRY_CIPHER_MODE_ECB,0);
  start(arg);
  free(nspd->buffer);
  free(ypd);
  free(nspd);
}
