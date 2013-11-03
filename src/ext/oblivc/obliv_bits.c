// TODO I need to fix some int sizes
#include <obliv_common.h>
#include <obliv_bits.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>      // for protoUseStdio()
#include <string.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>
#include <gcrypt.h>

// Q: What's with all these casts to and from void* ?
// A: Code generation becomes easier without the need for extraneous casts.
//      Might fix it some day. But user code never sees these anyway.

// Right now, we do not support multiple protocols at the same time
static ProtocolDesc *currentProto;

inline bool known(const OblivBit* o) { return !o->unknown; }

// Because I am evil and I do not like 
// Java-style redundant "say the type twice" practice
#define CAST(p) ((void*)p)

// --------------------------- Transports -----------------------------------
struct stdioTransport
{ ProtocolTransport cb;
  bool needFlush;
};

// Ignores 'dest' parameter. So you can't osend to yourself
static bool* stdioFlushFlag(ProtocolTransport* pt) 
  { return &((struct stdioTransport*)pt)->needFlush; }

static int stdioSend(ProtocolTransport* pt,int dest,const void* s,size_t n)
{ *stdioFlushFlag(pt)=true;
  return fwrite(s,1,n,stdout); 
}

static int stdioRecv(ProtocolTransport* pt,int src,void* s,size_t n)
{ 
  bool *p = stdioFlushFlag(pt);
  if(*p) { fflush(stdout); *p=false; }
  return fread(s,1,n,stdin); 
}

static void stdioSetChannel(ProtocolTransport* pt,int x) {}

static void stdioCleanup(ProtocolTransport* pt) {}

// Extremely simple, no multiplexing: two parties, one connection
static struct stdioTransport stdioTransport 
  = {{2, 1, 0, stdioSetChannel, stdioSend, stdioRecv, stdioCleanup},false};

void protocolUseStdio(ProtocolDesc* pd)
  { pd->trans = &stdioTransport.cb; }


// TCP connections for 2-Party protocols. Ignorse src/dest parameters
//   since there is only one remote
struct tcp2PTransport
{ ProtocolTransport cb;
  int* socks; // size = cb.maxChannels
  int cursock;
};

static int tcp2PSend(ProtocolTransport* pt,int dest,const void* s,size_t n)
  { return write(((struct tcp2PTransport*)pt)->cursock,s,n); }

static int tcp2PRecv(ProtocolTransport* pt,int src,void* s,size_t n)
  { return read(((struct tcp2PTransport*)pt)->cursock,s,n); }

static void tcp2PCleanup(ProtocolTransport* pt)
{ struct tcp2PTransport* t = CAST(pt);
  int i;
  for(i=0;i<t->cb.maxChannels;++i) close(t->socks[i]);
  free(t->socks);
  free(t);
}

static void tcp2PSetChannel(ProtocolTransport* pt,int c)
{ struct tcp2PTransport* t = CAST(pt);
  t->cb.curChannel=c;
  t->cursock = t->socks[c];
}

static struct tcp2PTransport tcp2PTransport
  = {{2, 0, 0, tcp2PSetChannel, tcp2PSend, tcp2PRecv, tcp2PCleanup}, NULL, 0};

void protocolUseTcp2P(ProtocolDesc* pd,int* socks,int sockCount)
{
  struct tcp2PTransport* trans;
  trans = malloc(sizeof(*trans));
  *trans = tcp2PTransport;
  trans->socks = malloc(sizeof(int)*sockCount);
  memcpy(trans->socks,socks,sizeof(int)*sockCount);
  trans->cb.maxChannels = sockCount;
  trans->cursock = socks[0];
  pd->trans = (ProtocolTransport*)trans;
}

static int getsockaddr(const char* name,const char* port, struct sockaddr* res)
{
  struct addrinfo* list;
  if(getaddrinfo(name,port,NULL,&list) < 0) return -1;
  memcpy(res,list->ai_addr,list->ai_addrlen);
  freeaddrinfo(list);
  return 0;
}
// used as sock=tcpConnect(...); ...; close(sock);
static int tcpConnect(const char* name, const char* port)
{
  int outsock;
  struct sockaddr_in sa;

  if((outsock=socket(AF_INET,SOCK_STREAM,0))<0) return -1;
  getsockaddr(name,port,(struct sockaddr*)&sa);
  if(connect(outsock,(struct sockaddr*)&sa,sizeof(sa))<0) return -1;
  return outsock;
}

