// TODO I need to fix some int sizes
#include <obliv_common.h>
#include <obliv_bits.h>
#include <inttypes.h>
#include <stdio.h>      // for protoUseStdio()
#include <string.h>
#include <gcrypt.h>

// Q: What's with all these casts to and from void* ?
// A: Code generation becomes easier without the need for extraneous casts.
//      Might fix it some day. But user code never sees these anyway.

// Right now, we do not support multiple protocols at the same time
static ProtocolDesc currentProto;

// --------------------------- Transports -----------------------------------
struct stdioTransport
{ ProtocolTransport cb;
  bool needFlush;
};

// Ignores 'dest' parameter. So you can't osend to yourself
static bool* stdioFlushFlag(ProtocolDesc* pd)
  { return &((struct stdioTransport*)pd->trans)->needFlush; }

static int stdioSend(ProtocolDesc* pd,int dest,const void* s,size_t n)
{ *stdioFlushFlag(pd)=true;
  return fwrite(s,1,n,stdout); 
}

static int stdioRecv(ProtocolDesc* pd,int src,void* s,size_t n)
{ 
  bool *p = stdioFlushFlag(pd);
  if(*p) { fflush(stdout); *p=false; }
  return fread(s,1,n,stdin); 
}

static void stdioCleanup(ProtocolDesc* pd) {}

static struct stdioTransport stdioTransport 
  = {{ stdioSend, stdioRecv, stdioCleanup},false};

void protocolUseStdio(ProtocolDesc* pd)
  { pd->trans = &stdioTransport.cb; }

void cleanupProtocol(ProtocolDesc* pd)
  { pd->trans->cleanup(pd); }

void setCurrentParty(ProtocolDesc* pd, int party)
  { pd->thisParty=party; }

void __obliv_c__assignBitKnown(OblivBit* dest, bool value)
  { dest->knownValue = value; dest->known=true; }

void __obliv_c__copyBit(OblivBit* dest,const OblivBit* src)
  { if(dest!=src) *dest=*src; }

bool __obliv_c__bitIsKnown(const OblivBit* bit,bool* v)
{ if(bit->known) *v=bit->knownValue;
  return bit->known;
}

static int tobool(int x) { return x?1:0; }

// TODO all sorts of identical parameter optimizations
// Implementation note: remember that all these pointers may alias each other
void dbgProtoSetBitAnd(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (a->knownValue&& b->knownValue);
  dest->known = false;
  currentProto.debug.mulCount++;
}

void dbgProtoSetBitOr(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (a->knownValue|| b->knownValue);
  dest->known = false;
  currentProto.debug.mulCount++;
}
void dbgProtoSetBitXor(ProtocolDesc* pd,
    OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  dest->knownValue= (tobool(a->knownValue) != tobool(b->knownValue));
  dest->known = false;
  currentProto.debug.xorCount++;
}
void dbgProtoSetBitNot(ProtocolDesc* pd,OblivBit* dest,const OblivBit* a)
{
  dest->knownValue= !a->knownValue;
  dest->known = a->known;
}
void dbgProtoFlipBit(ProtocolDesc* pd,OblivBit* dest) 
  { dest->knownValue = !dest->knownValue; }

//-------------------- Yao Protocol (honest but curious) -------------

#define OT_BATCH_SIZE 7

void gcryDefaultLibInit(void)
{
  if(!gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P))
  { if(!gcry_check_version(NULL))
    { fprintf(stderr,"libgcrypt init failed\n");
      exit(1);
    }
    gcry_control (GCRYCTL_DISABLE_SECMEM, 0);
    gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
  }
}

// Assume: generator is party 1, evaluator is party 2

static void yaoKeyDebug(const yao_key_t k)
{ int i;
  fprintf(stderr,"%d: ",currentProto.thisParty);
  for(i=YAO_KEY_BYTES-1;i>=0;--i) fprintf(stderr,"%02x ",0xff&k[i]);
  fprintf(stderr,"\n");
}
static void debugOblivBit(const OblivBit* o)
{
  if(o->known) fprintf(stderr,"Known value %d\n",(int)o->knownValue);
  else 
  { fprintf(stderr,"inv %d, ",(int)o->yao.inverted);
    yaoKeyDebug(o->yao.w);
  }
}

