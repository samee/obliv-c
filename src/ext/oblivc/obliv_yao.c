#include <obliv_common.h>
#include <obliv_bits.h>
#include <obliv_yao.h>
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

//-------------------- Yao Protocol (honest but curious) -------------

static pthread_once_t gcryInitDone = PTHREAD_ONCE_INIT;

GCRY_THREAD_OPTION_PTHREAD_IMPL;

static void gcryDefaultLibInitAux(void)
{
  if(!gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))
  { 
    gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
    if(!gcry_check_version(NULL))
    { fprintf(stderr,"libgcrypt init failed\n");
      exit(1);
    }
    gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
  }
}
void gcryDefaultLibInit(void)
  { pthread_once(&gcryInitDone,gcryDefaultLibInitAux); }

// Assume: generator is party 1, evaluator is party 2

/*
static void yaoKeyDebug(const yao_key_t k)
{ int i;
  fprintf(stderr,"%d: ",currentProto->thisParty);
  for(i=YAO_KEY_BYTES-1;i>=0;--i) fprintf(stderr,"%02x ",0xff&k[i]);
  fprintf(stderr,"\n");
}
static void debugOblivBit(const OblivBit* o)
{
  if(known(o)) fprintf(stderr,"Known value %d\n",(int)o->knownValue);
  else 
  { fprintf(stderr,"inv %d, ",(int)o->yao.inverted);
    yaoKeyDebug(o->yao.w);
  }
}
*/

const char yaoFixedKey[] = "\x61\x7e\x8d\xa2\xa0\x51\x1e\x96"
                           "\x5e\x41\xc2\x9b\x15\x3f\xc7\x7a";
const int yaoFixedKeyAlgo = GCRY_CIPHER_AES128;
#define FIXED_KEY_BLOCKLEN 16

// Finite field doubling: used in fixed key garbling
void yaoKeyDouble(yao_key_t d)
{
#if YAO_KEY_BYTES==10
  char tmp = (d[7] >> 7) & 1;
  char tmp2 = (d[9] >> 7) & 1;
  ((uint64_t *)d)[0]<<=1;
  ((uint16_t *)d)[4]<<=1;
  ((uint16_t *)d)[4]|=tmp;
   d[0] ^= ((tmp2<<1) | tmp2);
#else
  char carry = 0, next;
  int i;
  for(i=0;i<YAO_KEY_BYTES;++i)
  { next = (d[i] >> 7);
    d[i] = ((d[i]<<1)|carry);
    carry = next;
  }
   if(next == 1)
  d[0] ^= 0x03;
#endif
}