int protocolConnectTcp2P(ProtocolDesc* pd,const char* server,const char* port,
                          int sockCount)
{
  int i, *socks = malloc(sizeof(int)*sockCount);
  for(i=0;i<sockCount;++i) if((socks[i]=tcpConnect(server,port))<0) return -1;
  protocolUseTcp2P(pd,socks,sockCount);
  free(socks);
  return 0;
}

// used as sock=tcpListenAny(...); sock2=accept(sock); ...; close(both);
static int tcpListenAny(const char* portn)
{
  in_port_t port;
  int outsock;
  if(sscanf(portn,"%hu",&port)<1) return -1;
  if((outsock=socket(AF_INET,SOCK_STREAM,0))<0) return -1;
  struct sockaddr_in sa = { .sin_family=AF_INET, .sin_port=htons(port)
                          , .sin_addr={INADDR_ANY} };
  if(bind(outsock,(struct sockaddr*)&sa,sizeof(sa))<0) return -1;
  if(listen(outsock,SOMAXCONN)<0) return -1;
  return outsock;
}
int protocolAcceptTcp2P(ProtocolDesc* pd,const char* port,int sockCount)
{
  int i, listenSock, *socks = malloc(sizeof(int)*sockCount);
  listenSock = tcpListenAny(port);
  for(i=0;i<sockCount;++i) if((socks[i]=accept(listenSock,0,0))<0) return -1;
  protocolUseTcp2P(pd,socks,sockCount);
  close(listenSock);
  free(socks);
  return 0;
}
// ---------------------------------------------------------------------------

void cleanupProtocol(ProtocolDesc* pd)
  { pd->trans->cleanup(pd->trans); }

void setCurrentParty(ProtocolDesc* pd, int party)
  { pd->thisParty=party; }

void __obliv_c__assignBitKnown(OblivBit* dest, bool value)
  { dest->knownValue = value; dest->unknown=false; }

void __obliv_c__copyBit(OblivBit* dest,const OblivBit* src)
  { if(dest!=src) *dest=*src; }

bool __obliv_c__bitIsKnown(bool* v,const OblivBit* bit)
{ if(known(bit)) *v=bit->knownValue;
  return known(bit);
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

void yaoSetBitAnd(ProtocolDesc* pdsuper,OblivBit* r,
                  const OblivBit* a,const OblivBit* b)
{ YaoProtocolDesc* pd = CAST(pdsuper);
  pd->nonFreeGate(pd,r,0x8,a,b); 
}
void yaoSetBitOr(ProtocolDesc* pdsuper,OblivBit* r,
                 const OblivBit* a,const OblivBit* b)
{ YaoProtocolDesc* pd = CAST(pdsuper);
  pd->nonFreeGate(pd,r,0xE,a,b); 
}
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

// Java-style iterator over bits in OblivInput array, assumes all sizes > 0
typedef struct { int i,j; OblivInputs* oi; size_t n; } OIBitSrc;
static bool hasBit (OIBitSrc* s) { return s->i<s->n; }
static bool curBit (OIBitSrc* s) { return s->oi[s->i].src & (1<<s->j); }
static OblivBit* curDestBit(OIBitSrc* s) { return s->oi[s->i].dest+s->j; }
static void nextBit(OIBitSrc* s) 
  { if(++(s->j)>=s->oi[s->i].size) { s->j=0; ++(s->i); } }
/*
static int bitCount(OIBitSrc* s) 
{ int res=0,i;
  for(i=0;i<s->n;++i) res+=s->oi[i].size;
  return res;
}
*/
void yaoGenrFeedOblivInputs(ProtocolDesc* pdsuper
                           ,OblivInputs* oi,size_t n,int src)
{ 
  YaoProtocolDesc* pd = CAST(pdsuper);
  yao_key_t w0,w1;
  OIBitSrc it = {0,0,oi,n};
  if(src==1) for(;hasBit(&it);nextBit(&it))
  { OblivBit* o = curDestBit(&it);
    yaoKeyNewPair(pd,w0,w1);
    if(curBit(&it)) osend(pdsuper,2,w1,YAO_KEY_BYTES);
    else osend(pdsuper,2,w0,YAO_KEY_BYTES);
    o->yao.inverted = false; o->unknown = true;
    yaoKeyCopy(o->yao.w,w0);
    pd->icount++;
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
      o->yao.inverted = false; o->unknown = true;
      yaoKeyCopy(o->yao.w,w0);
      pd->icount++;
      if(++bp>=OT_BATCH_SIZE)
      { npotSend1Of2Once(pd->sender,buf0,buf1,OT_BATCH_SIZE,YAO_KEY_BYTES);
        bp=0;
      }
    }
    if(bp>0) npotSend1Of2Once(pd->sender,buf0,buf1,bp,YAO_KEY_BYTES);
  }
}
void yaoEvalFeedOblivInputs(ProtocolDesc* pdsuper
                           ,OblivInputs* oi,size_t n,int src)
{ OIBitSrc it = {0,0,oi,n};
  YaoProtocolDesc* pd = CAST(pdsuper);
  if(src==1) for(;hasBit(&it);nextBit(&it))
  { OblivBit* o = curDestBit(&it);
    orecv(pdsuper,1,o->yao.w,YAO_KEY_BYTES);
    o->unknown = true;
    pd->icount++;
  }else 
  { char buf[OT_BATCH_SIZE*YAO_KEY_BYTES], *dest[OT_BATCH_SIZE];
    int mask=0;
    int bp=0,i;
    // XXX we are currently using NPOT, so it can be used with any protocol
    //   later on if we change it (to use e.g. passive-secure OT-extension)
    //   we might have to use different functions for each protocol
    for(;hasBit(&it);nextBit(&it))
    { OblivBit* o = curDestBit(&it);
      dest[bp]=o->yao.w;
      mask|=(curBit(&it)?1<<bp:0);
      o->unknown = true; // Known to me, but not to both parties
      pd->icount++;
      if(++bp>=OT_BATCH_SIZE)
      { npotRecv1Of2Once(pd->recver,buf,mask,OT_BATCH_SIZE,YAO_KEY_BYTES);
        for(i=0;i<bp;++i) yaoKeyCopy(dest[i],buf+i*YAO_KEY_BYTES);
        bp=0;
        mask=0;
      }
    }
    if(bp>0)
    { npotRecv1Of2Once(pd->recver,buf,mask,bp,YAO_KEY_BYTES);
      for(i=0;i<bp;++i) yaoKeyCopy(dest[i],buf+i*YAO_KEY_BYTES);
    }
  }
}