void yaoKeyCopy(yao_key_t d, const yao_key_t s) { memcpy(d,s,YAO_KEY_BYTES); }
void yaoKeyZero(yao_key_t d) { memset(d,0,YAO_KEY_BYTES); }
bool yaoKeyLsb(const yao_key_t k) { return k[0]&1; }
void yaoKeyXor(yao_key_t d, const yao_key_t s)
{ int i;
  for(i=0;i<YAO_KEY_BYTES;++i) d[i]^=s[i];
}
// XXX do I need i when it is already encoded in lsb(a)||lsb(b)
void yaoSetHashMask(yao_key_t d,const yao_key_t a,const yao_key_t b,
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
void yaoKeyNewPair(ProtocolDesc* pd,yao_key_t w0,yao_key_t w1)
{
  unsigned* ic = &pd->yao.icount;
  char buf[YAO_KEY_BYTES+sizeof(*ic)], dest[20];
  memcpy(buf,pd->yao.I,YAO_KEY_BYTES);
  memcpy(buf+YAO_KEY_BYTES,ic,sizeof(*ic));
  (*ic)++;
  gcry_md_hash_buffer(GCRY_MD_SHA1,dest,buf,sizeof(buf));
  memcpy(w0,dest,YAO_KEY_BYTES);
  yaoKeyCopy(w1,w0);
  yaoKeyXor(w1,pd->yao.R);
}

void yaoSetBitAnd(ProtocolDesc* pd,OblivBit* r,
                  const OblivBit* a,const OblivBit* b)
  { pd->yao.nonFreeGate(pd,r,0x8,a,b); }
void yaoSetBitOr(ProtocolDesc* pd,OblivBit* r,
                 const OblivBit* a,const OblivBit* b)
  { pd->yao.nonFreeGate(pd,r,0xE,a,b); }
void yaoSetBitXor(ProtocolDesc* pd,OblivBit* r,
                 const OblivBit* a,const OblivBit* b)
{
  OblivBit t;
  yaoKeyCopy(t.yao.w,a->yao.w);
  yaoKeyXor (t.yao.w,b->yao.w);
  t.yao.inverted = (a->yao.inverted!=b->yao.inverted); // no-op for evaluator
  t.known = false;
  *r = t;
}
void yaoFlipBit(ProtocolDesc* pd,OblivBit* r)
  { r->yao.inverted = !r->yao.inverted; }
void yaoSetBitNot(ProtocolDesc* pd,OblivBit* r,const OblivBit* a)
  { *r = *a; yaoFlipBit(pd,r); }

// Java-style iterator over bits in OblivInput array, assumes all sizes > 0
typedef struct { int i,j; OblivInputs* oi; size_t n; } OIBitSrc;
static bool hasBit (OIBitSrc* s) { return s->i<s->n; }
static bool curBit (OIBitSrc* s) { return s->oi[s->i].src & (1<<s->j); }
static OblivBit* curDestBit(OIBitSrc* s) { return s->oi[s->i].dest+s->j; }
static void nextBit(OIBitSrc* s) 
  { if(++(s->j)>=s->oi[s->i].size) { s->j=0; ++(s->i); } }
static int bitCount(OIBitSrc* s) 
{ int res=0,i;
  for(i=0;i<s->n;++i) res+=s->oi[i].size;
  return res;
}

void yaoGenrFeedOblivInputs(ProtocolDesc* pd,OblivInputs* oi,size_t n,int src)
{ 
  yao_key_t w0,w1;
  OIBitSrc it = {0,0,oi,n};
  if(src==1) for(;hasBit(&it);nextBit(&it))
  { OblivBit* o = curDestBit(&it);
    yaoKeyNewPair(pd,w0,w1);
    if(curBit(&it)) osend(pd,2,w1,YAO_KEY_BYTES);
    else osend(pd,2,w0,YAO_KEY_BYTES);
    o->yao.inverted = o->known = false;
    yaoKeyCopy(o->yao.w,w0);
    pd->yao.icount++;
  }else 
  {
    char buf0[OT_BATCH_SIZE*YAO_KEY_BYTES], buf1[OT_BATCH_SIZE*YAO_KEY_BYTES];
    int bp=0;
    for(;hasBit(&it);nextBit(&it))
    { 
      OblivBit* o = curDestBit(&it);
      yaoKeyNewPair(pd,w0,w1);
      yaoKeyCopy(buf0+bp*YAO_KEY_BYTES,w0);
      yaoKeyCopy(buf1+bp*YAO_KEY_BYTES,w1);
      o->yao.inverted = o->known = false;
      yaoKeyCopy(o->yao.w,w0);
      pd->yao.icount++;
      if(++bp>=OT_BATCH_SIZE)
      { npotSend1Of2Once(pd->yao.sender,buf0,buf1,OT_BATCH_SIZE,YAO_KEY_BYTES);
        bp=0;
      }
    }
    if(bp>0) npotSend1Of2Once(pd->yao.sender,buf0,buf1,bp,YAO_KEY_BYTES);
  }
}
void yaoEvalFeedOblivInputs(ProtocolDesc* pd,OblivInputs* oi,size_t n,int src)
{ OIBitSrc it = {0,0,oi,n};
  if(src==1) for(;hasBit(&it);nextBit(&it))
  { OblivBit* o = curDestBit(&it);
    orecv(pd,1,o->yao.w,YAO_KEY_BYTES);
    o->known = false;
    pd->yao.icount++;
  }else 
  { char buf[OT_BATCH_SIZE*YAO_KEY_BYTES], *dest[OT_BATCH_SIZE];
    int mask=0;
    int bp=0,i;
    for(;hasBit(&it);nextBit(&it))
    { OblivBit* o = curDestBit(&it);
      dest[bp]=o->yao.w;
      mask|=(curBit(&it)?1<<bp:0);
      o->known = false; // Known to me, but not to both parties
      pd->yao.icount++;
      if(++bp>=OT_BATCH_SIZE)
      { npotRecv1Of2Once(pd->yao.recver,buf,mask,OT_BATCH_SIZE,YAO_KEY_BYTES);
        for(i=0;i<bp;++i) yaoKeyCopy(dest[i],buf+i*YAO_KEY_BYTES);
        bp=0;
        mask=0;
      }
    }
    if(bp>0)
    { npotRecv1Of2Once(pd->yao.recver,buf,mask,bp,YAO_KEY_BYTES);
      for(i=0;i<bp;++i) yaoKeyCopy(dest[i],buf+i*YAO_KEY_BYTES);
    }
  }
}

widest_t yaoGenrRevealOblivBits(ProtocolDesc* pd,
    const OblivBit* o,size_t n,int party)
{
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  for(i=0;i<n;++i) if(!o[i].known)
    flipflags |= ((yaoKeyLsb(o[i].yao.w) != o[i].yao.inverted)?1LL<<i:0);
  // Assuming little endian
  if(party != 1) osend(pd,2,&flipflags,bc);
  if(party != 2) { orecv(pd,2,&rv,bc); rv^=flipflags; }
  for(i=0;i<n;++i) if(o[i].known && o[i].knownValue)
    rv |= (1LL<<i);
  pd->yao.ocount+=n;
  return rv;
}
widest_t yaoEvalRevealOblivBits(ProtocolDesc* pd,
    const OblivBit* o,size_t n,int party)
{
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  for(i=0;i<n;++i) if(!o[i].known)
    flipflags |= (yaoKeyLsb(o[i].yao.w)?1LL<<i:0);
  // Assuming little endian
  if(party != 1) { orecv(pd,1,&rv,bc); rv^=flipflags; }
  if(party != 2) osend(pd,1,&flipflags,bc);
  for(i=0;i<n;++i) if(o[i].known && o[i].knownValue)
    rv |= (1LL<<i);
  pd->yao.ocount+=n;
  return rv;
}

// Encodes a 2-input truth table for f(a,b) = ((ttable&(1<<(2*a+b)))!=0)
void yaoGenerateGate(ProtocolDesc* pd, OblivBit* r, char ttable, 
    const OblivBit* a, const OblivBit* b)
{ uint64_t k = pd->yao.gcount;
  int im=0,i;
  yao_key_t wa,wb,wc,wt;
  yao_key_t wdebug;
  const char* R = pd->yao.R;

  // adjust truth table according to invert fields (faster with im^=...) TODO
  if(a->yao.inverted) ttable = (((ttable&3)<<2)|((ttable>>2)&3));
  if(b->yao.inverted) ttable = (((ttable&5)<<1)|((ttable>>1)&5));

  // Doing garbled-row-reduction: wc is set to first mask
  yaoKeyCopy(wa,a->yao.w);
  yaoKeyCopy(wb,b->yao.w);
  if(yaoKeyLsb(wa)) { im^=2; yaoKeyXor(wa,R); }
  if(yaoKeyLsb(wb)) { im^=1; yaoKeyXor(wb,R); }

  // Create labels for new wire in wc
  yaoSetHashMask(wc,wa,wb,k,0);
  if(ttable&(1<<im)) yaoKeyXor(wc,R);

  for(i=1;i<4;++i) // skip 0, because GRR
  { yaoKeyXor(wb,R);
    if(i==2) yaoKeyXor(wa,R);
    yaoSetHashMask(wt,wa,wb,k,i);
    yaoKeyXor(wt,wc);
    if(ttable&(1<<(i^im))) yaoKeyXor(wt,R);
    osend(pd,2,wt,YAO_KEY_BYTES);
  }

  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,wc);
  r->known = r->yao.inverted = false;
  pd->yao.gcount++;
}

