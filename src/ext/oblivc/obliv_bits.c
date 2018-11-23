// TODO I need to fix some int sizes
#include <obliv_bits.h>
#include <obliv_float_ops.h>
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

// Q: What's with all these casts to and from void* ?
// A: Code generation becomes easier without the need for extraneous casts.
//      Might fix it some day. But user code never sees these anyway.

// Right now, we do not support multiple protocols at the same time
static __thread ProtocolDesc *currentProto;

static inline bool known(const OblivBit* o) { return !o->unknown; }

// --------------------------- STDIO trans -----------------------------------

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
  pt->flush(pt);
  return fread(s,1,n,stdin);
}

static int stdioFlush(ProtocolTransport* pt) {
  bool *p = stdioFlushFlag(pt);
  int status = 0;
  if(*p) {
    status = fflush(stdout);
    if(status == 0) {
      *p=false;
    }
  }
  return status;
}

static void stdioCleanup(ProtocolTransport* pt) {
  pt->flush(pt);
}

// Extremely simple, no multiplexing: two parties, one connection
static struct stdioTransport stdioTransport
  = {{.maxParties=2, .split=NULL, .send=stdioSend, .recv=stdioRecv, .flush=stdioFlush,
      .cleanup = stdioCleanup},false};

void protocolUseStdio(ProtocolDesc* pd)
  { pd->trans = &stdioTransport.cb; }

// --------------------------- TCP trans -----------------------------------

// TCP connections for 2-Party protocols. Ignores src/dest parameters
//   since there is only one remote
typedef struct tcp2PTransport
{ ProtocolTransport cb;
  int sock;
  bool isClient;
  bool isProfiled;
  FILE* sockStream;
  bool needFlush;
  bool keepAlive;
  int sinceFlush;
  size_t bytes;
  size_t flushCount;
  struct tcp2PTransport* parent;
} tcp2PTransport;

// Profiling output
size_t tcp2PBytesSent(ProtocolDesc* pd) { return ((tcp2PTransport*)(pd->trans))->bytes; }
size_t tcp2PFlushCount(ProtocolDesc* pd) { return ((tcp2PTransport*)(pd->trans))->flushCount; }

static int tcp2PSend(ProtocolTransport* pt,int dest,const void* s,size_t n)
{
  struct tcp2PTransport* tcpt = CAST(pt);
  size_t n2=0;
  tcpt->needFlush=true;
  while(n>n2) {
    int res = fwrite(n2+(char*)s,1,n-n2,tcpt->sockStream);
    if(res<0) { perror("TCP write error: "); return res; }
    n2+=res;
  }
  return n2;
}

static int tcp2PSendProfiled(ProtocolTransport* pt,int dest,const void* s,size_t n)
{
  struct tcp2PTransport* tcpt = CAST(pt);
  size_t res = tcp2PSend(pt, dest, s, n);
  if (res >= 0) tcpt->bytes += res;
  return res;
}

static int tcp2PRecv(ProtocolTransport* pt,int src,void* s,size_t n)
{
  struct tcp2PTransport* tcpt = CAST(pt);
  int res=0,n2=0;
  if (tcpt->needFlush)
  {
    transFlush(pt);
    tcpt->needFlush=false;
  }
  while(n>n2)
  {
    res = fread(n2+(char*)s,1,n-n2, tcpt->sockStream);
    if(res<0 || feof(tcpt->sockStream))
    {
      perror("TCP read error: ");
      return res;
    }
    n2+=res;
  }
  return res;
}

static int tcp2PFlush(ProtocolTransport* pt)
{
  struct tcp2PTransport* tcpt = CAST(pt);
  return fflush(tcpt->sockStream);
}

static int tcp2PFlushProfiled(ProtocolTransport* pt)
{
  struct tcp2PTransport* tcpt = CAST(pt);
  if(tcpt->needFlush) tcpt->flushCount++;
  return tcp2PFlush(pt);
}

static void tcp2PCleanup(ProtocolTransport* pt)
{
  tcp2PTransport* t = CAST(pt);
  fflush(t->sockStream);
  if(!t->keepAlive) fclose(t->sockStream);
  free(pt);
}

static void tcp2PCleanupProfiled(ProtocolTransport* pt)
{
  tcp2PTransport* t = CAST(pt);
  t->flushCount++;
  if(t->parent!=NULL)
  { t->parent->bytes+=t->bytes;
    t->parent->flushCount+=t->flushCount;
  }
  tcp2PCleanup(pt);
}

static inline bool transIsTcp2P(ProtocolTransport* pt) { return pt->cleanup == tcp2PCleanup; }

FILE* transGetFile(ProtocolTransport* t)
{
  if(transIsTcp2P(t)) return ((tcp2PTransport*)t)->sockStream;
  else return NULL;
}

static ProtocolTransport* tcp2PSplit(ProtocolTransport* tsrc);