bool yaoGenrRevealOblivBits(ProtocolDesc* pdsuper,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  YaoProtocolDesc *pd = CAST(pdsuper);
  for(i=0;i<n;++i) if(o[i].unknown)
    flipflags |= ((yaoKeyLsb(o[i].yao.w) != o[i].yao.inverted)?1LL<<i:0);
  // Assuming little endian
  if(party != 1) osend(pdsuper,2,&flipflags,bc);
  if(party != 2) { orecv(pdsuper,2,&rv,bc); rv^=flipflags; }
  for(i=0;i<n;++i) if(!o[i].unknown && o[i].knownValue)
    rv |= (1LL<<i);
  pd->ocount+=n;
  if(party!=2) { *dest=rv; return true; }
  else return false;
}
bool yaoEvalRevealOblivBits(ProtocolDesc* pdsuper,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  YaoProtocolDesc* pd = CAST(pdsuper);
  for(i=0;i<n;++i) if(o[i].unknown)
    flipflags |= (yaoKeyLsb(o[i].yao.w)?1LL<<i:0);
  // Assuming little endian
  if(party != 1) { orecv(pdsuper,1,&rv,bc); rv^=flipflags; }
  if(party != 2) osend(pdsuper,1,&flipflags,bc);
  for(i=0;i<n;++i) if(!o[i].unknown && o[i].knownValue)
    rv |= (1LL<<i);
  pd->ocount+=n;
  if(party!=1) { *dest=rv; return true; }
  else return false;
}

// Encodes a 2-input truth table for f(a,b) = ((ttable&(1<<(2*a+b)))!=0)
void yaoGenerateGate(YaoProtocolDesc* pd, OblivBit* r, char ttable, 
    const OblivBit* a, const OblivBit* b)
{ 
  uint64_t k = pd->gcount;
  int im=0,i;
  yao_key_t wa,wb,wc,wt;
  const char* R = pd->R;

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
    osend(PROTOCOL_DESC(pd),2,wt,YAO_KEY_BYTES);
  }

  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,wc);
  r->unknown = true; r->yao.inverted = false;
  pd->gcount++;
}