void yaoEvaluateGate(ProtocolDesc* pd, OblivBit* r, char ttable, 
  const OblivBit* a, const OblivBit* b)
{
  int i=0,j;
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
  yaoSetHashMask(t,a->yao.w,b->yao.w,pd->yao.gcount++,i);
  yaoKeyXor(w,t);
  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,w);
  r->known = false;
}

unsigned yaoGateCount() { return currentProto.yao.gcount; }

void execYaoProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
  int me = pd->thisParty;
  int tailind,tailpos;
  pd->partyCount = 2;
  pd->yao.nonFreeGate = (me==1?yaoGenerateGate:yaoEvaluateGate);
  pd->feedOblivInputs = (me==1?yaoGenrFeedOblivInputs:yaoEvalFeedOblivInputs);
  pd->revealOblivBits = (me==1?yaoGenrRevealOblivBits:yaoEvalRevealOblivBits);
  pd->setBitAnd = yaoSetBitAnd;
  pd->setBitOr  = yaoSetBitOr;
  pd->setBitXor = yaoSetBitXor;
  pd->setBitNot = yaoSetBitNot;
  pd->flipBit   = yaoFlipBit;
  pd->yao.gcount = pd->yao.icount = pd->yao.ocount = 0;

  dhRandomInit();

  if(me==1)
  {
    gcry_randomize(pd->yao.R,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    gcry_randomize(pd->yao.I,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    pd->yao.R[0] |= 1;   // flipper bit

    tailind=YAO_KEY_BYTES-1;
    tailpos=8-(8*YAO_KEY_BYTES-YAO_KEY_BITS);
    pd->yao.R[tailind] &= (1<<tailpos)-1;
    pd->yao.I[tailind] &= (1<<tailpos)-1;
    pd->yao.sender = npotSenderNew(1<<OT_BATCH_SIZE,pd,me);
  }else pd->yao.recver = npotRecverNew(1<<OT_BATCH_SIZE,pd,me);

  currentProto = *pd;
  start(arg);

  if(me==1) npotSenderRelease(pd->yao.sender);
  else npotRecverRelease(pd->yao.recver);
}

void __obliv_c__setBitAnd(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(a->known || b->known)
  { if(!a->known) { const OblivBit* t=a; a=b; b=t; }
    if(a->knownValue) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,false);
  }else currentProto.setBitAnd(&currentProto,dest,a,b);
}
void __obliv_c__setBitOr(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(a->known || b->known)
  { if(!a->known) { const OblivBit* t=a; a=b; b=t; }
    if(!a->knownValue) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,true);
  }else currentProto.setBitOr(&currentProto,dest,a,b);
}
void __obliv_c__setBitXor(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  bool v;
  if(a->known || b->known)
  { if(!a->known) { const OblivBit* t=a; a=b; b=t; }
    v = a->knownValue;
    __obliv_c__copyBit(dest,b);
    if(v) __obliv_c__flipBit(dest);
  }else currentProto.setBitXor(&currentProto,dest,a,b); 
}
void __obliv_c__setBitNot(OblivBit* dest,const OblivBit* a)
{ if(dest->known) { *dest=*a; dest->knownValue=!dest->knownValue; }
  else currentProto.setBitNot(&currentProto,dest,a); 
}
void __obliv_c__flipBit(OblivBit* dest) 
{ if(dest->known) dest->knownValue = !dest->knownValue;
  else currentProto.flipBit(&currentProto,dest); 
}