// Remove old SHA routines?
//#define DISABLE_FIXED_KEY
#ifdef DISABLE_FIXED_KEY
// d = H(a,k), used by half gate scheme
void yaoSetHalfMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,uint64_t k)
{
  char buf[YAO_KEY_BYTES+8], dest[20]; // dest length = DIGEST length
  memcpy(buf,a,YAO_KEY_BYTES);
  memcpy(buf+YAO_KEY_BYTES,&k,sizeof(k));
  gcry_md_hash_buffer(GCRY_MD_SHA1,dest,buf,sizeof(buf));
  memcpy(d,dest,YAO_KEY_BYTES);
}
// XXX do I need i when it is already encoded in lsb(a)||lsb(b)
void yaoSetHashMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,const yao_key_t b,
                    uint64_t k,int i)
{
  char buf[2*YAO_KEY_BYTES+8], dest[20];  // dest length == DIGEST length
  k=((k<<2)|i);
  memcpy(buf,a,YAO_KEY_BYTES);
  memcpy(buf+YAO_KEY_BYTES,b,YAO_KEY_BYTES);
  memcpy(buf+2*YAO_KEY_BYTES,&k,8);
  gcry_md_hash_buffer(GCRY_MD_SHA1,dest,buf,sizeof(buf));
  memcpy(d,dest,YAO_KEY_BYTES);
}
#else
// d = H(a,k), used by half gate scheme
void yaoSetHalfMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,uint64_t k)
{
  char  buf[FIXED_KEY_BLOCKLEN];
  char obuf[FIXED_KEY_BLOCKLEN]; // buf length >= YAO_KEY_BYTES
  int i;
  assert(YAO_KEY_BYTES<=FIXED_KEY_BLOCKLEN);
  for(i=YAO_KEY_BYTES;i<FIXED_KEY_BLOCKLEN;++i) buf[i]=0;
  yaoKeyCopy(buf,a); yaoKeyDouble(buf);
  for(i=0;i<sizeof(k);++i) buf[i]^=((k>>8*i)&0xff);
  gcry_cipher_encrypt(ypd->fixedKeyCipher,obuf,sizeof(obuf),buf,sizeof(buf));
  yaoKeyCopy(d,obuf);
  yaoKeyXor(d,buf);
}
// Same as yaoSetHalfMask(ypd,d1,a1,k); yaoSetHalfMask(ypd,d2,a2,k)
// Uses a single call to gcry_cipher_encrypt, which is faster.
// Eliminate a redundant memcpy if
//      YAO_KEY_BYTES == FIXED_KEY_BLOCKLEN
//      and d1==d2+FIXED_KEY_BLOCKLEN
void yaoSetHalfMask2(YaoProtocolDesc* ypd,
                     yao_key_t d1, const yao_key_t a1,
                     yao_key_t d2, const yao_key_t a2, uint64_t k)
{
  char buf[2*FIXED_KEY_BLOCKLEN];
  char *obuf;
  const int blen=FIXED_KEY_BLOCKLEN;
  int i,j;
  assert(YAO_KEY_BYTES<=FIXED_KEY_BLOCKLEN);

  if(YAO_KEY_BYTES==blen && d2==d1+YAO_KEY_BYTES)
    obuf=d1; // eliminate redundant yaoKeyCopy later
  else obuf=alloca(2*blen);

  memset(buf+YAO_KEY_BYTES, 0, FIXED_KEY_BLOCKLEN-YAO_KEY_BYTES);
  memset(buf+YAO_KEY_BYTES+FIXED_KEY_BLOCKLEN, 0, FIXED_KEY_BLOCKLEN-YAO_KEY_BYTES);
  yaoKeyCopy(buf     ,a1); yaoKeyDouble(buf);
  yaoKeyCopy(buf+blen,a2); yaoKeyDouble(buf+blen);
  for(i=0;i<sizeof(k);++i) for(j=0;j<2;++j) buf[i+j*blen]^=((k>>8*i)&0xff);

  gcry_cipher_encrypt(ypd->fixedKeyCipher,obuf,2*blen,buf,2*blen);
  if(obuf!=d1) { yaoKeyCopy(d1,obuf); yaoKeyCopy(d2,obuf+blen); }
  yaoKeyXor(d1,buf); yaoKeyXor(d2,buf+blen);
}
void yaoSetHashMask(YaoProtocolDesc* ypd,
                    yao_key_t d,const yao_key_t a,const yao_key_t b,
                    uint64_t k,int ii)
{
  char  buf[FIXED_KEY_BLOCKLEN];
  char obuf[FIXED_KEY_BLOCKLEN];
  int i;
  k=4*k+ii;
  assert(YAO_KEY_BYTES<=FIXED_KEY_BLOCKLEN);
  for(i=YAO_KEY_BYTES;i<FIXED_KEY_BLOCKLEN;++i) buf[i]=0;
  yaoKeyCopy(buf,a); yaoKeyDouble(buf);
  yaoKeyXor (buf,b); yaoKeyDouble(buf);
  for(i=0;i<sizeof(k);++i) buf[i]^=((k>>8*i)&0xff);
  gcry_cipher_encrypt(ypd->fixedKeyCipher,obuf,sizeof(obuf),buf,sizeof(buf));
  yaoKeyCopy(d,obuf);
  yaoKeyXor(d,buf);
}
#endif

// Why am I using SHA1 as a PRG? Not necessarily safe. TODO change to AES
void yaoKeyNewPair(YaoProtocolDesc* pd,yao_key_t w0,yao_key_t w1)
{
  unsigned* ic = &pd->icount;
  char buf[YAO_KEY_BYTES+sizeof(*ic)], dest[20];
  memcpy(buf,pd->I,YAO_KEY_BYTES);
  memcpy(buf+YAO_KEY_BYTES,ic,sizeof(*ic));
  (*ic)++;
  gcry_md_hash_buffer(GCRY_MD_SHA1,dest,buf,sizeof(buf));
  memcpy(w0,dest,YAO_KEY_BYTES);
  yaoKeyCopy(w1,w0);
  yaoKeyXor(w1,pd->R);
}

void yaoSetBitAnd(ProtocolDesc* pd,OblivBit* r,
                  const OblivBit* a,const OblivBit* b)
  { ((YaoProtocolDesc*)pd->extra)->nonFreeGate(pd,r,0x8,a,b); }
