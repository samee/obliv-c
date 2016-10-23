#include<assert.h>
#include<stdlib.h>
#include<bcrandom.h>
#include<obliv_yao.h>
#include<obliv_types.h>
#include<obliv_types_internal.h>
#include<obliv_bits.h>
#include<obliv_common.h>
#include<obliv_copy_internal.h>

#include<stdio.h>
#define HERE printf("Party %d: %s:%d\n",p,__FILE__,__LINE__)

struct OcShareContext
{
#ifdef USE_PLAIN_OT
  OTsender sender;
  OTrecver recver;
#else
  struct HonestOTExtSender* sender;
  struct HonestOTExtRecver* recver;
#endif
  BCipherRandomGen *gen,*padder;
  size_t padnonce;
};

void ocShareInit(ProtocolDesc* pd)
{
  assert(*((char*)pd->extra)==OC_PD_TYPE_YAO);
  YaoProtocolDesc* ypd = pd->extra;
  assert(ypd->extra==NULL);  // If this fails, I am using shares
                             //   in a protocol that is not semi-honest Yao
  struct OcShareContext* ctx = malloc(sizeof(struct OcShareContext));
  dhRandomInit();
  int me = ocCurrentParty();
  int yu = 3-me;
  ctx->gen = newBCipherRandomGen();
  ctx->padder = newBCipherRandomGen();
  ctx->padnonce = 0;
#ifdef USE_PLAIN_OT
  if(me==1)
  { ctx->sender = honestOTExtSenderAbstract(honestOTExtSenderNew(pd,2));
    ctx->recver = honestOTExtRecverAbstract(honestOTExtRecverNew(pd,2));
  }else
  { ctx->recver = honestOTExtRecverAbstract(honestOTExtRecverNew(pd,1));
    ctx->sender = honestOTExtSenderAbstract(honestOTExtSenderNew(pd,1));
  }
#else
  if(me==1)
  { ctx->sender = honestOTExtSenderNew(pd,2);
    ctx->recver = honestOTExtRecverNew(pd,2);
  }else
  { ctx->recver = honestOTExtRecverNew(pd,1);
    ctx->sender = honestOTExtSenderNew(pd,1);
  }
#endif
  // TODO eventually I should move this to a dedicated
  // That would also allow proper splitting on multithreaded
  ypd->extra = ctx;
}
static inline struct OcShareContext* protoShareCtx(ProtocolDesc* pd)
{ YaoProtocolDesc* ypd = pd->extra;
  return ypd->extra;
}
// Would be nice if somebody called it before exiting protocol
void ocShareCleanup(ProtocolDesc* pd)
{
  YaoProtocolDesc* ypd = pd->extra;
  struct OcShareContext* ctx = ypd->extra;
#ifdef USE_PLAIN_OT
  otSenderRelease(&ctx->sender);
  otRecverRelease(&ctx->recver);
#else
  honestOTExtSenderRelease(ctx->sender);
  honestOTExtRecverRelease(ctx->recver);
#endif
  releaseBCipherRandomGen(ctx->gen);
  releaseBCipherRandomGen(ctx->padder);
  free(ctx);
  ypd->extra=NULL;
}

void ocShareXor(char* z,const char* x,const char* y,size_t n)
{ 
  if(z==y) { const char *t=x; x=y; y=t; }
  memmove(z,x,n);
  memxor(z,y,n);
}
struct CorrFunXorArgs {
  size_t len;
  char* mask;
};
// FIXME correlation callback signature
void corrFunXor(char* dest,const char* src,int i,void* vargs)
{
  struct CorrFunXorArgs* args = CAST(vargs);
  memcpy(dest,src,args->len);
  memxor(dest,args->mask+i*args->len,args->len);
}
void corrFunSameXor(char* dest,const char* src,int i,void* vargs)
{
  struct CorrFunXorArgs* args = CAST(vargs);
  memcpy(dest,src,args->len);
  memxor(dest,args->mask,args->len);
}