static void dbgFeedOblivBool(OblivBit* dest,int party,bool a)
{ 
  int curparty = __obliv_c__currentParty();
  
  dest->known=false;
  if(party==1) { if(curparty==1) dest->knownValue=a; }
  else if(party==2 && curparty == 1) 
    orecv(&currentProto,1,&dest->knownValue,sizeof(bool));
  else if(party==2 && curparty == 2) osend(&currentProto,1,&a,sizeof(bool));
  else fprintf(stderr,"Error: This is a 2 party protocol\n");
}
  /*
void __obliv_c__feedOblivBits(OblivBit* dest, int party
                             ,const bool* src,size_t size)
  { while(size--) __obliv_c__feedOblivBool(dest++,party,*(src++)); }
*/

void __obliv_c__setupOblivBits(OblivInputs* spec,OblivBit*  dest
                                     ,widest_t v,size_t size)
{ spec->dest=dest;
  spec->src=v;
  spec->size=size;
}
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

/*
bool __obliv_c__revealOblivBool(const OblivBit* dest,int party)
{ if(party!=0 && party!=currentProto.thisParty) return false;
  else return dest->knownValue;
}
*/
widest_t dbgProtoRevealOblivBits
  (ProtocolDesc* pd,const OblivBit* dest,size_t size,int party)
{ widest_t rv=0;
  if(currentProto.thisParty==1)
  { dest+=size;
    while(size-->0) rv = (rv<<1)+tobool((--dest)->knownValue);
    if(party==0 || party==2) osend(pd,2,&rv,sizeof(rv));
    if(party==2) return 0;
    else return rv;
  }else // assuming thisParty==2
  { if(party==0 || party==2) { orecv(pd,1,&rv,sizeof(rv)); return rv; }
    else return 0;
  }
}