void yaoSetBitOr(ProtocolDesc* pd,OblivBit* r,
                 const OblivBit* a,const OblivBit* b)
  { ((YaoProtocolDesc*)pd->extra)->nonFreeGate(pd,r,0xE,a,b); }
void yaoSetBitXor(ProtocolDesc* pd,OblivBit* r,
                 const OblivBit* a,const OblivBit* b)
{
  OblivBit t;
  yaoKeyCopy(t.yao.w,a->yao.w);
  yaoKeyXor (t.yao.w,b->yao.w);
  t.yao.inverted = (a->yao.inverted!=b->yao.inverted); // no-op for evaluator
  t.unknown = true;
  *r = t;
}
void yaoFlipBit(ProtocolDesc* pd,OblivBit* r)
  { r->yao.inverted = !r->yao.inverted; }
void yaoSetBitNot(ProtocolDesc* pd,OblivBit* r,const OblivBit* a)
  { *r = *a; yaoFlipBit(pd,r); }

#define YAO_FEED_MAX_BATCH 1000000

void yaoGenrFeedOblivInputs(ProtocolDesc* pd
                           ,OblivInputs* oi,size_t n,int src)
{ 
  YaoProtocolDesc* ypd = pd->extra;
  yao_key_t w0,w1;
  OIBitSrc it = oiBitSrc(oi,n);
  if(src==1) for(;hasBit(&it);nextBit(&it))
  { OblivBit* o = curDestBit(&it);
    yaoKeyNewPair(ypd,w0,w1); // does ypd->icount++
    if(curBit(&it)) osend(pd,2,w1,YAO_KEY_BYTES);
    else osend(pd,2,w0,YAO_KEY_BYTES);
    o->yao.inverted = false; o->unknown = true;
    yaoKeyCopy(o->yao.w,w0);
  }else 
  { int bc = bitCount(&it);
    // limit memory usage
    int batch = (bc<YAO_FEED_MAX_BATCH?bc:YAO_FEED_MAX_BATCH);
    char *buf0 = malloc(batch*YAO_KEY_BYTES),
         *buf1 = malloc(batch*YAO_KEY_BYTES);
    int bp=0;
    for(;hasBit(&it);nextBit(&it))
    { 
      OblivBit* o = curDestBit(&it);
      yaoKeyNewPair(ypd,w0,w1); // does ypd->icount++
      yaoKeyCopy(buf0+bp*YAO_KEY_BYTES,w0);
      yaoKeyCopy(buf1+bp*YAO_KEY_BYTES,w1);
      o->yao.inverted = false; o->unknown = true;
      yaoKeyCopy(o->yao.w,w0);
      ++bp;
      if(bp>=batch) // flush out every now and then
      { ypd->sender.send(ypd->sender.sender,buf0,buf1,bp,YAO_KEY_BYTES);
        bp=0;
      }
    }
    if(bp) ypd->sender.send(ypd->sender.sender,buf0,buf1,bp,YAO_KEY_BYTES);
    free(buf0); free(buf1);
  }
}
void yaoEvalFeedOblivInputs(ProtocolDesc* pd
                           ,OblivInputs* oi,size_t n,int src)
{ OIBitSrc it = oiBitSrc(oi,n);
  YaoProtocolDesc* ypd = pd->extra;
  if(src==1) for(;hasBit(&it);nextBit(&it))
  { OblivBit* o = curDestBit(&it);
    orecv(pd,1,o->yao.w,YAO_KEY_BYTES);
    o->unknown = true;
    ypd->icount++;
  }else 
  { int bc = bitCount(&it);
    // limit memory usage
    int batch = (bc<YAO_FEED_MAX_BATCH?bc:YAO_FEED_MAX_BATCH);
    char *buf = malloc(batch*YAO_KEY_BYTES),
         **dest = malloc(batch*sizeof(char*));
    bool *sel = malloc(batch*sizeof(bool));
    int bp=0,i;
    for(;hasBit(&it);nextBit(&it))
    { OblivBit* o = curDestBit(&it);
      dest[bp]=o->yao.w;
      sel[bp]=o->yao.value=curBit(&it);
      o->unknown = true; // Known to me, but not to both parties
      ypd->icount++;
      ++bp;
      if(bp>=batch)
      { ypd->recver.recv(ypd->recver.recver,buf,sel,bp,YAO_KEY_BYTES);
        for(i=0;i<bp;++i) yaoKeyCopy(dest[i],buf+i*YAO_KEY_BYTES);
        bp=0;
      }
    }
    if(bp) // flush out every now and then
    { ypd->recver.recv(ypd->recver.recver,buf,sel,bp,YAO_KEY_BYTES);
      for(i=0;i<bp;++i) yaoKeyCopy(dest[i],buf+i*YAO_KEY_BYTES);
    }
    free(buf); free(dest); free(sel);
  }
}