static const tcp2PTransport tcp2PTransportTemplate
  = {{.maxParties=2, .split=tcp2PSplit, .send=tcp2PSend, .recv=tcp2PRecv, .flush=tcp2PFlush,
      .cleanup = tcp2PCleanup},
     .sock=0, .isClient=0, .needFlush=false, .bytes=0, .flushCount=0,
     .parent=NULL};

static const tcp2PTransport tcp2PProfiledTransportTemplate
  = {{.maxParties=2, .split=tcp2PSplit, .send=tcp2PSendProfiled, .recv=tcp2PRecv,
     .flush=tcp2PFlushProfiled, .cleanup = tcp2PCleanupProfiled},
     .sock=0, .isClient=0, .needFlush=false, .bytes=0, .flushCount=0,
     .parent=NULL};

// isClient value will only be used for the split() method, otherwise
// its value doesn't matter. In that case, it indicates which party should be
// the server vs. client for the new connections (which is usually the same as
// the old roles).
static tcp2PTransport* tcp2PNew(int sock,bool isClient, bool isProfiled)
{ tcp2PTransport* trans = malloc(sizeof(*trans));
  if (isProfiled) {
    *trans = tcp2PProfiledTransportTemplate;
  } else {
    *trans = tcp2PTransportTemplate;
  }
  trans->sock = sock;
  trans->isProfiled=isProfiled;
  trans->isClient=isClient;
  trans->sockStream=fdopen(sock, "rb+");
  trans->sinceFlush = 0;
  const int one=1;
  setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&one,sizeof(one));
  /*setvbuf(trans->sockStream, trans->buffer, _IOFBF, BUFFER_SIZE);*/
  return trans;
}

void protocolUseTcp2P(ProtocolDesc* pd,int sock,bool isClient)
{
  pd->trans = &tcp2PNew(sock,isClient, false)->cb;
  tcp2PTransport* t = CAST(pd->trans);
  t->keepAlive = false;
}

void protocolUseTcp2PProfiled(ProtocolDesc* pd,int sock,bool isClient)
{
  pd->trans = &tcp2PNew(sock,isClient, true)->cb;
  tcp2PTransport* t = CAST(pd->trans);
  t->keepAlive = false;
}

void protocolUseTcp2PKeepAlive(ProtocolDesc* pd,int sock,bool isClient)
{
  pd->trans = &tcp2PNew(sock,isClient, false)->cb;
  tcp2PTransport* t = CAST(pd->trans);
  t->keepAlive = true;
}

static int getsockaddr(const char* name,const char* port, struct sockaddr* res)
{
  struct addrinfo *list, *iter;
  if(getaddrinfo(name,port,NULL,&list) < 0) return -1;
  for(iter=list;iter!=NULL && iter->ai_family!=AF_INET;iter=iter->ai_next);
  if(!iter) { freeaddrinfo(list); return -1; }
  memcpy(res,iter->ai_addr,iter->ai_addrlen);
  freeaddrinfo(list);
  return 0;
}
// used as sock=tcpConnect(...); ...; close(sock);
static int tcpConnect(struct sockaddr_in* sa)
{
  int outsock;
  if((outsock=socket(AF_INET,SOCK_STREAM,0))<0) return -1;
  if(connect(outsock,(struct sockaddr*)sa,sizeof(*sa))<0) return -1;
  return outsock;
}

int protocolConnectTcp2P(ProtocolDesc* pd,const char* server,const char* port)
{
  struct sockaddr_in sa;
  if(getsockaddr(server,port,(struct sockaddr*)&sa)<0) return -1; // dns error
  int sock=tcpConnect(&sa); if(sock<0) return -1;
  protocolUseTcp2P(pd,sock,true);
  return 0;
}

int protocolConnectTcp2PProfiled(ProtocolDesc* pd,const char* server,const char* port)
{
  struct sockaddr_in sa;
  if(getsockaddr(server,port,(struct sockaddr*)&sa)<0) return -1; // dns error
  int sock=tcpConnect(&sa); if(sock<0) return -1;
  protocolUseTcp2PProfiled(pd,sock,true);
  return 0;
}