static void broadcastBits(int source,void* p,size_t n)
{
  int i;
  if(currentProto.thisParty!=source) orecv(&currentProto,source,p,n);
  else for(i=1;i<=currentProto.partyCount;++i) if(i!=source)
      osend(&currentProto,i,p,n);
}

void execDebugProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
  pd->feedOblivInputs = dbgProtoFeedOblivInputs;
  pd->revealOblivBits = dbgProtoRevealOblivBits;
  pd->setBitAnd = dbgProtoSetBitAnd;
  pd->setBitOr  = dbgProtoSetBitOr;
  pd->setBitXor = dbgProtoSetBitXor;
  pd->setBitNot = dbgProtoSetBitNot;
  pd->flipBit   = dbgProtoFlipBit;
  pd->partyCount= 2;
  currentProto = *pd;
  currentProto.debug.mulCount = currentProto.debug.xorCount = 0;
  start(arg);
}

widest_t __obliv_c__revealOblivBits(const OblivBit* dest, size_t size,
    int party)
  { return currentProto.revealOblivBits(&currentProto,dest,size,party); }

int __obliv_c__currentParty() { return currentProto.thisParty; }

void __obliv_c__setSignedKnown
  (void* vdest, size_t size, long long signed value)
{
  OblivBit* dest=vdest;
  while(size-->0)
  { __obliv_c__assignBitKnown(dest,value&1);
    value>>=1; dest++;
  }
}
void __obliv_c__setUnsignedKnown
  (void* vdest, size_t size, long long unsigned value)
{
  OblivBit* dest=vdest;
  while(size-->0)
  { __obliv_c__assignBitKnown(dest,value&1);
    value>>=1; dest++;
  }
}
void __obliv_c__setBitsKnown(OblivBit* dest, const bool* value, size_t size)
  { while(size-->0) __obliv_c__assignBitKnown(dest++,value++); }