bool yaoGenrRevealOblivBits(ProtocolDesc* pd,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  YaoProtocolDesc *ypd = pd->extra;
  for(i=0;i<n;++i) if(o[i].unknown)
    flipflags |= ((yaoKeyLsb(o[i].yao.w) != o[i].yao.inverted)?1LL<<i:0);
  // Assuming little endian
  if(party != 1) osend(pd,2,&flipflags,bc);
  if(party != 2) { orecv(pd,2,&rv,bc); rv^=flipflags; }
  for(i=0;i<n;++i) if(!o[i].unknown && o[i].knownValue)
    rv |= (1LL<<i);
  ypd->ocount+=n;
  if(party!=2) { *dest=rv; return true; }
  else return false;
}
bool yaoEvalRevealOblivBits(ProtocolDesc* pd,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  YaoProtocolDesc* ypd = pd->extra;
  for(i=0;i<n;++i) if(o[i].unknown)
    flipflags |= (yaoKeyLsb(o[i].yao.w)?1LL<<i:0);
  // Assuming little endian
  if(party != 1) { orecv(pd,1,&rv,bc); rv^=flipflags; }
  if(party != 2) osend(pd,1,&flipflags,bc);
  for(i=0;i<n;++i) if(!o[i].unknown && o[i].knownValue)
    rv |= (1LL<<i);
  ypd->ocount+=n;
  if(party!=1) { *dest=rv; return true; }
  else return false;
}

// Encodes a 2-input truth table for f(a,b) = (bool)(ttable&(1<<(2*a+b)))
void yaoGenerateGate(ProtocolDesc* pd, OblivBit* r, char ttable, 
    const OblivBit* a, const OblivBit* b)
{ 
  YaoProtocolDesc* ypd = pd->extra;
  uint64_t k = ypd->gcount;
  int im=0,i;
  yao_key_t wa,wb,wc,wt;
  const char* R = ypd->R;

  // adjust truth table according to invert fields (faster with im^=...) TODO
  if(a->yao.inverted) ttable = (((ttable&3)<<2)|((ttable>>2)&3));
  if(b->yao.inverted) ttable = (((ttable&5)<<1)|((ttable>>1)&5));

  // Doing garbled-row-reduction: wc is set to first mask
  yaoKeyCopy(wa,a->yao.w);
  yaoKeyCopy(wb,b->yao.w);
  if(yaoKeyLsb(wa)) { im^=2; yaoKeyXor(wa,R); }
  if(yaoKeyLsb(wb)) { im^=1; yaoKeyXor(wb,R); }

  // Create labels for new wire in wc
  yaoSetHashMask(ypd,wc,wa,wb,k,0);
  if(ttable&(1<<im)) yaoKeyXor(wc,R);

  for(i=1;i<4;++i) // skip 0, because GRR
  { yaoKeyXor(wb,R);
    if(i==2) yaoKeyXor(wa,R);
    yaoSetHashMask(ypd,wt,wa,wb,k,i);
    yaoKeyXor(wt,wc);
    if(ttable&(1<<(i^im))) yaoKeyXor(wt,R);
    osend(pd,2,wt,YAO_KEY_BYTES);
  }

  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,wc);
  r->unknown = true; r->yao.inverted = false;
  ypd->gcount++;
}

void yaoEvaluateGate(ProtocolDesc* pd, OblivBit* r, char ttable, 
  const OblivBit* a, const OblivBit* b)
{
  int i=0,j;
  YaoProtocolDesc* ypd = pd->extra;
  yao_key_t w,t;
  yaoKeyZero(t);
  if(yaoKeyLsb(a->yao.w)) i^=2;
  if(yaoKeyLsb(b->yao.w)) i^=1;
  // I wonder: can the generator do timing attacks here?
  if(i==0) yaoKeyCopy(w,t);
  for(j=1;j<4;++j)
  { orecv(pd,1,t,sizeof(yao_key_t));
    if(i==j) yaoKeyCopy(w,t);
  }
  yaoSetHashMask(ypd,t,a->yao.w,b->yao.w,ypd->gcount++,i);
  yaoKeyXor(w,t);
  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,w);
  r->unknown = true;
}