void yaoEvaluateGate(YaoProtocolDesc* pd, OblivBit* r, char ttable, 
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
  { orecv(PROTOCOL_DESC(pd),1,t,sizeof(yao_key_t));
    if(i==j) yaoKeyCopy(w,t);
  }
  yaoSetHashMask(t,a->yao.w,b->yao.w,pd->gcount++,i);
  yaoKeyXor(w,t);
  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,w);
  r->unknown = true;
}

unsigned yaoGateCount() { return ((YaoProtocolDesc*)currentProto)->gcount; }

/* execYaoProtocol is divided into 2 parts which are reused by other
   protocols such as DualEx */
void setupYaoProtocol(YaoProtocolDesc* pd)
{
  ProtocolDesc* pdb = PROTOCOL_DESC(pd);
  int me = pdb->thisParty;
  pdb->partyCount = 2;
  pd->nonFreeGate = (me==1?yaoGenerateGate:yaoEvaluateGate);
  pdb->feedOblivInputs = (me==1?yaoGenrFeedOblivInputs:yaoEvalFeedOblivInputs);
  pdb->revealOblivBits = (me==1?yaoGenrRevealOblivBits:yaoEvalRevealOblivBits);
  pdb->setBitAnd = yaoSetBitAnd;
  pdb->setBitOr  = yaoSetBitOr;
  pdb->setBitXor = yaoSetBitXor;
  pdb->setBitNot = yaoSetBitNot;
  pdb->flipBit   = yaoFlipBit;

  dhRandomInit(); // FIXME use pthread_once
}
void mainYaoProtocol(YaoProtocolDesc* pd, protocol_run start, void* arg)
{
  ProtocolDesc* pdb = PROTOCOL_DESC(pd);
  int me = pdb->thisParty;
  int tailind,tailpos;
  pd->gcount = pd->icount = pd->ocount = 0;
  if(me==1)
  {
    gcry_randomize(pd->R,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    gcry_randomize(pd->I,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    pd->R[0] |= 1;   // flipper bit

    tailind=YAO_KEY_BYTES-1;
    tailpos=8-(8*YAO_KEY_BYTES-YAO_KEY_BITS);
    pd->R[tailind] &= (1<<tailpos)-1;
    pd->I[tailind] &= (1<<tailpos)-1;
    pd->sender = npotSenderNew(1<<OT_BATCH_SIZE,pdb,me);
  }else pd->recver = npotRecverNew(1<<OT_BATCH_SIZE,pdb,me);

  currentProto = pdb;
  start(arg);

  if(me==1) npotSenderRelease(pd->sender);
  else npotRecverRelease(pd->recver);
}

void execYaoProtocol(YaoProtocolDesc* pd, protocol_run start, void* arg)
{
  setupYaoProtocol(pd);
  mainYaoProtocol(pd,start,arg);
}

void __obliv_c__setBitAnd(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(known(a) || known(b))
  { if(!known(a)) { const OblivBit* t=a; a=b; b=t; }
    if(a->knownValue) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,false);
  }else currentProto->setBitAnd(currentProto,dest,a,b);
}
void __obliv_c__setBitOr(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(known(a) || known(b))
  { if(!known(a)) { const OblivBit* t=a; a=b; b=t; }
    if(!a->knownValue) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,true);
  }else currentProto->setBitOr(currentProto,dest,a,b);
}
void __obliv_c__setBitXor(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  bool v;
  if(known(a) || known(b))
  { if(!known(a)) { const OblivBit* t=a; a=b; b=t; }
    v = a->knownValue;
    __obliv_c__copyBit(dest,b);
    if(v) __obliv_c__flipBit(dest);
  }else currentProto->setBitXor(currentProto,dest,a,b); 
}
void __obliv_c__setBitNot(OblivBit* dest,const OblivBit* a)
{ if(known(dest)){ *dest=*a; dest->knownValue=!dest->knownValue; }
  else currentProto->setBitNot(currentProto,dest,a); 
}
void __obliv_c__flipBit(OblivBit* dest) 
{ if(known(dest)) dest->knownValue = !dest->knownValue;
  else currentProto->flipBit(currentProto,dest); 
}