void __obliv_c__copyBits(OblivBit* dest, const OblivBit* src, size_t size)
  { if(dest!=src) while(size-->0) __obliv_c__copyBit(dest++,src++); }
bool __obliv_c__allBitsKnown(const OblivBit* bits, bool* dest, size_t size)
{ while(size-->0) if(!__obliv_c__bitIsKnown(bits++,dest++)) return false;
  return true;
}

void bitwiseOp(OblivBit* dest,const OblivBit* a,const OblivBit* b,size_t size
              ,void (*f)(OblivBit*,const OblivBit*,const OblivBit*))
  { while(size-->0) f(dest++,a++,b++); }


void __obliv_c__setBitwiseAnd (void* vdest
                              ,const void* vop1,const void* vop2
                              ,size_t size)
  { bitwiseOp(vdest,vop1,vop2,size,__obliv_c__setBitAnd); }

void __obliv_c__setBitwiseOr  (void* vdest
                              ,const void* vop1,const void* vop2
                              ,size_t size)
  { bitwiseOp(vdest,vop1,vop2,size,__obliv_c__setBitOr); }

void __obliv_c__setBitwiseXor (void* vdest
                              ,const void* vop1,const void* vop2
                              ,size_t size)
  { bitwiseOp(vdest,vop1,vop2,size,__obliv_c__setBitXor); }

void __obliv_c__setBitwiseNot (void* vdest,const void* vop,size_t size)
{ OblivBit *dest = vdest;
  const OblivBit *op = vop;
  while(size-->0) __obliv_c__setBitNot(dest++,op++); 
}

void __obliv_c__setBitwiseNotInPlace (void* vdest,size_t size)
{ OblivBit *dest=vdest; 
  while(size-->0) __obliv_c__flipBit(dest++); 
}

// carryIn and/or carryOut can be NULL, in which case they are ignored
void __obliv_c__setBitsAdd (void* vdest,void* carryOut
                           ,const void* vop1,const void* vop2
                           ,const void* carryIn
                           ,size_t size)
{
  OblivBit carry,bxc,axc,t;
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  size_t skipLast;
  if(size==0)
  { if(carryIn && carryOut) __obliv_c__copyBit(carryOut,carryIn);
    return;
  }
  if(carryIn) __obliv_c__copyBit(&carry,carryIn);
  else __obliv_c__assignBitKnown(&carry,0);
  // skip AND on last bit if carryOut==NULL
  skipLast = (carryOut==NULL);
  while(size-->skipLast)
  { __obliv_c__setBitXor(&axc,op1,&carry);
    __obliv_c__setBitXor(&bxc,op2,&carry);
    __obliv_c__setBitXor(dest,op1,&bxc);
    __obliv_c__setBitAnd(&t,&axc,&bxc);
    __obliv_c__setBitXor(&carry,&carry,&t);
    ++dest; ++op1; ++op2;
  }
  if(carryOut) __obliv_c__copyBit(carryOut,&carry);
  else
  { __obliv_c__setBitXor(&axc,op1,&carry);
    __obliv_c__setBitXor(dest,&axc,op2);
  }
}

void __obliv_c__setPlainAdd (void* vdest
                            ,const void* vop1 ,const void* vop2
                            ,size_t size)
  { __obliv_c__setBitsAdd (vdest,NULL,vop1,vop2,NULL,size); }

void __obliv_c__setBitsSub (void* vdest, void* borrowOut
                           ,const void* vop1,const void* vop2
                           ,const void* borrowIn,size_t size)
{
  OblivBit borrow,bxc,bxa,t;
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  size_t skipLast;
  if(size==0)
  { if(borrowIn && borrowOut) __obliv_c__copyBit(borrowOut,borrowIn);
    return;
  }
  if(borrowIn) __obliv_c__copyBit(&borrow,borrowIn);
  else __obliv_c__assignBitKnown(&borrow,0);
  // skip AND on last bit if borrowOut==NULL
  skipLast = (borrowOut==NULL);
  while(size-->skipLast)
  { // c = borrow; a = op1; b=op2; borrow = (b+c)(b+a)+c
    __obliv_c__setBitXor(&bxa,op1,op2);
    __obliv_c__setBitXor(&bxc,&borrow,op2);
    __obliv_c__setBitXor(dest,&bxa,&borrow);
    __obliv_c__setBitAnd(&t,&bxa,&bxc);
    __obliv_c__setBitXor(&borrow,&borrow,&t);
    ++dest; ++op1; ++op2;
  }
  if(borrowOut) __obliv_c__copyBit(borrowOut,&borrow);
  else
  { __obliv_c__setBitXor(&bxa,op1,op2);
    __obliv_c__setBitXor(dest,&bxa,&borrow);
  }
}