// dest = src + cond * ypd->R. Who knows if this creates timing channels
void yaoKeyCondXor(yao_key_t dest, bool cond,
                   const yao_key_t a, const yao_key_t b)
{
   yao_key_t tmp;
   memset(tmp, cond?-1:0, YAO_KEY_BYTES);
 size_t i;
  for(i=0;i+sizeof(uint64_t)<=YAO_KEY_BYTES;i+=sizeof(uint64_t))
    *((uint64_t*)((char*)dest+i)) =  ( *((uint64_t*)((char*)tmp+i)) & *((uint64_t*)((char*)b+i)) ) ^ *((uint64_t*)((char*)a+i));
  if(i+sizeof(uint32_t)<=YAO_KEY_BYTES){
    *((uint32_t*)((char*)dest+i)) =  ( *((uint32_t*)((char*)tmp+i)) & *((uint32_t*)((char*)b+i)) ) ^ *((uint32_t*)((char*)a+i));
   i+=sizeof(uint32_t);
  }
  if(i+sizeof(uint16_t)<=YAO_KEY_BYTES)  {
    *((uint16_t*)((char*)dest+i)) =  ( *((uint16_t*)((char*)tmp+i)) & *((uint16_t*)((char*)b+i)) ) ^ *((uint16_t*)((char*)a+i));
   i+=sizeof(uint16_t);
  }
  if(i < YAO_KEY_BYTES)
    *(((char*)dest+i)) =  ( *(((char*)tmp+i)) & *(((char*)b+i)) ) ^ *(((char*)a+i));
}

// Computes r = (a xor ac)(b xor bc) xor rc
void yaoGenerateHalfGatePair(ProtocolDesc* pd, OblivBit* r,
    bool ac, bool bc, bool rc, const OblivBit* a, const OblivBit* b)
{
  YaoProtocolDesc* ypd = pd->extra;
  if(a->yao.inverted) ac=!ac;
  if(b->yao.inverted) bc=!bc;

  bool pa = yaoKeyLsb(a->yao.w), pb = yaoKeyLsb(b->yao.w);
  yao_key_t row,t,wg,we,wa1,wb1;
  const char *wa0 = a->yao.w, *wb0 = b->yao.w;

  yaoKeyCopy(wa1,wa0); yaoKeyXor(wa1,ypd->R);
  yaoKeyCopy(wb1,wb0); yaoKeyXor(wb1,ypd->R);

  //yaoSetHalfMask(ypd,row,wa0,ypd->gcount);
  //yaoSetHalfMask(ypd,t  ,wa1,ypd->gcount);
  yaoSetHalfMask2(ypd,row,wa0,t,wa1,ypd->gcount);
  yaoKeyCopy(wg,(pa?t:row));
  yaoKeyXor (row,t);
  yaoKeyCondXor(row,(pb!=bc),row,ypd->R);
  osend(pd,2,row,YAO_KEY_BYTES);
  yaoKeyCondXor(wg,((pa!=ac)&&(pb!=bc))!=rc,wg,ypd->R);
  ypd->gcount++;

  //yaoSetHalfMask(ypd,row,wb0,ypd->gcount);
  //yaoSetHalfMask(ypd,t  ,wb1,ypd->gcount);
  yaoSetHalfMask2(ypd,row,wb0,t,wb1,ypd->gcount);
  yaoKeyCopy(we,(pb?t:row));
  yaoKeyXor (row,t);
  yaoKeyXor (row,(ac?wa1:wa0));
  osend(pd,2,row,YAO_KEY_BYTES);
  ypd->gcount++;

  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,wg);
  yaoKeyXor (r->yao.w,we);

  r->yao.inverted = false; r->unknown = true;
}

void yaoGenerateAndPair(ProtocolDesc* pd, OblivBit* r, 
                        const OblivBit* a, const OblivBit* b)
  { yaoGenerateHalfGatePair(pd,r,0,0,0,a,b); }

void yaoGenerateOrPair(ProtocolDesc* pd, OblivBit* r,
                       const OblivBit* a, const OblivBit* b)
  { yaoGenerateHalfGatePair(pd,r,1,1,1,a,b); }