static void dbgFeedOblivBool(OblivBit* dest,int party,bool a)
{ 
  int curparty = __obliv_c__currentParty();
  
  dest->unknown=true;
  if(party==1) { if(curparty==1) dest->knownValue=a; }
  else if(party==2 && curparty == 1) 
    orecv(currentProto,1,&dest->knownValue,sizeof(bool));
  else if(party==2 && curparty == 2) osend(currentProto,1,&a,sizeof(bool));
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

static void broadcastBits(int source,void* p,size_t n)
{
  int i;
  if(currentProto->thisParty!=source) orecv(currentProto,source,p,n);
  else for(i=1;i<=currentProto->partyCount;++i) if(i!=source)
      osend(currentProto,i,p,n);
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
  currentProto = pd;
  currentProto->debug.mulCount = currentProto->debug.xorCount = 0;
  start(arg);
}

bool __obliv_c__revealOblivBits (widest_t* dest, const OblivBit* src
                                ,size_t size, int party)
  { return currentProto->revealOblivBits(currentProto,dest,src,size,party); }

int __obliv_c__currentParty() { return currentProto->thisParty; }

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
{ while(size-->0) if(!__obliv_c__bitIsKnown(dest++,bits++)) return false;
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

void __obliv_c__setLShift (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt)
{
  int i;
  OblivBit* dest=vdest;
  const OblivBit* src=vsrc;
  for(i=n-1;i>=shiftAmt;--i) __obliv_c__copyBit(dest+i,src+i-shiftAmt);
  for(;i>=0;--i) __obliv_c__assignBitKnown(dest+i,false);
}
void __obliv_c__setRShift (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt,bool isSigned)
{
  int i;
  OblivBit* dest=vdest;
  const OblivBit* src=vsrc;
  for(i=shiftAmt;i<n;++i) __obliv_c__copyBit(dest+i-shiftAmt,src+i);
  (isSigned?__obliv_c__setSignExtend
           :__obliv_c__setZeroExtend)(dest,n,dest,n-shiftAmt);
}
void __obliv_c__setRShiftSigned (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt)
  { __obliv_c__setRShift(vdest,vsrc,n,shiftAmt,true); }
void __obliv_c__setRShiftUnsigned (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt)
  { __obliv_c__setRShift(vdest,vsrc,n,shiftAmt,false); }

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

#define MAX_BITS (8*sizeof(widest_t))
// dest = c?src:0;
static void setZeroOrVal (OblivBit* dest
    ,const OblivBit* src ,size_t size ,const OblivBit* c)
{
  int i;
  for(i=0;i<size;++i) __obliv_c__setBitAnd(dest+i,src+i,c);
}
void __obliv_c__condNeg (const void* vcond, void* vdest
                        ,const void* vsrc, size_t n)
{
  int i;
  OblivBit c,t, *dest = vdest;
  const OblivBit *src = vsrc;
  __obliv_c__copyBit(&c,vcond);
  for(i=0;i<n-1;++i)
  { __obliv_c__setBitXor(dest+i,src+i,vcond);  // flip first
    __obliv_c__setBitXor(&t,dest+i,&c);        // then, conditional increment
    __obliv_c__setBitAnd(&c,&c,dest+i);
    __obliv_c__copyBit(dest+i,&t);
  }
  __obliv_c__setBitXor(dest+i,src+i,vcond);
  __obliv_c__setBitXor(dest+i,dest+i,&c);
}

// get absolute value and sign
static void setAbs (void* vdest, void* vsign, const void* vsrc, size_t n)
{
  __obliv_c__copyBit(vsign,((const OblivBit*)vsrc)+n-1);
  __obliv_c__condNeg(vsign,vdest,vsrc,n);
}

void __obliv_c__setNeg (void* vdest, const void* vsrc, size_t n)
{
  __obliv_c__condNeg(&__obliv_c__trueCond,vdest,vsrc,n);
}

void __obliv_c__setMul (void* vdest
                       ,const void* vop1 ,const void* vop2
                       ,size_t size)
{
  const OblivBit *op1=vop1, *op2=vop2;
  OblivBit temp[MAX_BITS],sum[MAX_BITS];
  int i;
  assert(size<=MAX_BITS);
  __obliv_c__setUnsignedKnown(sum,0,size);
  for(i=0;i<size;++i)
  { setZeroOrVal(temp,op1,size-i,op2+i);
    __obliv_c__setPlainAdd(sum+i,sum+i,temp,size-i);
  }
  __obliv_c__copyBits(vdest,sum,size);
}

// All parameters have equal number of bits
void __obliv_c__setDivModUnsigned (void* vquot, void* vrem
                                  ,const void* vop1, const void* vop2
                                  ,size_t n)
{
  const OblivBit *op1=vop1, *op2=vop2;
  OblivBit overflow[MAX_BITS]; // overflow[i] = does it overflow on op2<<i
  OblivBit temp[MAX_BITS],rem[MAX_BITS],quot[MAX_BITS],b;
  int i;
  assert(n<=MAX_BITS);
  __obliv_c__copyBits(rem,op1,n);
  __obliv_c__assignBitKnown(overflow,false);
  for(i=1;i<n;++i) __obliv_c__setBitOr(overflow+i,overflow+i-1,op2+n-i);
  for(i=n-1;i>=0;--i)
  {
    __obliv_c__setBitsSub(temp,&b,rem+i,op2,NULL,n-i);
    __obliv_c__setBitOr(&b,&b,overflow+i);
    __obliv_c__ifThenElse(rem+i,rem+i,temp,n-i,&b);
    __obliv_c__setBitNot(quot+i,&b);
  }
  if(vrem)  __obliv_c__copyBits(vrem,rem,n);
  if(vquot) __obliv_c__copyBits(vquot,quot,n);
}

void __obliv_c__setDivModSigned (void* vquot, void* vrem
                                ,const void* vop1, const void* vop2
                                ,size_t n)
{
  OblivBit neg1,neg2;
  OblivBit op1[MAX_BITS],op2[MAX_BITS];
  setAbs(op1,&neg1,vop1,n);
  setAbs(op2,&neg2,vop2,n);
  __obliv_c__setDivModUnsigned(vquot,vrem,op1,op2,n);
  __obliv_c__setBitXor(&neg2,&neg2,&neg1);
  if(vrem)  __obliv_c__condNeg(&neg1,vrem,vrem,n);
  if(vquot) __obliv_c__condNeg(&neg2,vquot,vquot,n);
}
void __obliv_c__setDivUnsigned (void* vdest
                               ,const void* vop1 ,const void* vop2
                               ,size_t n)
{
  __obliv_c__setDivModUnsigned(vdest,NULL,vop1,vop2,n);
}
void __obliv_c__setModUnsigned (void* vdest
                               ,const void* vop1 ,const void* vop2
                               ,size_t n)
{
  __obliv_c__setDivModUnsigned(NULL,vdest,vop1,vop2,n);
}
void __obliv_c__setDivSigned (void* vdest
                             ,const void* vop1 ,const void* vop2
                             ,size_t n)
{
  __obliv_c__setDivModSigned(vdest,NULL,vop1,vop2,n);
}
void __obliv_c__setModSigned (void* vdest
                             ,const void* vop1 ,const void* vop2
                             ,size_t n)
{
  __obliv_c__setDivModSigned(NULL,vdest,vop1,vop2,n);
}
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
  { currentProto->feedOblivInputs(currentProto,spec,count,party); }

#define feedOblivFun(t,ot,tname) \
    __obliv_c__##ot feedObliv##tname (t v,int party) \
    { __obliv_c__##ot rv; \
      OblivInputs spec; \
      setupObliv##tname(&spec,&rv,v); \
      feedOblivInputs(&spec,1,party); \
      return rv; \
    }

feedOblivFun(bool,bool,Bool)
feedOblivFun(char,char,Char)
feedOblivFun(short,short,Short)
feedOblivFun(int,int,Int)
feedOblivFun(long,long,Long)
feedOblivFun(long long,lLong,LLong)

#undef feedOblivFun

// TODO pass const values by ref later
bool revealOblivBool(bool* dest,__obliv_c__bool src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,1,party)) 
    { *dest=(bool)wd; return true; }
  return false;
}
bool revealOblivChar(char* dest, __obliv_c__char src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,bitsize(char),party)) 
    { *dest=(char)wd; return true; }
  return false;
}
bool revealOblivInt(int* dest, __obliv_c__int src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,bitsize(char),party)) 
    { *dest=(int)wd; return true; }
  return false;
}
bool revealOblivShort(short* dest, __obliv_c__short src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,bitsize(char),party)) 
    { *dest=(short)wd; return true; }
  return false;
}
bool revealOblivLong(long* dest, __obliv_c__long src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,bitsize(char),party)) 
    { *dest=(long)wd; return true; }
  return false;
}
bool revealOblivLLong(long long* dest, __obliv_c__lLong src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,bitsize(char),party)) 
    { *dest=(long long)wd; return true; }
  return false;
}

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