void genrGateToShare(ProtocolDesc* pd,struct OcShareContext* ctx,
    char* dest,char* delta,const __obliv_c__bool* wc,
    size_t n,size_t eltsize)
{
  const int klen = ctx->padder->klen, bufsz = n*eltsize;
  const int ninc = (eltsize+ctx->padder->blen-1)/ctx->padder->blen;
  char keyx[klen];
  int i, ksz = (YAO_KEY_BYTES<klen?YAO_KEY_BYTES:klen);
  char *itembuf = malloc(bufsz);
  memset(keyx+ksz,0,klen-YAO_KEY_BYTES);
  for(i=0;i<n;++i)
  { bool b;
    if(__obliv_c__bitIsKnown(&b,wc[i].bits))
    { randomizeBuffer(ctx->gen,dest+i*eltsize,eltsize);
      memcpy(itembuf+i*eltsize,dest+i*eltsize,eltsize);
      if(b) memxor(itembuf+i*eltsize,delta+i*eltsize,eltsize);
    }else
    { const char *w = wc[i].bits[0].yao.w;
      memcpy(keyx,w,ksz);
      bool v = yaoKeyLsb(w);
      // Note I do not need to use w->inverted. It is already used
      // in the generator's choice bits when doing OTs later.

      resetBCipherRandomGen(ctx->padder,keyx);
      setctrFromIntBCipherRandomGen(ctx->padder,ctx->padnonce);
      randomizeBuffer(ctx->padder,(v?itembuf:dest)+i*eltsize,eltsize);

      memxor(keyx,((YaoProtocolDesc*)pd->extra)->R,ksz);
      resetBCipherRandomGen(ctx->padder,keyx);
      setctrFromIntBCipherRandomGen(ctx->padder,ctx->padnonce);
      randomizeBuffer(ctx->padder,(v?dest:itembuf)+i*eltsize,eltsize);

      memxor(itembuf+i*eltsize,dest+i*eltsize,eltsize);
      memxor(itembuf+i*eltsize,delta+i*eltsize,eltsize);

      ctx->padnonce+=ninc;
    }
  }
  osend(pd,2,itembuf,bufsz);
  free(itembuf);
}
void evalGateToShare(ProtocolDesc* pd,struct OcShareContext* ctx,char* dest,
    const __obliv_c__bool* wc,size_t n,size_t eltsize)
{
  const int klen = ctx->padder->klen, bufsz = n*eltsize;
  const int ninc = (eltsize+ctx->padder->blen-1)/ctx->padder->blen;
  char keyx[klen];
  int i, ksz = (YAO_KEY_BYTES<klen?YAO_KEY_BYTES:klen);
  char *itembuf = malloc(eltsize);
  memset(keyx+ksz,0,klen-YAO_KEY_BYTES);
  orecv(pd,1,dest,bufsz);
  for(i=0;i<n;++i)
  { bool b;
    if(!__obliv_c__bitIsKnown(&b,wc[i].bits))
    {
      memcpy(keyx,wc[i].bits[0].yao.w,ksz);
      resetBCipherRandomGen(ctx->padder,keyx);
      setctrFromIntBCipherRandomGen(ctx->padder,ctx->padnonce);
      randomizeBuffer(ctx->padder,itembuf,eltsize);
      if(yaoKeyLsb(keyx)) memxor(dest+i*eltsize,itembuf,eltsize);
      else memcpy(dest+i*eltsize,itembuf,eltsize);
      ctx->padnonce+=ninc;
    }// else: keep received data in dest as is
  }
  free(itembuf);
}
// z = x0 or x1, based on y, size n*eltsize
// x0,x1 = input,  size n*eltsize
// c = control bits, size n 
// wc = control bits in yao wire form
// t = scratch memory, size 4n*eltsize
void ocShareMuxes(ProtocolDesc* pd,char* z,     
                  const char* x0,const char* x1,size_t n,size_t eltsize,
                  const bool* c,const __obliv_c__bool* wc,char* t)
{
  const size_t bufsz = n*eltsize;
  char *t2 = t+bufsz, *t3 = t+2*bufsz, *tr = t+3*bufsz;
  struct OcShareContext* ctx = protoShareCtx(pd);
  int i;
  int p = protoCurrentParty(pd);
  memcpy(t2,x0,bufsz); memxor(t2,x1,bufsz); // t2 = x0^x1
#ifdef USE_PLAIN_OT
  randomizeBuffer(ctx->gen,t,bufsz);
  memxor(t2,t,bufsz);
  if(protoCurrentParty(pd)==1)
  { ctx->sender.send(ctx->sender.sender,t,t2,n,eltsize);
    ctx->recver.recv(ctx->recver.recver,tr,c,n,eltsize);
  }else
  { ctx->recver.recv(ctx->recver.recver,tr,c,n,eltsize);
    ctx->sender.send(ctx->sender.sender,t,t2,n,eltsize);
  }
#else
  struct CorrFunXorArgs a = {.len=eltsize,.mask=t2};
  if(protoCurrentParty(pd)==1)
  { genrGateToShare(pd,ctx,t,t2,wc,n,eltsize);
    //honestCorrelatedOTExtSend1Of2(ctx->sender,t,t3,n,eltsize,corrFunXor,&a);
    honestCorrelatedOTExtRecv1Of2(ctx->recver,tr,c,n,eltsize);
  }else
  { evalGateToShare(pd,ctx,tr,wc,n,eltsize);
    //honestCorrelatedOTExtRecv1Of2(ctx->recver,tr,c,n,eltsize);
    honestCorrelatedOTExtSend1Of2(ctx->sender,t,t3,n,eltsize,corrFunXor,&a);
  }
#endif
  for(i=0;i<n;++i) memmove(z+i*eltsize,(c[i]?x1:x0)+i*eltsize,eltsize);
  memxor(z,tr,bufsz);
  memxor(z,t ,bufsz);
  /*
   * if sender:
   *   t = random
   *   t2 += t
   *   send(t,t2)
   *   z = t+(x0+x1)*c[i]+x0 // won't need x0,x1 again
   *   t = recv(c)
   *   z += t
   * if recver:
   *   t = recv(c) // r'+(x0'+x1')*c
   *   z = x0+t+c[i]*t2 // x0, x1 gone
   *   t = random
   *   t2 += t
   *   send(t,t2)
   *   z += t
   */
}