// TODO merge these too? Maybe I paired the wrong way
void yaoEvaluateHalfGatePair(ProtocolDesc* pd, OblivBit* r,
    const OblivBit* a, const OblivBit* b)
{
  YaoProtocolDesc* ypd = pd->extra;
  yao_key_t row,t,wg,we;

  yaoSetHalfMask(ypd,t,a->yao.w,ypd->gcount++);
  orecv(pd,1,row,YAO_KEY_BYTES);
  yaoKeyCondXor(wg,yaoKeyLsb(a->yao.w),t,row);

  yaoSetHalfMask(ypd,t,b->yao.w,ypd->gcount++);
  orecv(pd,1,row,YAO_KEY_BYTES);
  yaoKeyXor(row,a->yao.w);
  yaoKeyCondXor(we,yaoKeyLsb(b->yao.w),t,row);

  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,wg);
  yaoKeyXor (r->yao.w,we);

  r->unknown = true;
}

uint64_t yaoGateCount()
{ uint64_t rv = ((YaoProtocolDesc*)currentProto->extra)->gcount;
  if(currentProto->setBitAnd==yaoGenerateAndPair
      || currentProto->setBitAnd==yaoEvaluateHalfGatePair) // halfgate
    return rv/2;
  else return rv;
}

// FIXME don't like this convention: OT should have used transport
// objects directly, instead of being wrapped in ProtocolDesc
void yaoUseNpot(ProtocolDesc* pd,int me)
{ YaoProtocolDesc* ypd = pd->extra;
  if(me==1) ypd->sender =
    npotSenderAbstract(npotSenderNew(1<<NPOT_BATCH_SIZE,pd,2));
  else ypd->recver =
    npotRecverAbstract(npotRecverNew(1<<NPOT_BATCH_SIZE,pd,1));
}
// Used with yaoUseNpot
void yaoReleaseOt(ProtocolDesc* pd,int me)
{ YaoProtocolDesc* ypd = pd->extra;
  if(me==1) otSenderRelease(&ypd->sender);
  else otRecverRelease(&ypd->recver);
}
/* execYaoProtocol is divided into 2 parts which are reused by other
   protocols such as DualEx */
void setupYaoProtocol(ProtocolDesc* pd,bool halfgates)
{
  YaoProtocolDesc* ypd = malloc(sizeof(YaoProtocolDesc));
  int me = pd->thisParty;
  pd->extra = ypd;
  pd->error = 0;
  ypd->protoType = OC_PD_TYPE_YAO;
  ypd->extra = NULL;
  pd->partyCount = 2;
  pd->currentParty = ocCurrentPartyDefault;
  pd->feedOblivInputs = (me==1?yaoGenrFeedOblivInputs:yaoEvalFeedOblivInputs);
  pd->revealOblivBits = (me==1?yaoGenrRevealOblivBits:yaoEvalRevealOblivBits);
  if(halfgates)
  { pd->setBitAnd = (me==1?yaoGenerateAndPair:yaoEvaluateHalfGatePair);
    pd->setBitOr  = (me==1?yaoGenerateOrPair :yaoEvaluateHalfGatePair);
  }else
  { ypd->nonFreeGate = (me==1?yaoGenerateGate:yaoEvaluateGate);
    pd->setBitAnd = yaoSetBitAnd;
    pd->setBitOr  = yaoSetBitOr;
  }
  pd->setBitXor = yaoSetBitXor;
  pd->setBitNot = yaoSetBitNot;
  pd->flipBit   = yaoFlipBit;

  if(pd->thisParty==1) ypd->sender.sender=NULL;
  else ypd->recver.recver=NULL;

  dhRandomInit();
  gcry_cipher_open(&ypd->fixedKeyCipher,yaoFixedKeyAlgo,GCRY_CIPHER_MODE_ECB,0);
  gcry_cipher_setkey(ypd->fixedKeyCipher,yaoFixedKey,sizeof(yaoFixedKey)-1);
}

