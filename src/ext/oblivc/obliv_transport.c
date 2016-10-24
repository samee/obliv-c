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

static void stdioCleanup(ProtocolTransport* pt) {}

// Extremely simple, no multiplexing: two parties, one connection
static struct stdioTransport stdioTransport 
  = {{2, NULL, stdioSend, stdioRecv, stdioCleanup},false};

void protocolUseStdio(ProtocolDesc* pd)
  { pd->trans = &stdioTransport.cb; }

//#define PROFILE_NETWORK
// TCP connections for 2-Party protocols. Ignores src/dest parameters
//   since there is only one remote
typedef struct tcp2PTransport
{ ProtocolTransport cb;
  int sock;
  bool isClient;
  FILE* sockStream;
  bool needFlush;
  int sinceFlush;
#ifdef PROFILE_NETWORK
  size_t bytes;
  int flushCount;
  struct tcp2PTransport* parent;
#endif
} tcp2PTransport;

size_t tcp2PBytesSent(ProtocolDesc* pd) 
#ifdef PROFILE_NETWORK
  { return ((tcp2PTransport*)(pd->trans))->bytes; }
#else
  { return 0; }
#endif

int tcp2PFlushCount(ProtocolDesc* pd)
#ifdef PROFILE_NETWORK
  { return ((tcp2PTransport*)(pd->trans))->flushCount; }
#else
  { return 0; }
#endif

static int tcp2PSend(ProtocolTransport* pt,int dest,const void* s,size_t n)
{ struct tcp2PTransport* tcpt = CAST(pt);
  size_t n2=0;
  tcpt->needFlush=true;
  /*while(n>n2) {*/
	/*int res = write(tcpt->sock,n2+(char*)s,n-n2);*/
	/*if(res<=0) { perror("TCP write error: "); return res; }*/
	/*n2+=res;*/
/*#ifdef PROFILE_NETWORK*/
	/*tcpt->bytes += res;*/
/*#endif*/
  /*}*/
  while(n>n2) {
	int res = fwrite(n2+(char*)s,1,n-n2,tcpt->sockStream);
	if(res<0) { perror("TCP write error: "); return res; }
	n2+=res;
#ifdef PROFILE_NETWORK
	tcpt->bytes += res;
#endif
  }
  return n2;
}

static int tcp2PRecv(ProtocolTransport* pt,int src,void* s,size_t n)
{ int res=0,n2=0;
	struct tcp2PTransport* tcpt = CAST(pt);
	if(tcpt->needFlush)
	{
		fflush(tcpt->sockStream);	
#ifdef PROFILE_NETWORK
                tcpt->flushCount++;
#endif
		tcpt->needFlush=false;
	}
  /*while(n>n2)*/
  /*{ res = read(((tcp2PTransport*)pt)->sock,n2+(char*)s,n-n2);*/
	/*if(res<=0) { perror("TCP read error: "); return res; }*/
	/*n2+=res;*/
  /*}*/
  while(n>n2)
  { res = fread(n2+(char*)s,1,n-n2, tcpt->sockStream);
	if(res<0 || feof(tcpt->sockStream)) { perror("TCP read error: "); return res; }
	n2+=res;
  }
  return res;
}

static void tcp2PCleanup(ProtocolTransport* pt)
{ 
  tcp2PTransport* t = CAST(pt);
  fflush(t->sockStream);
  fclose(t->sockStream);
#ifdef PROFILE_NETWORK
  t->flushCount++;
  if(t->parent==NULL)
  { fprintf(stderr,"Total bytes sent: %zd\n",t->bytes);
    fprintf(stderr,"Total flush done: %d\n",t->flushCount);
  }
  else
  { t->parent->bytes+=t->bytes;
    t->parent->flushCount+=t->flushCount;
  }
#endif
  free(pt);
}

static inline bool transIsTcp2P(ProtocolTransport* pt)
  { return pt->cleanup == tcp2PCleanup; }
FILE* transGetFile(ProtocolTransport* t)
{
  if(transIsTcp2P(t)) return ((tcp2PTransport*)t)->sockStream;
  else return NULL;
}
static ProtocolTransport* tcp2PSplit(ProtocolTransport* tsrc);

#ifdef PROFILE_NETWORK
static const tcp2PTransport tcp2PTransportTemplate
  = {{.maxParties=2, .split=tcp2PSplit, .send=tcp2PSend, .recv=tcp2PRecv,
      .cleanup = tcp2PCleanup},
     .sock=0, .isClient=0, .needFlush=false, .bytes=0, .flushCount=0,
     .parent=NULL};
#else
static const tcp2PTransport tcp2PTransportTemplate
  = {{.maxParties=2, .split=tcp2PSplit, .send=tcp2PSend, .recv=tcp2PRecv,
      .cleanup = tcp2PCleanup},
    .sock=0, .isClient=0, .needFlush=false};
#endif

// isClient value will only be used for the split() method, otherwise
// its value doesn't matter. In that case, it indicates which party should be
// the server vs. client for the new connections (which is usually the same as
// the old roles).
static tcp2PTransport* tcp2PNew(int sock,bool isClient)
{ tcp2PTransport* trans = malloc(sizeof(*trans));
  *trans = tcp2PTransportTemplate;
  trans->sock = sock;
  trans->isClient=isClient;
  trans->sockStream=fdopen(sock, "rb+");
  trans->sinceFlush = 0;
  const int one=1;
  setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&one,sizeof(one));
  /*setvbuf(trans->sockStream, trans->buffer, _IOFBF, BUFFER_SIZE);*/
  return trans;
}
void protocolUseTcp2P(ProtocolDesc* pd,int sock,bool isClient)
  { pd->trans = &tcp2PNew(sock,isClient)->cb; }

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
    fflush(((tcp2PTransport*)t)->sockStream); 
#ifdef PROFILE_NETWORK
    ((tcp2PTransport*)t)->flushCount++;
#endif
    int newsock = accept(listenSock,0,0);
    close(listenSock);
    return newsock;
  }
}

static ProtocolTransport* tcp2PSplit(ProtocolTransport* tsrc)
{
  tcp2PTransport* t = CAST(tsrc);
  fflush(t->sockStream); 
#ifdef PROFILE_NETWORK
  ((tcp2PTransport*)t)->flushCount++;
#endif
  // I should really rewrite sockSplit to use FILE* sockStream
  int newsock = sockSplit(t->sock,tsrc,t->isClient);
  if(newsock<0) { fprintf(stderr,"sockSplit() failed\n"); return NULL; }
#ifdef PROFILE_NETWORK
  if(!t->isClient) t->bytes+=sizeof(in_port_t);
#endif
  tcp2PTransport* tnew = tcp2PNew(newsock,t->isClient);
#ifdef PROFILE_NETWORK
  tnew->parent=t;
#endif
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
// ---------------------------------------------------------------------------