// ------------------------- Glue with Obliv-C ------------------------------

static bool oblivBitLSB(int me,const OblivBit* bit)
{
  if(!bit->unknown)
  { if(me==1) return false;
    else return bit->knownValue;
  }else return yaoKeyLsb(bit->yao.w) !=( ( me == 1) && (bit->yao.inverted) );
}

bool ocOBoolLSB(int me,__obliv_c__bool b)
{
  return oblivBitLSB(me,&b.bits[0]);
}
void unpackBools(bool* dest, size_t n, const char* src)
{
  int i,j;
  for(i=0;i<(n+7)/8;++i) for(j=0;j<8 && i*8+j<n;++j)
    dest[i*8+j]=((src[i]>>j)&1);
}
// For each bit of party 2, we do OT with it
// For each bit of party 1, we play with the "inverted" flag
// We have to go on the C-side of things to unwrap these obliv ints
// I considered using a feedXor type interface, but decided against it
// since this will almost always be used with large arrays
// TODO list params
void ocFromShared_impl(ProtocolDesc* pd,
                       void* dest,const void* src,size_t n,
                       size_t bits,size_t bytes)
{
  // How much scratch space do I need?
  // Let's go with malloc for now, we change that later.
  struct OcShareContext* ctx = protoShareCtx(pd);
  YaoProtocolDesc* ypd = pd->extra;
  int i,j,p = pd->currentParty(pd);
  if(p==1)
  {
    yao_key_t *key0 = malloc(n*bits*YAO_KEY_BYTES);
    yao_key_t *key1 = malloc(n*bits*YAO_KEY_BYTES);
#ifdef USE_PLAIN_OT
    for(i=0;i<n*bits;++i) yaoKeyNewPair(ypd,key0[i],key1[i]);
    ctx->sender.send(ctx->sender.sender,
                     (char*)key0,(char*)key1,n*bits,YAO_KEY_BYTES);
#else
    struct CorrFunXorArgs a = {.len=YAO_KEY_BYTES,.mask=ypd->R};
    honestCorrelatedOTExtSend1Of2(ctx->sender,
        (char*)key0,(char*)key1,n*bits,YAO_KEY_BYTES,corrFunSameXor,&a);
    ypd->icount+=n*bits;
#endif
    for(i=0;i<n;++i)
    { OblivBit* dbit = __obliv_c__bits(i*bytes+(char*)dest);
      for(j=0;j<bits;++j)
      { dbit[j].unknown = true;
        dbit[j].yao.inverted = false;
        size_t bpos = i*bits+j;
        yao_key_t *key = ((((const char*)src)[bpos/8]>>(bpos%8))&1)?key1:key0;
        yaoKeyCopy(dbit[j].yao.w,key[bpos]);
      }
    }
    free(key0); free(key1);

  }else
  {
    yao_key_t *key = malloc(n*bits*YAO_KEY_BYTES);
    bool *sel = malloc(n*bits*sizeof(bool));
    unpackBools(sel,n*bits,src);
#ifdef USE_PLAIN_OT
    ctx->recver.recv(ctx->recver.recver,(char*)key,sel,n*bits,YAO_KEY_BYTES);
#else
    honestCorrelatedOTExtRecv1Of2(ctx->recver,(char*)key,sel,n*bits,
        YAO_KEY_BYTES);
#endif
    ypd->icount+=n*bits;
    for(i=0;i<n;++i)
    { OblivBit* dbit = __obliv_c__bits(i*bytes+(char*)dest);
      for(j=0;j<bits;++j) 
      { dbit[j].unknown = true;
        yaoKeyCopy(dbit[j].yao.w,key[i*bits+j]);
      }
    } 
    free(key);
    free(sel);
  }
}
void ocRevealShared_impl(ProtocolDesc* pd,void* dest,
                         const void* src,size_t sz,int party)
{
  int me = protoCurrentParty(pd);
  if(me==1)
  { if(party!=1) osend(pd,2,src,sz);
    if(party!=2)
    { orecv(pd,2,dest,sz);
      memxor(dest,src,sz);
    }
  }else
  { if(party!=1) orecv(pd,1,dest,sz);
    if(party!=2)
    { osend(pd,1,src,sz);
      memxor(dest,src,sz);
    }
  }
}