// used as sock=tcpListenAny(...); sock2=accept(sock); ...; close(both);
static int tcpListenAny(const char* portn)
{
  in_port_t port;
  int outsock;
  if(sscanf(portn,"%hu",&port)<1) return -1;
  if((outsock=socket(AF_INET,SOCK_STREAM,0))<0) return -1;
  int reuse = 1;
  if (setsockopt(outsock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
  { fprintf(stderr,"setsockopt(SO_REUSEADDR) failed\n"); return -1; }

  struct sockaddr_in sa = { .sin_family=AF_INET, .sin_port=htons(port)
                          , .sin_addr={INADDR_ANY} };
  if(bind(outsock,(struct sockaddr*)&sa,sizeof(sa))<0) return -1;
  if(listen(outsock,SOMAXCONN)<0) return -1;
  return outsock;
}

int protocolAcceptTcp2P(ProtocolDesc* pd,const char* port)
{
  int listenSock, sock;
  listenSock = tcpListenAny(port);
  if((sock=accept(listenSock,0,0))<0) return -1;
  protocolUseTcp2P(pd,sock,false);
  close(listenSock);
  return 0;
}

int protocolAcceptTcp2PProfiled(ProtocolDesc* pd,const char* port)
{
  int listenSock, sock;
  listenSock = tcpListenAny(port);
  if((sock=accept(listenSock,0,0))<0) return -1;
  protocolUseTcp2PProfiled(pd,sock,false);
  close(listenSock);
  return 0;
}

/*
   If two parties connected over a given socket execute this function
   (one with isClient true and the other with false), they both end up with
   a new socket that they can use in parallel with the old one. This is useful
   just before two parties are planning to spawn a new thread each, such that
   the two threads can have an independent channel with the corresponding thread
   on the remote side. Meant to work on TCP sockets only.

   Needs transport object to send the new port number along.
   Used only with tcp2PSplit, so won't need a party number.
   */
static int sockSplit(int sock,ProtocolTransport* t,bool isClient)
{
  struct sockaddr_in sa; socklen_t sz=sizeof(sa);
  if(isClient)
  {
    if(getpeername(sock,(struct sockaddr*)&sa,&sz)<0) return -1;
    //int rres=read(sock,&sa.sin_port,sizeof(sa.sin_port));
    int rres = transRecv(t,0,&sa.sin_port,sizeof(sa.sin_port));
    if(rres<0) { fprintf(stderr,"Socket read error\n"); return -1; }
    if(rres<sizeof(sa.sin_port))
      { fprintf(stderr,"BUG: fix with repeated reads\n"); return -1; }
    return tcpConnect(&sa);
  }
  else
  { // any change here should also change PROFILE_NETWORK in tcp2PSplit()
    int listenSock=tcpListenAny("0");
    if(getsockname(listenSock,(struct sockaddr*)&sa,&sz)<0) return -1;
    //if(write(sock,&sa.sin_port,sizeof(sa.sin_port))<0) return -1;
    if(transSend(t,0,&sa.sin_port,sizeof(sa.sin_port))<0) return -1;
    transFlush(t);
    int newsock = accept(listenSock,0,0);
    close(listenSock);
    return newsock;
  }
}

static ProtocolTransport* tcp2PSplit(ProtocolTransport* tsrc)
{
  tcp2PTransport* t = CAST(tsrc);
  transFlush(tsrc);
  // I should really rewrite sockSplit to use FILE* sockStream
  int newsock = sockSplit(t->sock,tsrc,t->isClient);
  if(newsock<0) { fprintf(stderr,"sockSplit() failed\n"); return NULL; }
  tcp2PTransport* tnew = tcp2PNew(newsock,t->isClient,t->isProfiled);
  tnew->parent=t;
  return CAST(tnew);
}

typedef struct
{ ProtocolTransport cb;
  ProtocolDesc pd;
} SizeCheckTransportAdapter; // spliced object

static int sizeCheckSend(ProtocolTransport* pt,int dest,const void* s,size_t n)
{ int sent = osend(&((SizeCheckTransportAdapter*)pt)->pd,dest,s,n);
  if(sent==n) return n;
  else
  { fprintf(stderr,"Was going to send %zu bytes to %d, sent %d\n",
                   n,dest,sent);
    if(sent<0) fprintf(stderr,"That means %s\n",strerror(sent));
    exit(-1);
  }
}

static int sizeCheckRecv(ProtocolTransport* pt,int src,void* s,size_t n)
{ int recv = orecv(&((SizeCheckTransportAdapter*)pt)->pd,src,s,n);
  if(recv==n) return n;
  else
  { fprintf(stderr,"Was going to recv %zu bytes from %d, received %d\n",
                    n,src,recv);
    if(recv<0) fprintf(stderr,"That means %s\n",strerror(recv));
    exit(-1);
  }
}
static void sizeCheckCleanup(ProtocolTransport* pt)
{ ProtocolTransport *inner = ((SizeCheckTransportAdapter*)pt)->pd.trans;
  inner->cleanup(inner);
  free(pt);
}

void protocolAddSizeCheck(ProtocolDesc* pd)
{
  SizeCheckTransportAdapter* t = malloc(sizeof(SizeCheckTransportAdapter));
  t->pd = *pd; // Dummy protocol object, sliced just for the Transport object
  pd->trans = &t->cb;
  t->cb.send=sizeCheckSend;
  t->cb.recv=sizeCheckRecv;
  t->cb.cleanup=sizeCheckCleanup;
}

// --------------------------- Protocols -----------------------------------

int ocCurrentParty() { return currentProto->currentParty(currentProto); }
int ocCurrentPartyDefault(ProtocolDesc* pd) { return pd->thisParty; }
void setCurrentParty(ProtocolDesc* pd, int party)
  { pd->thisParty=party; }

ProtocolDesc* ocCurrentProto() { return currentProto; }
void ocSetCurrentProto(ProtocolDesc* pd) { currentProto=pd; }

bool ocCanSplitProto(ProtocolDesc * pdin)
{
  return pdin->trans->split != NULL
        && pdin->splitextra != NULL
        && pdin->extra != NULL;
}

bool ocSplitProto(ProtocolDesc* pdout, ProtocolDesc * pdin)
{ if (!ocCanSplitProto(pdin)) return false;
  else
  {
    *pdout = (struct ProtocolDesc) {
      .error = pdin->error,
      .partyCount = pdin->error,
      .currentParty = pdin->currentParty,
      .feedOblivInputs = pdin->feedOblivInputs,
      .revealOblivBits = pdin->revealOblivBits,
      .setBitAnd = pdin->setBitAnd,
      .setBitOr = pdin->setBitOr,
      .setBitXor = pdin->setBitXor,
      .setBitNot = pdin->setBitNot,
      .flipBit = pdin->flipBit,
      .thisParty = pdin->thisParty,
      .trans = pdin->trans->split(pdin->trans),
      .splitextra = pdin->splitextra,
      .cleanextra = pdin->cleanextra,
      .extra = NULL
    };
    pdout->splitextra(pdout, pdin);
    return true;
  }
}

void ocCleanupProto(ProtocolDesc* pd)
  {
    pd->trans->cleanup(pd->trans);
    if (pd->extra != NULL) pd->cleanextra(pd);
  }

void cleanupProtocol(ProtocolDesc* pd)
{
  ocCleanupProto(pd);
}

// --------------------------- Debug Protocol ----------------------------

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

bool ocInDebugProto(void) { return ocCurrentProto()->extra==NULL; }

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
  uint64_t* ic = &pd->icount;
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
  { size_t bc = bitCount(&it);
    // limit memory usage
    size_t batch = (bc<YAO_FEED_MAX_BATCH?bc:YAO_FEED_MAX_BATCH);
    char *buf0 = malloc(batch*YAO_KEY_BYTES),
         *buf1 = malloc(batch*YAO_KEY_BYTES);
    size_t bp=0;
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
  { size_t bc = bitCount(&it);
    // limit memory usage
    size_t batch = (bc<YAO_FEED_MAX_BATCH?bc:YAO_FEED_MAX_BATCH);
    char *buf = malloc(batch*YAO_KEY_BYTES),
         **dest = malloc(batch*sizeof(char*));
    bool *sel = malloc(batch*sizeof(bool));
    size_t bp=0,i;
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
  size_t i,bc=(n+7)/8;
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
  size_t i,bc=(n+7)/8;
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
{ uint64_t rv = ((YaoProtocolDesc*)currentProto->extra)->gcount - ((YaoProtocolDesc*)currentProto->extra)->gcount_offset;
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
  if (ypd->ownOT) {
    if(me==1) otSenderRelease(&ypd->sender);
    else otRecverRelease(&ypd->recver);
  }
}

void splitYaoProtocolExtra(ProtocolDesc* pdout, ProtocolDesc * pdin) {
  YaoProtocolDesc* ypdin = pdin->extra;
  YaoProtocolDesc* ypdout = malloc(sizeof(YaoProtocolDesc));
  pdout->extra=ypdout;

  ypdout->protoType = ypdin->protoType;
  ypdout->extra = NULL;
  ypdout->icount = ypdout->ocount = 0;
  ypdout->ownOT = true; // For now we don't do anything special for OT on forked protocols
  gcry_cipher_open(&ypdout->fixedKeyCipher,yaoFixedKeyAlgo,GCRY_CIPHER_MODE_ECB,0);
  gcry_cipher_setkey(ypdout->fixedKeyCipher,yaoFixedKey,sizeof(yaoFixedKey)-1);
  if (pdout->thisParty == 1) {
    memcpy(ypdout->R,ypdin->R,YAO_KEY_BYTES);
    gcry_randomize(ypdout->I,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    gcry_randomize(&ypdout->gcount_offset,sizeof(ypdout->gcount_offset),GCRY_STRONG_RANDOM);
    osend(pdout,2,&ypdout->gcount_offset,sizeof(ypdout->gcount_offset));
    ypdout->sender = honestOTExtSenderAbstract(honestOTExtSenderNew(pdout,2));
  } else {
    orecv(pdout,1,&ypdout->gcount_offset,sizeof(ypdout->gcount_offset));
    ypdout->recver = honestOTExtRecverAbstract(honestOTExtRecverNew(pdout,1));
  }
  ypdout->gcount = ypdout->gcount_offset;
  oflush(pdin); oflush(pdout);
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

  pd->splitextra = splitYaoProtocolExtra;
  pd->cleanextra = cleanupYaoProtocol;
}

// point_and_permute should always be true.
// It is false only in the NP protocol, where evaluator knows everything
void mainYaoProtocol(ProtocolDesc* pd, bool point_and_permute,
                     protocol_run start, void* arg)
{
  YaoProtocolDesc* ypd = pd->extra;
  int me = pd->thisParty;
  ypd->ownOT=false;
  ypd->gcount = ypd->gcount_offset = ypd->icount = ypd->ocount = 0;
  if(me==1)
  {
    gcry_randomize(ypd->R,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    gcry_randomize(ypd->I,YAO_KEY_BYTES,GCRY_STRONG_RANDOM);
    if(point_and_permute) ypd->R[0] |= 1;   // flipper bit

    if(ypd->sender.sender==NULL)
    { ypd->ownOT=true;
      ypd->sender = honestOTExtSenderAbstract(honestOTExtSenderNew(pd,2));
    }
  }else
    if(ypd->recver.recver==NULL)
    { ypd->ownOT=true;
      ypd->recver = honestOTExtRecverAbstract(honestOTExtRecverNew(pd,1));
    }

  currentProto = pd;
  start(arg);
}

void cleanupYaoProtocol(ProtocolDesc* pd)
{
  YaoProtocolDesc* ypd = pd->extra;
  gcry_cipher_close(ypd->fixedKeyCipher);
  yaoReleaseOt(pd, pd->thisParty);
  free(ypd);
  pd->extra = NULL;
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
  cleanupYaoProtocol(pd);
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

// --------------------------- NNOB protocol ---------------------------------

/*void nnobAndGatesCount(ProtocolDesc* pd, protocol_run start, void* arg)*/
/*{*/
  /*pd->currentParty = ocCurrentPartyDefault;*/
  /*pd->error = 0;*/
  /*pd->feedOblivInputs = dbgProtoFeedOblivInputs;*/
  /*pd->revealOblivBits = dbgProtoRevealOblivBits;*/
  /*pd->setBitAnd = dbgProtoSetBitAnd;*/
  /*pd->setBitOr  = dbgProtoSetBitOr;*/
  /*pd->setBitXor = dbgProtoSetBitXor;*/
  /*pd->setBitNot = dbgProtoSetBitNot;*/
  /*pd->flipBit   = dbgProtoFlipBit;*/
  /*pd->partyCount= 2;*/
  /*pd->extra = NULL;*/
  /*currentProto = pd;*/
  /*currentProto->debug.mulCount = currentProto->debug.xorCount = 0;*/
  /*start(arg);*/
/*}*/

#ifdef ENABLE_NNOB
void setupNnobProtocol(ProtocolDesc* pd) {
  NnobProtocolDesc* npd = malloc(sizeof(NnobProtocolDesc));
  pd->extra=npd;
  pd->error = 0;
  pd->partyCount = 2;
  pd->currentParty = ocCurrentPartyDefault;
  pd->feedOblivInputs = nnobFeedOblivInputs;
  pd->revealOblivBits = nnobRevealOblivInputs;
  pd->setBitAnd = nnobSetBitAnd;
  pd->setBitOr  = nnobSetBitOr;
  pd->setBitXor = nnobSetBitXor;
  pd->setBitNot = nnobSetBitNot;
  pd->flipBit   = nnobFlipBit;
}


void mainNnobProtocol(ProtocolDesc* pd, int numOTs, OTExtValidation validation, protocol_run start, void* arg) {
  /*int denom = logfloor(numOTs, 2)+1;*/
  /*int bucketSize = (int)(1 + (NNOB_KEY_BYTES*8)/denom);*/
  int bucketSize = BUCKET_SIZE;
  NnobProtocolDesc* npd = pd->extra;
  npd->bucketSize = bucketSize;
  int n = ((numOTs+7)/8)*8;
  int destparty = pd->thisParty==1?1:2;
  memset(npd->cumulativeHashCheckKey, 0, NNOB_KEY_BYTES);
  memset(npd->cumulativeHashCheckMac, 0, NNOB_KEY_BYTES);
  char dummy[NNOB_HASH_ALGO_KEYBYTES];
  dhRandomInit();
  npd->gen= newBCipherRandomGenByAlgoKey(NNOB_HASH_ALGO, dummy);
  npd->nonce = 0;

  npd->aBitsShareAndMac.counter = 0;
  npd->aBitsShareAndMac.n = n;
  npd->aBitsShareAndMac.share = malloc(n*NNOB_KEY_BYTES);
  npd->aBitsShareAndMac.mac = malloc(n*NNOB_KEY_BYTES);

  npd->aBitsKey.key = malloc(n*NNOB_KEY_BYTES);
  npd->aBitsKey.counter = 0;
  npd->aBitsKey.n = n;

  setupFDeal(npd, numOTs);

  npd->error = false;

  char mat1[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
  char mat2[8*A_BIT_PARAMETER_BYTES*NNOB_KEY_BYTES];
  char (*aBitFullMac)[A_BIT_PARAMETER_BYTES] = malloc(numOTs*A_BIT_PARAMETER_BYTES);
  char (*aBitFullKey)[A_BIT_PARAMETER_BYTES] = malloc(numOTs*A_BIT_PARAMETER_BYTES);
  if(destparty==1)
  {
    npd->error |= !WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share,
        mat1, aBitFullMac, numOTs, validation, destparty);
    npd->error |= !WaBitBoxGetKey(pd, npd->globalDelta,
        mat2, aBitFullKey, numOTs, validation, destparty);
    WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat1, numOTs);
    WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat2, numOTs);
    npd->error |= !aOTKeyOfZ(pd, &npd->FDeal.aOTKeyOfZ);
    npd->error |= !aOTShareAndMacOfZ(pd, &npd->FDeal.aOTShareAndMacOfZ);
    npd->error |= !aANDShareAndMac(pd, &npd->FDeal.aANDShareAndMac);
    npd->error |= !aANDKey(pd, &npd->FDeal.aANDKey);
  }
  else
  {
    npd->error |= !WaBitBoxGetKey(pd, npd->globalDelta,
        mat1, aBitFullKey, numOTs, validation, destparty);
    npd->error |= !WaBitBoxGetBitAndMac(pd, npd->aBitsShareAndMac.share,
        mat2, aBitFullMac, numOTs, validation, destparty);
    WaBitToaBit(npd->aBitsShareAndMac.mac, aBitFullMac, mat2, numOTs);
    WaBitToaBit(npd->aBitsKey.key, aBitFullKey, mat1, numOTs);
    npd->error |= !aOTShareAndMacOfZ(pd, &npd->FDeal.aOTShareAndMacOfZ);
    npd->error |= !aOTKeyOfZ(pd, &npd->FDeal.aOTKeyOfZ);
    npd->error |= !aANDKey(pd, &npd->FDeal.aANDKey);
    npd->error |= !aANDShareAndMac(pd, &npd->FDeal.aANDShareAndMac);
  }
  free(aBitFullKey);
  free(aBitFullMac);
  currentProto = pd;
  start(arg);
}

void cleanupNnobProtocol(ProtocolDesc* pd)
{
  NnobProtocolDesc* npd = pd->extra;
  releaseBCipherRandomGen(npd->gen);
  free(npd->aBitsShareAndMac.share);
  free(npd->aBitsShareAndMac.mac);
  free(npd->aBitsKey.key);
  free(npd->FDeal.aOTShareAndMacOfZ.x0);
  free(npd->FDeal.aOTShareAndMacOfZ.x1);
  free(npd->FDeal.aOTShareAndMacOfZ.c);
  free(npd->FDeal.aOTShareAndMacOfZ.z);
  free(npd->FDeal.aOTKeyOfZ.x0);
  free(npd->FDeal.aOTKeyOfZ.x1);
  free(npd->FDeal.aOTKeyOfZ.c);
  free(npd->FDeal.aOTKeyOfZ.z);
  free(npd->FDeal.aANDShareAndMac.x);
  free(npd->FDeal.aANDShareAndMac.y);
  free(npd->FDeal.aANDShareAndMac.z);
  free(npd->FDeal.aANDKey.x);
  free(npd->FDeal.aANDKey.y);
  free(npd->FDeal.aANDKey.z);
  free(npd);
}

void execNnobProtocol(ProtocolDesc* pd, protocol_run start, void* arg, int numOTs, bool useAltOTExt) {
  OTExtValidation validation = useAltOTExt?OTExtValidation_byPair:OTExtValidation_hhash;
  setupNnobProtocol(pd);
  mainNnobProtocol(pd, numOTs, validation, start, arg);

  NnobProtocolDesc* npd = pd->extra;
  int numANDGates = npd->FDeal.aANDKey.n;
  fprintf(stderr, "num of ANDs: %d\n", numANDGates);
  fprintf(stderr, "OTs per AND: %d\n", numOTs/numANDGates);
  fprintf(stderr, "OTs left: %d\n", npd->aBitsShareAndMac.n-npd->aBitsKey.counter);
  fprintf(stderr, "Bucket Size: %d\n", npd->bucketSize);
  cleanupNnobProtocol(pd);
}
#endif // ENABLE_NNOB

// --------------------------- Obliv-c primitives --------------------------------

void __obliv_c__assignBitKnown(OblivBit* dest, bool value)
  { dest->knownValue = value; dest->unknown=false; }

void __obliv_c__copyBit(OblivBit* dest,const OblivBit* src)
  { if(dest!=src) *dest=*src; }

bool __obliv_c__bitIsKnown(bool* v,const OblivBit* bit)
{ if(known(bit)) *v=bit->knownValue;
  return known(bit);
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
{ if(known(a)){ *dest=*a; dest->knownValue=!dest->knownValue; }
  else currentProto->setBitNot(currentProto,dest,a);
}
void __obliv_c__flipBit(OblivBit* dest)
{ if(known(dest)) dest->knownValue = !dest->knownValue;
  else currentProto->flipBit(currentProto,dest);
}

void __obliv_c__setupOblivBits(OblivInputs* spec,OblivBit* dest
                                     ,widest_t v,size_t size)
{ spec->dest=dest;
  spec->src=v;
  spec->size=size;
}

static void broadcastBits(int source,void* p,size_t n)
{
  int i;
  if(ocCurrentParty()!=source) orecv(currentProto,source,p,n);
  else for(i=1;i<=currentProto->partyCount;++i) if(i!=source)
      osend(currentProto,i,p,n);
}

bool __obliv_c__revealOblivBits (widest_t* dest, const OblivBit* src
                                ,size_t size, int party)
  { return currentProto->revealOblivBits(currentProto,dest,src,size,party); }

void __obliv_c__setSignedKnown
  (void* vdest, size_t size, long long signed value)
{
  OblivBit* dest=vdest;
  while(size-->0)
  { __obliv_c__assignBitKnown(dest,value&1);
    value>>=1;
    dest++;
  }
}

void __obliv_c__setFloatKnown(void* vdest, size_t size, float value) {
  unsigned char* floatBytes = (unsigned char*) &value;
  OblivBit* dest = vdest;
  int i = 0;
  int j = 0;
  unsigned char currentByte = floatBytes[j];
  while(size-- > 0)
  { __obliv_c__assignBitKnown(dest, currentByte & 0x01);
    currentByte >>= 1;
    dest++;
    i++;
    if (i == 8)
    { i = 0;
      j++;
      currentByte = floatBytes[j];
    }
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
  for(i=n-1;i>=(signed)shiftAmt;--i) __obliv_c__copyBit(dest+i,src+i-shiftAmt);
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

void __obliv_c__setPlainAddF (void* vdest
                          ,const void* vop1 ,const void* vop2
                          ,size_t size)
{ OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_add_circuit(dest, op1, op2);
}

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

void __obliv_c__setPlainSubF (void* vdest
                             ,const void* vop1 ,const void* vop2
                             ,size_t size)
{ OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_sub_circuit(dest, op1, op2);
}

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

void __obliv_c__condNegF (const void* vcond, void* vdest
                        ,const void* vsrc, size_t n)
{
  OblivBit *dest=vdest;
  const OblivBit *op1=vsrc, *op2=vcond;
  obliv_float_neg_circuit(dest, op1, op2);
}

void __obliv_c__setNegF (void* vdest, const void* vsrc, size_t n)
{
  __obliv_c__condNegF(&__obliv_c__trueCond,vdest,vsrc,n);
}

void __obliv_c__setMul (void* vdest
                       ,const void* vop1 ,const void* vop2
                       ,size_t size)
{
  const OblivBit *op1=vop1, *op2=vop2;
  OblivBit temp[MAX_BITS]={},sum[MAX_BITS]={};
  int i;
  assert(size<=MAX_BITS);
  __obliv_c__setUnsignedKnown(sum,0,size);
  for(i=0;i<size;++i)
  { setZeroOrVal(temp,op1,size-i,op2+i);
    __obliv_c__setPlainAdd(sum+i,sum+i,temp,size-i);
  }
  __obliv_c__copyBits(vdest,sum,size);
}

void __obliv_c__setMulF (void* vdest
                        ,const void* vop1,const void* vop2
                        ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_mult_circuit(dest, op1, op2);
}

void __obliv_c__setDivF (void* vdest
                        ,const void* vop1,const void* vop2
                        ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_div_circuit(dest, op1, op2);
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

void __obliv_c__condAssign (const void* cond,void* dest, const void* src,
                            size_t size)
{
  __obliv_c__ifThenElse (dest,src,dest,size,cond);
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

void __obliv_c__setLessOrEqualSigned (void* vdest
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

void __obliv_c__setEqualToF (void* vdest
                            ,const void* vop1,const void* vop2
                            ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_eq_circuit(dest, op1, op2);
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

void __obliv_c__setNotEqualF (void* vdest
                             ,const void* vop1,const void* vop2
                             ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_eq_circuit(dest, op1, op2);
  __obliv_c__flipBit(dest);
}

void __obliv_c__setLessThanF (void* vdest
                             ,const void* vop1,const void* vop2
                             ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_lt_circuit(dest, op1, op2);
}

void __obliv_c__setLessThanEqF (void* vdest
                               ,const void* vop1,const void* vop2
                               ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  obliv_float_le_circuit(dest, op1, op2);
}

void __obliv_c__setLogicalNot (void* vdest,const void* vop,size_t size)
{ OblivBit t;
  OblivBit *dest=vdest;
  const OblivBit *op=vop;
  __obliv_c__assignBitKnown(&t,0);
  while(size-->0) __obliv_c__setBitOr(&t,&t,op++);
  __obliv_c__setBitNot(dest,&t);
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
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivInt(OblivInputs* spec, __obliv_c__int* dest, int v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivShort(OblivInputs* spec, __obliv_c__short* dest, short v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivLong(OblivInputs* spec, __obliv_c__long* dest, long v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivLLong(OblivInputs* spec, __obliv_c__lLong* dest, long long v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivFloat(OblivInputs* spec, __obliv_c__float* dest, float v)
{ spec->dest=dest->bits;
  spec->src_f=v;
  spec->size=__bitsize(v);
}

void feedOblivInputs(OblivInputs* spec, size_t count, int party)
  { currentProto->feedOblivInputs(currentProto,spec,count,party); }

#define feedOblivFun(t,ot,tname) \
    __obliv_c__##ot feedObliv##tname (t v,int party) \
    { __obliv_c__##ot rv; \
      OblivInputs spec; \
      setupObliv##tname(&spec,&rv,v); \
      feedOblivInputs(&spec,1,party); \
      return rv; \
    }\
    void feedObliv##tname##Array(__obliv_c__##ot dest[],const t src[],size_t n,\
                             int party)\
    {\
      size_t i,p = protoCurrentParty(currentProto);\
      OblivInputs *specs = malloc(n*sizeof*specs);\
      for(i=0;i<n;++i) setupObliv##tname(specs+i,dest+i,p==party?src[i]:0);\
      feedOblivInputs(specs,n,party);\
      free(specs);\
    }

feedOblivFun(bool,bool,Bool)
feedOblivFun(char,char,Char)
feedOblivFun(short,short,Short)
feedOblivFun(int,int,Int)
feedOblivFun(long,long,Long)
feedOblivFun(long long,lLong,LLong)
feedOblivFun(float, float, Float)

#undef feedOblivFun

// TODO pass const values by ref later
bool revealOblivBool(bool * dest, __obliv_c__bool src, int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,1,party))
    { *dest=(bool)wd; return true; }
  return false;
}
bool revealOblivBoolArray(bool *dest, const __obliv_c__bool * src,
                              size_t n, int party)
{ bool rv = true;
  for (size_t ii = 0; ii < n; ii++)
    { rv &= revealOblivBool(&dest[ii], src[ii], party); }
  return rv;
}

#define revealOblivFun(t, ot, tname) \
      bool revealObliv##tname(t * dest, __obliv_c__##ot src, int party) \
      { widest_t wd; \
        if(__obliv_c__revealOblivBits(&wd,src.bits,__bitsize(t),party)) \
          { *dest=*(t*)&wd; return true; } \
        return false; \
      } \
      bool revealObliv##tname##Array(t *dest, const __obliv_c__##ot * src,\
                                    size_t n, int party) \
      { bool rv = true; \
        if(party!=1) \
          for (size_t ii = 0; ii < n; ii++) \
            { rv &= revealObliv##tname(&dest[ii], src[ii], 2); } \
        if(party!=2) \
          for (size_t ii = 0; ii < n; ii++) \
            { rv &= revealObliv##tname(&dest[ii], src[ii], 1); } \
        return rv; \
      }

revealOblivFun(char,char,Char);
revealOblivFun(short,short,Short);
revealOblivFun(int,int,Int);
revealOblivFun(long,long,Long);
revealOblivFun(long long,lLong,LLong);
revealOblivFun(float,float,Float);

#undef revealOblivFun

// TODO fix data width
bool ocBroadcastBool(bool v,int party)
{ char t = v;
  broadcastBits(party,&t,1);
  return t;
}
void ocBroadcastBoolArray(bool *dest, bool *src, size_t n, int party)
{ for (size_t ii = 0; ii < n; ii ++)
    { dest[ii] = ocBroadcastBool(src[ii], party); }
}

#define broadcastFun(t,tname)           \
      t ocBroadcast##tname(t v, int party)   \
      { broadcastBits(party,&v,sizeof(t)); \
        return v;                           \
      } \
      void ocBroadcast##tname##Array(t *dest, t *src, size_t n, int party) \
      { if (ocCurrentParty() == party && dest != src) \
          { memcpy(dest, src, n * sizeof(t)); } \
        broadcastBits(party,dest,n * sizeof(t)); \
      }

broadcastFun(char,Char)
broadcastFun(int,Int)
broadcastFun(short,Short)
broadcastFun(long,Long)
broadcastFun(long long,LLong)

#undef broadcastFun