// point_and_permute should always be true.
// It is false only in the NP protocol, where evaluator knows everything
void mainYaoProtocol(ProtocolDesc* pd, bool point_and_permute,
                     protocol_run start, void* arg)
{
  YaoProtocolDesc* ypd = pd->extra;
  int me = pd->thisParty;
  bool ownOT=false;
  ypd->gcount = ypd->icount = ypd->ocount = 0;
  if(me==1)
  {
    gcry_randomize(ypd->R,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    gcry_randomize(ypd->I,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    if(point_and_permute) ypd->R[0] |= 1;   // flipper bit

    if(ypd->sender.sender==NULL)
    { ownOT=true;
      ypd->sender = honestOTExtSenderAbstract(honestOTExtSenderNew(pd,2));
    }
  }else 
    if(ypd->recver.recver==NULL)
    { ownOT=true;
      ypd->recver = honestOTExtRecverAbstract(honestOTExtRecverNew(pd,1));
    }

  currentProto = pd;
  start(arg);

  if(ownOT)
  { if(me==1) otSenderRelease(&ypd->sender);
    else otRecverRelease(&ypd->recver);
  }
}

void cleanupYaoProtocol(ProtocolDesc* pd)
{
  YaoProtocolDesc* ypd = pd->extra;
  gcry_cipher_close(ypd->fixedKeyCipher);
  free(ypd);
}

void execYaoProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
  setupYaoProtocol(pd,true);
  mainYaoProtocol(pd,true,start,arg);
  cleanupYaoProtocol(pd);
}

void execYaoProtocol_noHalf(ProtocolDesc* pd, protocol_run start, void* arg)
{
  setupYaoProtocol(pd,false);
  mainYaoProtocol(pd,true,start,arg);
  free(pd->extra);
}

// Special purpose gates, meant to be used if you like doing low-level
// optimizations. Note: this one assumes constant propagation has already
// been done, and 'a' is private to the generator.
// TODO I should really find a better way to factor this out with 
// YaoGenerateHalfGatePair
void yaoGenerateGenHalf(ProtocolDesc* pd,OblivBit* r,
    bool ac, bool bc, bool rc, const OblivBit* a, bool b)
{
  YaoProtocolDesc* ypd = pd->extra;
  if(a->yao.inverted) ac=!ac;

  bool pa = yaoKeyLsb(a->yao.w);
  yao_key_t row,t,wg,wa1;
  const char *wa0 = a->yao.w;

  yaoKeyCopy(wa1,wa0); yaoKeyXor(wa1,ypd->R);
  yaoSetHalfMask2(ypd,row,wa0,t,wa1,ypd->gcount);
  yaoKeyCopy(wg,(pa?t:row));
  yaoKeyXor (row,t);
  yaoKeyCondXor(row,(b!=bc),row,ypd->R);
  osend(pd,2,row,YAO_KEY_BYTES);
  yaoKeyCondXor(wg,((pa!=ac)&&(b!=bc))!=rc,wg,ypd->R);
  yaoKeyCopy(r->yao.w,wg);
  r->yao.inverted = false; r->unknown = true;
  ypd->gcount++;
}
void yaoEvaluateGenHalf(ProtocolDesc* pd,OblivBit* r,const OblivBit* a)
{
  YaoProtocolDesc* ypd = pd->extra;
  yao_key_t row,t;

  yaoSetHalfMask(ypd,t,a->yao.w,ypd->gcount++);
  orecv(pd,1,row,YAO_KEY_BYTES);
  yaoKeyCondXor(r->yao.w,yaoKeyLsb(a->yao.w),t,row);
  r->unknown = true;
}
typedef struct
{ yao_key_t *w,*R;
  bool *flip;
  int n;
} YaoSendEHalfAndAux;

void yaoGenerateEvalHalf_aux(char* opt1,const char* opt0,int c,void* vargs)
{
  YaoSendEHalfAndAux* args = vargs;
  int i;
  for(i=0;i<args->n;++i)
  { char *dest=opt1+i*YAO_KEY_BYTES;
    const char *src=opt0+i*YAO_KEY_BYTES;
    yaoKeyCopy(dest,src);
    yaoKeyXor (dest,args->w[i]);
    yaoKeyCondXor(dest,args->flip[i],dest,*args->R);
  }
}
void yaoGenerateEvalHalf(ProtocolDesc* pd,YaoEHalfSwapper* sw,
    OblivBit r[],const OblivBit a[],int n)
{
  YaoProtocolDesc* ypd = pd->extra;
  YaoSendEHalfAndAux args;
  // This seems like too many mallocs. Should profile/preallocate this
  // Or split up OT "chunks" into multiple pieces for a piece. TODO
  // Simplifies this code to make it look more like GenHalf
  yao_key_t *dummy = malloc(n*sizeof(yao_key_t));
  yao_key_t *results = malloc(n*sizeof(yao_key_t));
  args.w = malloc(n*sizeof(yao_key_t));
  args.flip = malloc(n*sizeof(bool));
  int i,j=0;
  for(i=0;i<n;++i) 
    if(a[i].unknown)
    { yaoKeyCopy(args.w[j],a[i].yao.w);
      args.flip[j]=a[i].yao.inverted;
      j++;
    }else if(known(a+i) && a[i].knownValue==true)
    { yaoKeyZero(args.w[j]);
      args.flip[j]=true; // Negation of zero, if receiver picked 1
      j++;
    }
  args.R=&ypd->R;
  args.n=j;
  honestOTExtSend1Of2Chunk(sw->args,(char*)results,(char*)dummy,1,
      j*YAO_KEY_BYTES,
      yaoGenerateEvalHalf_aux,&args);
  j=0;
  for(i=0;i<n;++i)
    if(a[i].unknown||a[i].knownValue==true)
    { yaoKeyCopy(r[i].yao.w,results[j]);
      r[i].yao.inverted=false;
      r[i].unknown=true;
      j++;
    }else __obliv_c__assignBitKnown(r+i,false);
  free(args.w);
  free(args.flip);
  free(results);
  free(dummy);
}
void yaoEvaluateEvalHalf(ProtocolDesc* pd,YaoEHalfSwapper* sw,
    OblivBit r[],const OblivBit a[],int n)
{
  YaoProtocolDesc *ypd = pd->extra;
  yao_key_t *results = malloc(n*sizeof(yao_key_t));
  int i,j=0;
  bool s = sw->sel[sw->ind];
  for(i=0;i<n;++i) if(a[i].unknown||a[i].knownValue==true) j++;
  honestOTExtRecv1Of2Chunk(sw->args,(char*)results,1,j*YAO_KEY_BYTES,true);
  j=0;
  for(i=0;i<n;++i)
    if(a[i].unknown||a[i].knownValue==true)
    { yaoKeyCondXor(r[i].yao.w,s,results[j],a[i].yao.w);
      r[i].yao.inverted=false;
      r[i].unknown=true;
      j++;
    }else __obliv_c__assignBitKnown(r+i,false);
  free(results);
  sw->ind++;
}
// b is ignored if I am not generator
void yaoGHalfAnd(ProtocolDesc* pd,OblivBit* r,const OblivBit* a,bool b)
{
  __obliv_c__bool feedOblivBool(bool b,int party);
  if(known(a)) 
  { if(a->knownValue) *r=feedOblivBool(b,1).bits[0];
    else __obliv_c__assignBitKnown(r,false);
  }
  else if(protoCurrentParty(pd)==1) yaoGenerateGenHalf(pd,r,0,0,0,a,b);
  else yaoEvaluateGenHalf(pd,r,a);
}
void yaoGHalfSwapGate(ProtocolDesc* pd,
    OblivBit a[],OblivBit b[],int n,bool c)
{
  OblivBit x,r;
  while(n-->0)
  { __obliv_c__setBitXor(&x,a+n,b+n);
    yaoGHalfAnd(pd,&r,&x,c);
    __obliv_c__setBitXor(a+n,a+n,&r);
    __obliv_c__setBitXor(b+n,b+n,&r);
  }
}
// b ignored if I am generator
YaoEHalfSwapper yaoEHalfSwapSetup(ProtocolDesc* pd,const bool b[],size_t n)
{
  YaoProtocolDesc* ypd = pd->extra;
  void* rv;
  if(protoCurrentParty(pd)==1)
    rv=honestOTExtSend1Of2Start(ypd->sender.sender,n);
  else rv=honestOTExtRecv1Of2Start(ypd->recver.recver,b,n);
  int i;
  return (YaoEHalfSwapper){.args=rv,.sel=b,.ind=0};
}
void yaoEHalfSwapGate(ProtocolDesc* pd,
   OblivBit a[], OblivBit b[],int n,YaoEHalfSwapper* sw)
{
  // Again, too many mallocs
  OblivBit *x = calloc(n,sizeof(OblivBit));
  int i;
  for(i=0;i<n;++i) __obliv_c__setBitXor(x+i,a+i,b+i);
  if(protoCurrentParty(pd)==1) yaoGenerateEvalHalf(pd,sw,x,x,n);
  else yaoEvaluateEvalHalf(pd,sw,x,x,n);
  for(i=0;i<n;++i)
  { __obliv_c__setBitXor(a+i,a+i,x+i);
    __obliv_c__setBitXor(b+i,b+i,x+i);
  }
  free(x);
}