void __obliv_c__setPlainSub (void* vdest
                            ,const void* vop1 ,const void* vop2
                            ,size_t size)
  { __obliv_c__setBitsSub (vdest,NULL,vop1,vop2,NULL,size); }

void __obliv_c__setSignExtend (void* vdest, size_t dsize
                              ,const void* vsrc, size_t ssize)
{
  if(ssize>dsize) ssize=dsize;
  OblivBit *dest = vdest;
  __obliv_c__copyBits(vdest,vsrc,ssize);
  const OblivBit* s = ((const OblivBit*)vsrc)+ssize-1;
  dsize-=ssize;
  dest+=ssize;
  while(dsize-->0) __obliv_c__copyBit(dest++,s);
}
void __obliv_c__setZeroExtend (void* vdest, size_t dsize
                              ,const void* vsrc, size_t ssize)
{
  if(ssize>dsize) ssize=dsize;
  OblivBit *dest = vdest;
  __obliv_c__copyBits(vdest,vsrc,ssize);
  dsize-=ssize;
  dest+=ssize;
  while(dsize-->0) __obliv_c__assignBitKnown(dest++,0);
}

void __obliv_c__ifThenElse (void* vdest, const void* vtsrc
                           ,const void* vfsrc, size_t size
                           ,const void* vcond)
{
  // copying out vcond because it could be aliased by vdest
  OblivBit x,a,c=*(const OblivBit*)vcond;
  OblivBit *dest=vdest;
  const OblivBit *tsrc=vtsrc, *fsrc=vfsrc;
  while(size-->0)
  { __obliv_c__setBitXor(&x,tsrc,fsrc);
    __obliv_c__setBitAnd(&a,&c,&x);
    __obliv_c__setBitXor(dest,&a,fsrc);
    ++dest; ++fsrc; ++tsrc;
  }
}

// ltOut and ltIn may alias here
void __obliv_c__setLessThanUnit (OblivBit* ltOut
                                ,const OblivBit* op1, const OblivBit* op2
                                ,size_t size, const OblivBit* ltIn)
{
  // (a+b)(a+1)b + (a+b+1)c = b(a+1)+(a+b+1)c = ab+b+c+ac+bc = (a+b)(b+c)+c
  OblivBit t,x;
  __obliv_c__copyBit(ltOut,ltIn);
  while(size-->0)
  { __obliv_c__setBitXor(&x,op1,op2);
    __obliv_c__setBitXor(&t,op2,ltOut);
    __obliv_c__setBitAnd(&t,&t,&x);
    __obliv_c__setBitXor(ltOut,&t,ltOut);
    op1++; op2++;
  }
}

// assumes size >= 1
void __obliv_c__setLessThanSigned (void* vdest
                                  ,const void* vop1,const void* vop2
                                  ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1 = vop1, *op2 = vop2;
  __obliv_c__assignBitKnown(dest,0);
  __obliv_c__setLessThanUnit(dest,op1,op2,size-1,dest);
  __obliv_c__setLessThanUnit(dest,op2+size-1,op1+size-1,1,dest);
}

void __obliv_c__setLessThanOrEqualSigned (void* vdest
                                         ,const void* vop1, const void* vop2
                                         ,size_t size)
{
  __obliv_c__setLessThanSigned(vdest,vop2,vop1,size);
  __obliv_c__flipBit(vdest);
}

void __obliv_c__setLessThanUnsigned (void* vdest
                                    ,const void* vop1,const void* vop2
                                    ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1 = vop1, *op2 = vop2;
  __obliv_c__assignBitKnown(dest,0);
  __obliv_c__setLessThanUnit(dest,op1,op2,size,dest);
}