#define OC_TO_SHARED_TYPE(ot,t,T) \
  void ocToShared##T##N(ProtocolDesc* pd, \
                        char dest[][ocBitSize(ot)/8],const ot src[],size_t n) \
{ \
  int i,j,k,me = protoCurrentParty(pd); \
  assert(ocBitSize(ot)%8==0); \
  for(i=0;i<n;++i) \
  { const ot* x = &src[i];\
    for(j=0;j<ocBitSize(*x)/8;++j)\
    { char r=0; \
      for(k=7;k>=0;--k) \
        r = ((r<<1)|oblivBitLSB(me,&x->bits[8*j+k])); \
      dest[i][j]=r; \
    } \
  } \
}\
void \
ocFeedShared##T##N (char dest[][ocBitSize(ot)/8],\
                    const t src[],size_t n,int party) \
{\
  int me = ocCurrentParty();\
  assert(sizeof(dest[0])==sizeof(src[0])); \
  if(me==party) memcpy(dest,src,sizeof(dest[0])*n); \
  else memset(dest,0,sizeof(dest[0])*n); \
}\
void \
ocRevealShared##T##N (t dest[], \
                      const char src[][ocBitSize(ot)/8], \
                      size_t n,int party) \
{ \
  assert(sizeof(dest[0])==sizeof(src[0])); \
  ocRevealShared_impl(ocCurrentProto(),dest,src,n*sizeof(t),party);\
}
OC_TO_SHARED_TYPE(__obliv_c__char,char,Char)
OC_TO_SHARED_TYPE(__obliv_c__short,short,Short)
OC_TO_SHARED_TYPE(__obliv_c__int,int,Int)
OC_TO_SHARED_TYPE(__obliv_c__long,long,Long)
OC_TO_SHARED_TYPE(__obliv_c__lLong,long long,LLong)