void __obliv_c__setLessOrEqualUnsigned (void* vdest
                                       ,const void* vop1, const void* vop2
                                       ,size_t size)
{
  __obliv_c__setLessThanUnsigned(vdest,vop2,vop1,size);
  __obliv_c__flipBit(vdest);
}

void __obliv_c__setEqualTo (void* vdest
                           ,const void* vop1,const void* vop2
                           ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1 = vop1, *op2 =  vop2;
  __obliv_c__setNotEqual(dest,op1,op2,size);
  __obliv_c__flipBit(dest);
}

void __obliv_c__setNotEqual (void* vdest
                            ,const void* vop1,const void* vop2
                            ,size_t size)
{
  OblivBit t;
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  __obliv_c__assignBitKnown(dest,0);
  while(size-->0)
  { __obliv_c__setBitXor(&t,op1++,op2++);
    __obliv_c__setBitOr(dest,dest,&t);
  }
}

void __obliv_c__condAdd(const void* vc,void* vdest
                       ,const void* vx,size_t size)
{ OblivBit t[size];
  int i;
  for(i=0;i<size;++i) __obliv_c__setBitAnd(t+i,vc,((OblivBit*)vx)+i);
  __obliv_c__setBitsAdd(vdest,NULL,vdest,t,NULL,size);
}
void __obliv_c__condSub(const void* vc,void* vdest
                       ,const void* vx,size_t size)
{ OblivBit t[size];
  int i;
  for(i=0;i<size;++i) __obliv_c__setBitAnd(t+i,vc,((OblivBit*)vx)+i);
  __obliv_c__setBitsSub(vdest,NULL,vdest,t,NULL,size);
}

// ---- Translated versions of obliv.oh functions ----------------------

// TODO remove __obliv_c prefix and make these functions static/internal
void setupOblivBool(OblivInputs* spec, __obliv_c__bool* dest, bool v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,1); }
void setupOblivChar(OblivInputs* spec, __obliv_c__char* dest, char v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,bitsize(v)); }
void setupOblivInt(OblivInputs* spec, __obliv_c__int* dest, int v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,bitsize(v)); }
void setupOblivShort(OblivInputs* spec, __obliv_c__short* dest, short v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,bitsize(v)); }
void setupOblivLong(OblivInputs* spec, __obliv_c__long* dest, long v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,bitsize(v)); }
void setupOblivLLong(OblivInputs* spec, __obliv_c__lLong* dest, long long v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,bitsize(v)); }

void feedOblivInputs(OblivInputs* spec, size_t count, int party)
  { currentProto.feedOblivInputs(&currentProto,spec,count,party); }

// TODO pass const values by ref later
bool revealOblivBool(__obliv_c__bool src,int party)
  { return (bool)__obliv_c__revealOblivBits(src.bits,1,party); }
char revealOblivChar(__obliv_c__char src,int party)
  { return (char)__obliv_c__revealOblivBits(src.bits,bitsize(char),party); }
int revealOblivInt(__obliv_c__int src,int party)
  { return (int)__obliv_c__revealOblivBits(src.bits,bitsize(int),party); }
short revealOblivShort(__obliv_c__short src,int party)
  { return (short)__obliv_c__revealOblivBits(src.bits,bitsize(short),party); }
long revealOblivLong(__obliv_c__long src,int party)
  { return (long)__obliv_c__revealOblivBits(src.bits,bitsize(long),party); }
long long revealOblivLLong(__obliv_c__lLong src,int party)
  { return (long long)__obliv_c__revealOblivBits(src.bits,bitsize(long long)
                                                 ,party); }

// TODO fix data width
bool ocBroadcastBool(int source,bool v)
{
  char t = v;
  broadcastBits(source,&t,1);
  return t;
}
#define broadcastFun(t,tname)           \
  t ocBroadcast##tname(int source, t v)   \
  { broadcastBits(source,&v,sizeof(v)); \
    return v;                           \
  }
broadcastFun(char,Char)
broadcastFun(int,Int)
broadcastFun(short,Short)
broadcastFun(long,Long)
broadcastFun(long long,LLong)
#undef broadcastFun
