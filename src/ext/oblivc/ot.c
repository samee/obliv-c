#include<assert.h>
#include<gcrypt.h>
#include<inttypes.h>
#include<pthread.h>
#include<stdbool.h>
#include<stdint.h>
#include<stdlib.h>
#include<math.h>

#include<time.h>
#include<stdio.h>

#include<bcrandom.h>
#include<obliv_common.h>
#include<obliv_types.h>
#include<commitReveal.h>
#define HERE printf("%s:%d\n",__FILE__,__LINE__)

inline void printhex(const char* msg,const char* buf,size_t n)
{
  int i;
  printf("%s: ",msg);
  for(i=0;i<n;++i) printf("%.02x ",buf[i]&0xff);
  printf("\n");
}
// ---------------- Diffie Hellman Random Elt Generator ----------------------

static gcry_mpi_t DHModQ,DHModQMinus3; // minus 3?! This is just paranoia
static gcry_mpi_point_t DHg;           // The group generator of order q

void setctrFromIntBCipherRandomGen(BCipherRandomGen* gen,uint64_t ctr)
{
  const int isz = sizeof(ctr);
  memcpy(gen->ctr,&ctr,isz);
  memcpy(gen->ctr+isz,gen->zeroes,gen->blen-isz);
  gcry_cipher_setctr(gen->cipher,gen->ctr,gen->blen);
}

// allocates and returns a new DH element in range [2,p-2]
gcry_mpi_t dhRandomExp(BCipherRandomGen* gen)
{
  char out[1+(DHEltBits+7)/8];
  const int outsize = sizeof(out)/sizeof(*out);
  int islarge;
  gcry_mpi_t x;
  out[0]=0;  // needed for unsigned numbers in GCRYMPI_FMT_STD
  do
  { randomizeBuffer(gen,out+1,outsize-1);
    gcry_mpi_scan(&x,GCRYMPI_FMT_STD,out,outsize,NULL);
    islarge = gcry_mpi_cmp(x,DHModQMinus3);
  } while(islarge>=0);
  gcry_mpi_add_ui(x,x,2);
  return x;
}

static pthread_once_t dhRandomInitDone = PTHREAD_ONCE_INIT;
// Needs to be invoked before any other functions here
static void dhRandomInitAux(void)
{
  gcry_ctx_t DHCurve;
  gcryDefaultLibInit();
  gcry_mpi_ec_new(&DHCurve,NULL,DHCurveName);
  DHg = gcry_mpi_ec_get_point("g",DHCurve,1);
  DHModQ = gcry_mpi_ec_get_mpi("n",DHCurve,1);
  DHModQMinus3 = gcry_mpi_new(0);
  gcry_mpi_sub_ui(DHModQMinus3,DHModQ,3);
  gcry_ctx_release(DHCurve);
}

void dhRandomInit(void) { pthread_once(&dhRandomInitDone,dhRandomInitAux); }

// It would be nice if somebody called this at the end of main, but it
//   simply frees memory before exit.
void dhRandomFinalize(void)
{
  // if(!dhRandomInitDone) return;
  gcry_mpi_release(DHModQ);
  gcry_mpi_release(DHModQMinus3);
  gcry_mpi_point_release(DHg);
}

// x and y are just scratch memory that is used internally.
// u gets serialized into buf
void dhSerialize(char* buf,gcry_mpi_point_t u,
    gcry_ctx_t ctx,gcry_mpi_t x,gcry_mpi_t y) // TODO manage these plumbings
{
  size_t s;
  unsigned char *ubuf = CAST(buf);
  const int elts = DHEltSerialBytes/2;
  gcry_mpi_ec_get_affine(x,y,u,ctx); // kinda expensive
  gcry_mpi_print(GCRYMPI_FMT_PGP,ubuf,elts,&s,x);
  while(s<elts) ubuf[s++]=0;
  ubuf+=elts;
  gcry_mpi_print(GCRYMPI_FMT_PGP,ubuf,elts,&s,y);
  while(s<elts) ubuf[s++]=0;
}

void dhDeserialize(gcry_mpi_point_t* p, const char* buf)
{
  const int elts = DHEltSerialBytes/2;
  gcry_mpi_t x,y;
  gcry_mpi_scan(&x,GCRYMPI_FMT_PGP,buf,elts,NULL);
  gcry_mpi_scan(&y,GCRYMPI_FMT_PGP,buf+elts,elts,NULL);
  *p = gcry_mpi_point_snatch_set(NULL,x,y,gcry_mpi_set_ui(NULL,1));
  // TODO maybe I should check if *p is on the curve?
}

// These names started out as static. Now that they are not, the names should
// be properly prefixed to avoid name clashes TODO
// Once again, x and y are scratch variables
void dhSend(gcry_mpi_point_t u,ProtocolDesc* pd,int party,
                   gcry_ctx_t ctx,gcry_mpi_t x, gcry_mpi_t y)
{
  char buf[DHEltSerialBytes];
  dhSerialize(buf,u,ctx,x,y);
  osend(pd,party,buf,DHEltSerialBytes);
}
// Allocates a new gcry_mpi_t, and returns it
gcry_mpi_point_t dhRecv(ProtocolDesc* pd,int party)
{ char buf[DHEltSerialBytes];
  gcry_mpi_point_t x;
  orecv(pd,party,buf,DHEltSerialBytes);
  dhDeserialize(&x,buf);
  return x;
}

/*
void dhDebug(gcry_mpi_t x)
{
  unsigned char buff[520];
  gcry_mpi_print(GCRYMPI_FMT_HEX,buff,520,NULL,x);
  fprintf(stderr,"%s\n",buff);
}
void dhDebugPoint(gcry_mpi_point_t g,gcry_ctx_t ctx)
{
  gcry_mpi_t gx,gy;
  gx = gcry_mpi_new(0);
  gy = gcry_mpi_new(0);
  gcry_mpi_ec_get_affine(gx,gy,g,ctx);
  fprintf(stderr,"  x = "); dhDebug(gx);
  fprintf(stderr,"  y = "); dhDebug(gy);
  gcry_mpi_release(gx);
  gcry_mpi_release(gy);
}
*/

static void xorBuffer(char* dest,const char* x,const char* y,size_t len)
{ int i;
  for(i=0;i<len;++i) dest[i]=x[i]^y[i];
}

#define HASH_BYTES 32
// adds an extra counter R
//   For 2-party, this could be slightly faster by shoving i inside R
static void oneTimePad(char* dest,const char* src,size_t n,gcry_mpi_point_t k,
    uint64_t R,int i,gcry_ctx_t ctx,gcry_mpi_t x,gcry_mpi_t y)
{
  char sb[DHEltSerialBytes+sizeof(R)+sizeof(i)];
  int sz=0;
  char digest[HASH_BYTES];
  assert(n<=HASH_BYTES);
  memcpy(sb+sz,&R,sizeof(R)); sz+=sizeof(R); // careful,
  memcpy(sb+sz,&i,sizeof(i)); sz+=sizeof(i); //   endianness
  dhSerialize(sb+sz,k,ctx,x,y); sz+=DHEltSerialBytes;
  gcry_md_hash_buffer(GCRY_MD_SHA256, digest, sb, sz);
  xorBuffer(dest,src,digest,n);
}

typedef struct NpotSender
{ gcry_mpi_point_t gr,*Cr;
  gcry_mpi_t r;
  gcry_mpi_t scratchx,scratchy,scratchz;
  uint64_t R;
  int nmax;
  BCipherRandomGen* gen;
  ProtocolDesc* pd;
  int destParty;
  gcry_ctx_t ctx; // This doesn't seem to be thread-safe
} NpotSender;

/*
  Returns a new object for performing Naor-Pinkas Oblivious Transfers.
  Can later be used with npotSend*() family of functions. The basic object
  is really set up to perform 1-out-of-N transfers, although the npotSend*()
  functions often build on top of it to provide other kinds of OT behavior.

  Parameters:
    nmax      : The maximum N value that will be supported by npotSend()
    pd        : The ProtocolDesc object that is used to
                  perform internal send/recv
    destParty : The receiver for these oblivious transfers

  Caller responsibilities:
    npotRecverNew() needs to be called at the same time by destParty with
      the same nmax value.
    Object is later freed with npotRecverRelease()

  This initialization process also generates and sends out a public key of
  size O(nmax) bytes to the receiver. This is why a corresponding call to
  npotRecverNew() is needed on the other side.
*/
NpotSender* npotSenderNew(int nmax,ProtocolDesc* pd,int destParty)
{
  NpotSender *s;
  int i;
  s = malloc(sizeof(NpotSender));

  s->gen = newBCipherRandomGen();
  gcry_mpi_ec_new(&s->ctx,NULL,DHCurveName);
  s->r = dhRandomExp(s->gen);
  s->gr = gcry_mpi_point_new(0);
  s->scratchx = gcry_mpi_new(0);
  s->scratchy = gcry_mpi_new(0);
  s->scratchz = gcry_mpi_new(0);
  gcry_mpi_ec_mul(s->gr,s->r,DHg,s->ctx);
  s->nmax = nmax;
  s->R = 0;
  s->pd = pd; s->destParty = destParty;

  dhSend(s->gr,s->pd,s->destParty,s->ctx,s->scratchx,s->scratchy);
  s->Cr = malloc(sizeof(gcry_mpi_point_t)*(nmax-1));
  for(i=0;i<nmax-1;++i)
  {
    s->Cr[i] = gcry_mpi_point_new(0);
    gcry_mpi_release(s->scratchx);
    s->scratchx = dhRandomExp(s->gen);
    gcry_mpi_ec_mul(s->Cr[i],s->scratchx,DHg,s->ctx);
    dhSend(s->Cr[i],s->pd,s->destParty,s->ctx,s->scratchx,s->scratchy);
    gcry_mpi_ec_mul(s->Cr[i],s->r,s->Cr[i],s->ctx);
  }
  return s;
}

typedef struct { gcry_mpi_point_t PK0; } NpotSenderState;

static void npotSend_roundRecvKey(NpotSender* s,NpotSenderState* q)
  { q->PK0 = dhRecv(s->pd,s->destParty); }

// x,y,z are scratch
// TODO(samee): Use gcry_mpi_point_copy when the libgcrypt 1.8 graduates
// out of Debian experimental branch.
static void oblivc_mpi_point_copy(gcry_mpi_point_t w, gcry_mpi_point_t v,
      gcry_mpi_t x,gcry_mpi_t y,gcry_mpi_t z)
{
  gcry_mpi_point_get(x,y,z,v);
  gcry_mpi_point_set(w,x,y,z);
}
// x,y,z are scratch
// TODO(samee): We should start using gcry_mpi_ec_sub() directly.
//   But see https://github.com/samee/obliv-c/pull/39
//   We first need to add support for this in libgcrypt.
static void oblivc_mpi_point_neg(gcry_mpi_point_t w,gcry_mpi_point_t v,
    gcry_mpi_t x,gcry_mpi_t y,gcry_mpi_t z)
{
  gcry_mpi_point_get(x,y,z,v);
  gcry_mpi_neg(y,y);
  gcry_mpi_point_set(w,x,y,z);
}
static void npotSend_roundSendData(NpotSender* s,NpotSenderState* q,
    char** arr,int n,int len)
{
  int i;
  char buf[HASH_BYTES];
  gcry_mpi_point_t PK0,PKi;
  PK0 = q->PK0;

  PKi = gcry_mpi_point_new(0);
  gcry_mpi_ec_mul(PK0,s->r,PK0,s->ctx);
  oneTimePad(buf,arr[0],len,PK0,s->R,0,s->ctx,s->scratchx,s->scratchy);
  osend(s->pd,s->destParty,buf,len);
  oblivc_mpi_point_neg(PK0,PK0,s->scratchx,s->scratchy,s->scratchz);

  for(i=1;i<n;++i)
  { gcry_mpi_ec_add(PKi,PK0,s->Cr[i-1],s->ctx);
    oneTimePad(buf,arr[i],len,PKi,s->R,i,s->ctx,s->scratchx,s->scratchy);
    osend(s->pd,s->destParty,buf,len);
  }
  s->R++;
  gcry_mpi_point_release(PK0);
  gcry_mpi_point_release(PKi);
}

/*
  The simplest of npotSend*() family. Performs an 1-out-of-n oblivious
  transfer, where the receiver gets to choose one of the elements
  arr[0]..arr[n-1]. Each arr[i] is assumed to be exactly len bytes long, where
  len<=HASH_BYTES. Moreover, the same value of n and len must be used on the
  receiving side as it calls npotRecv to obtain the values. The maximum allowed
  value of n is the nmax value used when initializing the NpotSender* s.
  */
void npotSend(NpotSender* s,char** arr,int n,int len)
{
  NpotSenderState q;
  npotSend_roundRecvKey(s,&q);
  npotSend_roundSendData(s,&q,arr,n,len);
}

void npotSenderRelease(NpotSender* s)
{
  int i;
  if(!s) return;
  gcry_mpi_release(s->r);
  gcry_mpi_point_release(s->gr);
  for(i=0;i<s->nmax-1;++i) gcry_mpi_point_release(s->Cr[i]);
  free(s->Cr);
  gcry_mpi_release(s->scratchx);
  gcry_mpi_release(s->scratchy);
  gcry_mpi_release(s->scratchz);
  releaseBCipherRandomGen(s->gen);
  gcry_ctx_release(s->ctx);
  free(s);
}

typedef struct NpotRecver
{
  gcry_mpi_point_t gr,*C;
  gcry_mpi_t scratchx,scratchy,scratchz;
  int nmax;
  uint64_t R;
  BCipherRandomGen* gen;
  ProtocolDesc* pd;
  int srcParty;
  gcry_ctx_t ctx; // This doesn't seem to be thread-safe
} NpotRecver;

// nmax must match with that on the sender side
// See comment for npotSenderNew()
NpotRecver* npotRecverNew(int nmax,ProtocolDesc* pd,int srcParty)
{
  int i;
  NpotRecver* r;
  r = malloc(sizeof(NpotRecver));
  gcry_mpi_ec_new(&r->ctx,NULL,DHCurveName);
  r->nmax = nmax;
  r->R = 0;
  r->gr = dhRecv(pd,srcParty);
  r->scratchx = gcry_mpi_new(0);
  r->scratchy = gcry_mpi_new(0);
  r->scratchz = gcry_mpi_new(0);
  r->C = malloc(sizeof(gcry_mpi_point_t)*(nmax-1));
  r->pd = pd; r->srcParty = srcParty;
  for(i=0;i<nmax-1;++i) r->C[i]=dhRecv(pd,srcParty);
  r->gen = newBCipherRandomGen();
  return r;
}

typedef struct {
  gcry_mpi_t k;
  int seli,n;
} NpotRecverState;

static void npotRecv_roundSendKey(NpotRecver* r,NpotRecverState* q,int seli,
    int n)
{
  gcry_mpi_point_t gk,PK0,*p;
  int i;
  q->seli=seli;
  q->n=n;
  q->k = dhRandomExp(r->gen);
  gk = gcry_mpi_point_new(0);
  PK0 = gcry_mpi_point_new(0);
  gcry_mpi_ec_mul(gk,q->k,DHg,r->ctx);
  oblivc_mpi_point_copy(PK0,gk,r->scratchx,r->scratchy,r->scratchz);

  if(seli==0) { i=0; p=&gk; }
  else { i=seli-1; p=&PK0; }
  oblivc_mpi_point_neg(*p,gk,r->scratchx,r->scratchy,r->scratchz);
  gcry_mpi_ec_add(*p,*p,r->C[i],r->ctx);
  dhSend(PK0,r->pd,r->srcParty,r->ctx,r->scratchx,r->scratchy);

  gcry_mpi_point_release(gk);
  gcry_mpi_point_release(PK0);
}

static void npotRecv_roundRecvData(NpotRecver* r,NpotRecverState* q,char* dest,
    int len)
{
  int i;
  char selbuf[HASH_BYTES], dummybuf[HASH_BYTES];
  gcry_mpi_point_t gkr;
  gkr = gcry_mpi_point_new(0);
  gcry_mpi_ec_mul(gkr,q->k,r->gr,r->ctx);
  for(i=0;i<q->n;++i) orecv(r->pd,r->srcParty,i==q->seli?selbuf:dummybuf,len);
  oneTimePad(dest,selbuf,len,gkr,r->R,q->seli,r->ctx,r->scratchx,r->scratchy);
  r->R++;
  gcry_mpi_point_release(gkr);
  gcry_mpi_release(q->k);
}

// n and len must match those on the sender side. See comment for npotSend()
void npotRecv(NpotRecver* r,char* dest,int seli,int n,int len)
{
  NpotRecverState q;
  npotRecv_roundSendKey(r,&q,seli,n);
  npotRecv_roundRecvData(r,&q,dest,len);
}

void npotRecverRelease(NpotRecver* r)
{
  int i;
  if(!r) return;
  releaseBCipherRandomGen(r->gen);
  gcry_mpi_point_release(r->gr);
  for(i=0;i<r->nmax-1;++i) gcry_mpi_point_release(r->C[i]);
  free(r->C);
  gcry_mpi_release(r->scratchx);
  gcry_mpi_release(r->scratchy);
  gcry_mpi_release(r->scratchz);
  gcry_ctx_release(r->ctx);
  free(r);
}

// -------------------- Extending OT with base-k trick -----------------------

// Computes ceil(log_b(x))
static int logceil(int x,int b)
{ int res=0;
  x--;
  while(x>0) { x/=b; res++; }
  return res;
}

#define LOGMAX 60
// Express x in base b. Least significant digit ends up in dest[0]
// higher position digits are padded with 0 at the end of dest
// Returns false if dlen is too small, in which case dest has undefined contents
static bool inBaseN(int* dest, int dlen, int x, int b)
{
  int i=0;
  while(x>0)
  { if(i>=dlen) return false;
    dest[i]=x%b;
    x/=b;
    i++;
  }
  for(;i<dlen;++i) dest[i]=0;
  return true;
}

// Performs 1-out-of-n OT, even if n > s->nmax.
//   len is still small (<= HASH_BYTES)
// len must match up with npotRecvMany
void npotSendMany(NpotSender* s,char** arr,int n,int len)
{
  int base = s->nmax;
  if(n<=base) return npotSend(s,arr,n,len);
  char *keys,**starts, buf[HASH_BYTES];
  int lc = logceil(n,base);
  int r = len*base*lc,i,j;
  NpotSenderState state[LOGMAX];
  keys = malloc(r);
  starts = malloc(base*sizeof(char*));
  randomizeBuffer(s->gen,keys,r);
  for(i=0;i<lc;++i) npotSend_roundRecvKey(s,state+i);
  for(i=0;i<lc;++i)
  { for(j=0;j<base;++j) starts[j]=keys+(base*i+j)*len;
    npotSend_roundSendData(s,state+i,starts,base,len);
  }
  for(i=0;i<n;++i)
  { int ind[LOGMAX];
    assert(lc<=LOGMAX);
    inBaseN(ind,lc,i,base);
    memcpy(buf,arr[i],len);
    for(j=0;j<lc;++j) xorBuffer(buf,buf,keys+(j*base+ind[j])*len,len);
    osend(s->pd,s->destParty,buf,len);
  }
  free(starts);
  free(keys);
}

// Same warning as always, n and len must pair up with sender
void npotRecvMany(NpotRecver* r,char* dest,int seli,int n,int len)
{
  int base = r->nmax;
  if(n<=base) return npotRecv(r,dest,seli,n,len);
  char buf1[HASH_BYTES],buf2[HASH_BYTES];
  int lc = logceil(n,base),i;
  int ind[LOGMAX];
  NpotRecverState state[LOGMAX];
  assert(lc<=LOGMAX);
  inBaseN(ind,lc,seli,base);
  for(i=0;i<lc;++i) npotRecv_roundSendKey(r,state+i,ind[i],base);
  memset(buf2,0,len);
  for(i=0;i<lc;++i)
  { npotRecv_roundRecvData(r,state+i,buf1,len);
    xorBuffer(buf2,buf2,buf1,len);
  }
  for(i=0;i<n;++i) orecv(r->pd,r->srcParty,(i==seli?dest:buf1),len);
  xorBuffer(dest,dest,buf2,len);
}

// Performs 1-out-of-n OT using npotSendMany, but works even if len > HASH_BYTES
void npotSendLong(NpotSender* s,char** arr,int n,int len)
{
  char *keys,**kstarts;
  const int klen = BC_SEEDLEN_DEFAULT;
  char* buf;
  int i;
  BCipherRandomGen* gen;
  if(len<=HASH_BYTES) return npotSendMany(s,arr,n,len);
  keys = malloc(n*klen);
  kstarts = malloc(n*sizeof(keys));
  buf = malloc(len);
  randomizeBuffer(s->gen,keys,n*klen);

  for(i=0;i<n;++i) kstarts[i]=keys+i*klen;
  npotSendMany(s,kstarts,n,klen);

  gen = newBCipherRandomGen();
  for(i=0;i<n;++i)
  {
    resetBCipherRandomGen(gen,kstarts[i]);
    randomizeBuffer(gen,buf,len);
    xorBuffer(buf,buf,arr[i],len);
    osend(s->pd,s->destParty,buf,len);
  }
  releaseBCipherRandomGen(gen);

  free(buf);
  free(kstarts);
  free(keys);
}

void npotRecvLong(NpotRecver* r,char* dest,int seli,int n,int len)
{
  BCipherRandomGen* gen;
  char *dummy,key[BC_SEEDLEN_DEFAULT];
  int i;
  if(len<=HASH_BYTES) return npotRecvMany(r,dest,seli,n,len);
  dummy = malloc(len);

  npotRecvMany(r,key,seli,n,BC_SEEDLEN_DEFAULT);
  for(i=0;i<n;++i) orecv(r->pd,r->srcParty,i==seli?dest:dummy,len);

  gen = newBCipherRandomGen();
  resetBCipherRandomGen(gen,key);
  randomizeBuffer(gen,dummy,len);
  xorBuffer(dest,dest,dummy,len);

  releaseBCipherRandomGen(gen);
  free(dummy);
}

// Finally, I am ditching the double-pointer pattern
void npotSend1Of2Once(NpotSender* s,const char* opt0,const char* opt1,
    int n,int len)
{
  int i,j,c;
  char *buf,**starts;
  assert(n<8*sizeof(int));
  c = (1<<n);
  buf = malloc(c*n*len);
  starts = malloc(sizeof(char*)*c);
  for(i=0;i<c;++i)
  { for(j=0;j<n;++j)
      if (i&(1<<j)) memcpy(buf+(i*n+j)*len,opt1+j*len,len);
      else          memcpy(buf+(i*n+j)*len,opt0+j*len,len);
    starts[i] = buf+i*n*len;
  }
  npotSendLong(s,starts,c,len*n);
  free(buf);
  free(starts);
}

void npotRecv1Of2Once(NpotRecver* r,char* dest,unsigned mask,int n,int len)
{
  assert(n<8*sizeof(int));
  npotRecvLong(r,dest,mask,(1<<n),len*n);
}

void npotSend1Of2(NpotSender* s,const char* opt0,const char* opt1,int n,int len,
    int batchsize)
{
  int i;
  for (i=0;i+batchsize<=n;i+=batchsize)
    npotSend1Of2Once(s,opt0+i*len,opt1+i*len,batchsize,len);
  if(i<n) npotSend1Of2Once(s,opt0+i*len,opt1+i*len,n-i,len);
}

void npotRecv1Of2(NpotRecver* r,char* dest,const bool* sel,int n,int len,
    int batchsize)
{
  int i,j;
  unsigned mask;
  for (i=0;i+batchsize<=n;i+=batchsize)
  { for(j=mask=0;j<batchsize;++j) mask|=((sel[i+j]!=0)<<j);
    npotRecv1Of2Once(r,dest+i*len,mask,batchsize,len);
  }
  if(i<n)
  { for(j=mask=0;i+j<n;++j) mask|=((sel[i+j]==1)<<j);
    npotRecv1Of2Once(r,dest+i*len,mask,n-i,len);
  }
}

void npotAbstractSend(void* sender,const char* opt0,const char* opt1,
                      int n,int len)
  { npotSend1Of2(sender,opt0,opt1,n,len,NPOT_BATCH_SIZE); }

OTsender npotSenderAbstract(NpotSender* s)
{ return (OTsender) {.sender=(void*)s, .send=npotAbstractSend,
                     .release=(void (*)(void*))npotSenderRelease };
}

void npotAbstractRecv(void* recver,char* dest,const bool* sel,int n,int len)
  { npotRecv1Of2(recver,dest,sel,n,len,NPOT_BATCH_SIZE); }

OTrecver npotRecverAbstract(NpotRecver* r)
{ return (OTrecver) {.recver=(void*)r, .recv=npotAbstractRecv,
                     .release=(void (*)(void*))npotRecverRelease };
}

// --------------- OT-extension ---------------------------------------------

#define BATCH_SIZE 5
#define OT_SEEDLEN BC_SEEDLEN_DEFAULT

static void
unpackBytes(bool* dest, const char* src,int bits)
{
  int i,j;
  for(i=0;i<(bits+7)/8;++i)
  { char ch=src[i];
    for(j=0;j<8 && 8*i+j<bits;++j,ch>>=1)
      dest[8*i+j]=(ch&1);
  }
}
static void
packBytes(char* dest,const bool* src,int bits)
{
  int i,j,bytes=(bits+7)/8;
  for(i=0;i<bytes;++i)
  { char ch=0;
    for(j=7;j>=0;--j) if(8*i+j<bits) ch = ((ch<<1)|src[8*i+j]);
    dest[i]=ch;
  }
}
/*
  ExtensionBox:

  Sender and receiver gets random matrixes. Row size can be extended later,
  column size fixed to keyBytes. Property:
  senderMatrix[i][j] ^ recverMatrix[i][j] = R[i] && S[j]
  where [i][j] refer to bit (not byte) at ith row and jth column
  R is determined later as row size gets extended
  S is sender's random key fixed at initialization.

  No verification is done unless explicitly requested.
*/
typedef struct
{
  ProtocolDesc* pd;
  int destParty, keyBytes;
  BCipherRandomGen **keyblock;
  bool *S;
  char *spack; // same as S, in packed bytes;
} SenderExtensionBox;

SenderExtensionBox*
senderExtensionBoxNew (ProtocolDesc* pd, int destParty, int keyBytes)
{
  const int k = keyBytes*8;
  SenderExtensionBox* s = malloc(sizeof *s);
  s->pd=pd; s->destParty=destParty; s->keyBytes = k/8;
  s->spack = malloc(sizeof(char[k/8]));
  gcry_randomize(s->spack,k/8,GCRY_STRONG_RANDOM);
  s->S = malloc(sizeof(bool[k]));
  unpackBytes(s->S,s->spack,k);
  s->keyblock = malloc(sizeof(BCipherRandomGen*[k]));

  // Perform base OTs, initialize s->keyblock
  char seed[k*OT_SEEDLEN];
  int i;
  NpotRecver* ot = npotRecverNew(1<<BATCH_SIZE,pd,destParty);
  npotRecv1Of2(ot,seed,s->S,k,OT_SEEDLEN,BATCH_SIZE);
  npotRecverRelease(ot);
  for(i=0;i<k;++i)
    s->keyblock[i] = newBCipherRandomGenByKey(seed+i*OT_SEEDLEN);

  return s;
}
void
senderExtensionBoxRelease (SenderExtensionBox* s)
{
  const int k = s->keyBytes*8;
  int i;
  for(i=0;i<k;++i) releaseBCipherRandomGen(s->keyblock[i]);
  free(s->keyblock);
  free(s->S);
  free(s->spack);
  free(s);
}

typedef struct
{
  ProtocolDesc* pd;
  int srcParty, keyBytes;
  BCipherRandomGen **keyblock0, **keyblock1;
} RecverExtensionBox;

RecverExtensionBox*
recverExtensionBoxNew (ProtocolDesc* pd, int srcParty, int keyBytes)
{
  const int k = keyBytes*8;
  RecverExtensionBox* r = malloc(sizeof *r);
  r->pd=pd; r->srcParty=srcParty; r->keyBytes=keyBytes;
  char seed0[k*OT_SEEDLEN], seed1[k*OT_SEEDLEN];
  gcry_randomize(seed0,sizeof(seed0),GCRY_STRONG_RANDOM);
  gcry_randomize(seed1,sizeof(seed1),GCRY_STRONG_RANDOM);
  int i;
  r->keyblock0 = malloc(sizeof(BCipherRandomGen*[k]));
  r->keyblock1 = malloc(sizeof(BCipherRandomGen*[k]));
  NpotSender* ot = npotSenderNew(1<<BATCH_SIZE,pd,srcParty);
  npotSend1Of2(ot,seed0,seed1,k,OT_SEEDLEN,BATCH_SIZE);
  npotSenderRelease(ot);
  for(i=0;i<k;++i)
  { r->keyblock0[i] = newBCipherRandomGenByKey(seed0+i*OT_SEEDLEN);
    r->keyblock1[i] = newBCipherRandomGenByKey(seed1+i*OT_SEEDLEN);
  }
  return r;
}
void
recverExtensionBoxRelease (RecverExtensionBox* r)
{
  int i;
  const int k = r->keyBytes*8;
  for(i=0;i<k;++i)
  { releaseBCipherRandomGen(r->keyblock0[i]);
    releaseBCipherRandomGen(r->keyblock1[i]);
  }
  free(r->keyblock0); free(r->keyblock1);
  free(r);
}

/* Extension box, transposed: rows are obtained directly from base OTs
   box[] should be a char array of size s->keyBytes*rowBytes
   Output row r is found in index box[r*rowBytes .. (r+1)*rowBytes)
   Absolutely no validation is done: there is no guarantee that the receiver
     used a consistent mask over all rows (where s->S[r] is true)
   mask[] (in recver) should be of length rowBytes
*/
void
senderExtensionBox(SenderExtensionBox* s,char box[],size_t rowBytes)
{
  const int k = s->keyBytes*8;
  int i;
  char *keymine = malloc(k*rowBytes);
  for(i=0;i<k;++i)
    randomizeBuffer(s->keyblock[i],keymine+i*rowBytes,rowBytes);
  orecv(s->pd,s->destParty,box,k*rowBytes);
  for(i=0;i<k;++i)
  { char *keybox = box+i*rowBytes;
    if(s->S[i]) memxor(keybox,keymine+i*rowBytes,rowBytes);
    else        memcpy(keybox,keymine+i*rowBytes,rowBytes);
  }
  free(keymine);
}
void
recverExtensionBox(RecverExtensionBox* r,char box[],
                   const char mask[],size_t rowBytes)
{
  const int k = r->keyBytes*8;
  int i;
  char *keyxor = malloc(k*rowBytes);
  for(i=0;i<k;++i)
  { char *key0 = box+i*rowBytes, *key1 = keyxor+i*rowBytes;
    randomizeBuffer(r->keyblock0[i],key0,rowBytes);
    randomizeBuffer(r->keyblock1[i],key1,rowBytes);
  }
  for(i=0;i<k;++i)
  { char *key0 = box+i*rowBytes, *key1 = keyxor+i*rowBytes;
    memxor(key1,key0,rowBytes);
    memxor(key1,mask,rowBytes);
  }
  osend(r->pd,r->srcParty,keyxor,k*rowBytes);
  free(keyxor);
}

#define OT_THREAD_THRESHOLD 500
#define OT_THREAD_COUNT 8

#define CHECK_HASH_BYTES 10
#define CHECK_HASH_BITS (8*CHECK_HASH_BYTES)
#define CHECK_HASH_BITS_LOGCEIL 7
#define SECURITY_CONSTANT (2*CHECK_HASH_BITS+CHECK_HASH_BITS_LOGCEIL)

static bool transCanThread(ProtocolTransport* trans)
  { return trans->split!=NULL; }
static bool protoCanThread(ProtocolDesc* pd)
  { return transCanThread(pd->trans); }

/*
  Inputs:
  mat is char[rows][cols/8]
  src is char[cols/8]
  Output:
  dest is char[rows/8]
   */
void
bitmatMul(char* dest,const char* mat,const char* src,int rows,int cols)
{
  assert(cols%8==0 && rows%8==0);
  int r,c;
  for(r=0;r<rows;++r)
  {
    uint64_t ch=0;
    for(c=0;c<cols/(8*sizeof(uint64_t));++c)
      ch ^= (((const uint64_t*)src)[c]&((const uint64_t*)(mat+r*(cols/8)))[c]);
    c*=sizeof(uint64_t);
    for(;c<cols/8;++c) ch ^= (src[c]&mat[r*(cols/8)+c]);
    while(ch>1) ch = ((ch&1)^(ch>>1));
    setBit(dest,r,ch);
  }
}
typedef struct
{
  char (*dest)[CHECK_HASH_BYTES];
  const char *hashmat,*src;
  int rowBytes,from,to;
} BitMatMulThread;
void* bitmatMul_thread(void* args)
{
  BitMatMulThread* a=args;
  int i;
  for(i=a->from;i<a->to;i++)
    bitmatMul(a->dest[i],a->hashmat,a->src+i*a->rowBytes,
              8*CHECK_HASH_BYTES,8*a->rowBytes);
  return NULL;
}
/*
   Validates honesty of the receiver during an invocation of
   senderExtensionBox. The box[] and rowBytes are the same as in that
   function. There is a probability of 2^(-c) that c bits of our secret s->S
   would be leaked to the receiver. This is why, if you plan to use this
   function, it is recommended that s->keyBytes be set to twice the level of
   security you actually need (e.g. s->keyBytes=160 if 80-bits of security is
   desired). s->keyBytes = 2k ensures that the receiver either has to be lucky
   enough to win a gamble with probability 2^-k, or has to do 2^k iterations of
   hashing.
   Validation is done using an xor-homomorphic hash. This requires rowBytes
   to be larger than necessary by SECURITY_CONSTANT bits, where the extra
   mask[] bits in recverExtensionBox() should have been random (if the
   extra bits are not random, the sender might be able to deduce a few bits of
   the "usable" part of mask).
*/
bool
senderExtensionBoxValidate_hhash(SenderExtensionBox* s,BCipherRandomGen* gen,
                                 const char box[],size_t rowBytes)
{
  const int k = s->keyBytes*8, hlen = CHECK_HASH_BYTES;
  char *hashmat = malloc(rowBytes*8*hlen);
  char hashcur[k][hlen],hash0[hlen],hashxor[hlen];
  bool xorseen = false, res = true;

  if(!ocRandomBytes(s->pd,gen,hashmat,rowBytes*8*hlen,s->destParty))
    return false;
  int i,done=0,tc;
  BitMatMulThread args[OT_THREAD_COUNT];
  pthread_t hasht[OT_THREAD_COUNT];
  if(!protoCanThread(s->pd) || 8*rowBytes<=OT_THREAD_THRESHOLD) tc=1;
  else tc=OT_THREAD_COUNT;
  for(i=0;i<tc;++i)
  {
    int c = (k-done)/(tc-i);
    args[i] = (BitMatMulThread){.dest=hashcur,.hashmat=hashmat,.src=box,
                                .rowBytes=rowBytes,.from=done,.to=done+c};
    if(i==tc-1) bitmatMul_thread(&args[i]); // no thread if tc==1
    else pthread_create(&hasht[i],NULL,bitmatMul_thread,&args[i]);
    done+=c;
  }
  for(i=0;i<tc-1;++i) pthread_join(hasht[i],NULL);
  for(i=0;i<k;++i)
  { //bitmatMul(hashcur,hashmat,box+i*rowBytes,8*hlen,8*rowBytes);
    orecv(s->pd,s->destParty,hash0,hlen);
    if(s->S[i])
    { memxor(hashcur[i],hash0,hlen);
      if(xorseen && memcmp(hashxor,hashcur[i],hlen)) res = false;
      else { memcpy(hashxor,hashcur[i],hlen); xorseen = true; }
    }else if(memcmp(hash0,hashcur[i],hlen)) res = false;
  }
  free(hashmat);
  return res;
}
/* May return false if the sender is not co-operating. E.g. if the sender
   was trying to select a special validation hash matrix that would have
   revealed receiver's choice mask */
bool
recverExtensionBoxValidate_hhash(RecverExtensionBox* r,BCipherRandomGen* gen,
                                 const char box[],size_t rowBytes)
{
  const int k = r->keyBytes*8, hlen = CHECK_HASH_BYTES;
  char *hashmat = malloc(rowBytes*8*hlen);
  char hashcur[k][hlen];
  if(!ocRandomBytes(r->pd,gen,hashmat,rowBytes*8*hlen,r->srcParty))
    return false;
  int i,done=0,tc;
  BitMatMulThread args[OT_THREAD_COUNT];
  pthread_t hasht[OT_THREAD_COUNT];
  if(!protoCanThread(r->pd) || 8*rowBytes<=OT_THREAD_THRESHOLD) tc=1;
  else tc=OT_THREAD_COUNT;
  for(i=0;i<tc;++i)
  {
    int c = (k-done)/(tc-i);
    args[i] = (BitMatMulThread){.dest=hashcur,.hashmat=hashmat,.src=box,
                                .rowBytes=rowBytes,.from=done,.to=done+c};
    if(i==tc-1) bitmatMul_thread(&args[i]); // no thread if tc==1
    else pthread_create(&hasht[i],NULL,bitmatMul_thread,&args[i]);
    done+=c;
  }
  for(i=0;i<tc-1;++i) pthread_join(hasht[i],NULL);
  for(i=0;i<k;++i)
  { //bitmatMul(hashcur,hashmat,box+i*rowBytes,8*hlen,8*rowBytes);
    osend(r->pd,r->srcParty,hashcur[i],hlen);
  }
  free(hashmat);
  return true;
}

#define PHAIR_ALGO GCRY_MD_SHA1
#define PHAIR_HASHLEN (160/8)
/*
   Validates honesty of the receiver during an invocation of
   senderExtensionBox. The box[] and rowBytes are the same as in that
   function. There is a probability of 2^(-k(1-log(1+1/c))/2) that the receiver
   obtained enough information to compute s->S with 2^(k/2c) iterations, where
   the receiver can choose c in an adversarial manner. This is why, if you use
   this function, it is recommended that s->keyBytes be set to 4 times the
   level of security you actually need (e.g. s->keyBytes=320 if 80 bits of
   security is desired). s->keyBytes = 4k ensures that the receiver either
   has to be lucky enough to win a gamble with probability 2^-k, or has to do
   2^k iterations of hashing.

   On success, it writes k/2 = r->keyBytes*4 integers in the array, indicating
   the set of rows that should be used (using all rows in box will not add
   any security). Moreover, once this function is used, the same ExtensionBox
   should no longer be re-extended --- that can lead to too much information
   leak (half the rows are revealed each time).
*/
bool
senderExtensionBoxValidate_byPair(SenderExtensionBox* s,BCipherRandomGen* gen,
                                  int rowsRemaining[],
                                  const char box[], int rowBytes,bool dohash)
{
  const int k = s->keyBytes*8;
  unsigned perm[k],i;
  bool sx, res=true;
  char *rowxme = malloc(rowBytes), *rowxyou = (dohash?NULL:malloc(rowBytes));
  char phair_hash[PHAIR_HASHLEN], phair_hash_you[PHAIR_HASHLEN];
  bcRandomPermutation(gen,perm,k);
  osend(s->pd,s->destParty,perm,sizeof(int[k]));
  for(i=0;i<k;i+=2)
  { int a = perm[i], b = perm[i+1];
    rowsRemaining[i/2]=a;
    sx = (s->S[a]!=s->S[b]);
    osend(s->pd,s->destParty,&sx,sizeof(bool));
    memcpy(rowxme,box+a*rowBytes,rowBytes);
    memxor(rowxme,box+b*rowBytes,rowBytes);
    if(dohash)
    { gcry_md_hash_buffer(PHAIR_ALGO,phair_hash,rowxme,rowBytes);
      orecv(s->pd,s->destParty,phair_hash_you,PHAIR_HASHLEN);
      if(memcmp(phair_hash,phair_hash_you,PHAIR_HASHLEN)) res=false;
    }
    else
    { orecv(s->pd,s->destParty,rowxyou,rowBytes);
      if(memcmp(rowxme,rowxyou,rowBytes)) res=false;
    }
  }
  free(rowxme);
  free(rowxyou);
  return res;
}
// Returns false if sender is not cooperating (e.g. not providing a
// valid permutation). mask[] must be the same one used to construct box[].
// On success, it writes k/2 = r->keyBytes*4 integers in the array, indicating
// the set of rows that should be used (using all rows in box will not add
// any security)
bool
recverExtensionBoxValidate_byPair(RecverExtensionBox* r,BCipherRandomGen* gen,
                                  int rowsRemaining[],
                                  const char box[],const char mask[],
                                  int rowBytes,bool dohash)
{
  const int k = r->keyBytes*8;
  unsigned perm[k],i;
  bool sx, inperm[k];
  char *rowx = malloc(rowBytes);
  char phair_hash[PHAIR_HASHLEN];
  orecv(r->pd,r->srcParty,perm,sizeof(int[k]));
  memset(inperm,0,sizeof(inperm));
  for(i=0;i<k;++i)
  { if(i<0||i>=k) return false;
    else if(inperm[perm[i]]) return false;
    else inperm[perm[i]]=true;
  }
  for(i=0;i<k;i+=2)
  { int a = perm[i], b = perm[i+1];
    rowsRemaining[i/2]=a;
    memcpy(rowx,box+a*rowBytes,rowBytes);
    memxor(rowx,box+b*rowBytes,rowBytes);
    orecv(r->pd,r->srcParty,&sx,sizeof(bool));
    if(sx) memxor(rowx,mask,rowBytes);
    if(dohash)
    { gcry_md_hash_buffer(PHAIR_ALGO,phair_hash,rowx,rowBytes);
      osend(r->pd,r->srcParty,phair_hash,PHAIR_HASHLEN);
    }
    else osend(r->pd,r->srcParty,rowx,rowBytes);
  }
  free(rowx);
  return true;
}
// Same function for encypt and decrypt. One-time pad, so don't reuse keys
// Overlapping buffers not supported
#if 0
static void
bcipherCrypt(BCipherRandomGen* gen,const char* key,int klen,int nonce,
                  char* dest,const char* src,int n)
{
  int i;
  char keyx[gen->klen];
  assert(klen<=gen->klen);
  memcpy(keyx,key,klen); memset(keyx+klen,0,gen->klen-klen);
  resetBCipherRandomGen(gen,keyx);
  setctrFromIntBCipherRandomGen(gen,nonce);
  randomizeBuffer(gen,dest,n);
  for(i=0;i<n;++i) dest[i]^=src[i];
}
#endif

static void
bcipherCryptNoResize(BCipherRandomGen* gen,const char* key,int nonce,
                     char* dest,const char* src,int n)
{
  resetBCipherRandomGen(gen,key);
  setctrFromIntBCipherRandomGen(gen,nonce);
  randomizeBuffer(gen,dest,n);
  memxor(dest,src,n);
}
/*
   Actually use our extension box (possibly after validation, depending on
   how much we trust our receiver). Sends out encryptions of msg0 and msg1
   (both of length len bytes) to the receiver. The encryption key is taken
   from the column at bit position c in the box matrix. nonce is just a
   unique integer for CPA-secure encryption (usually just a sequential number).
   It is assumed that the receiver has only one of the two decrpytion keys but
   not both (either the one in column c by itself, or column c xored with our
   secret S).
   cipher is just a preallocated BCipherRandomGen, which we use internally for
   encryption (we apply resetBCipherRandomGen, so it will lose any existing
   state). It is just so we don't have to allocate a new cipher every time.
   */
typedef struct
{ BCipherRandomGen *cipher;
  const char *box;
  int n, rowBytes, *rows, k, nonce, nonceDelta, len, destParty, c;
  char *opt0, *opt1;
  const char *spack;
  ProtocolTransport *trans;
  char *buf; int bufused;
  OcOtCorrelator corrFun; // Callback function
  void* corrArg;
  void* sender;
} SendMsgArgs;

#define MSGBUFFER_SIZE 10000
static void sendBufFlush(SendMsgArgs* a)
{
  transSend(a->trans,a->destParty,a->buf,a->bufused);
  a->bufused=0;
}

static void sendBufSend(SendMsgArgs* a,const char* data)
{
  if(a->bufused>=a->len*MSGBUFFER_SIZE) sendBufFlush(a);
  memcpy(a->buf+a->bufused,data,a->len);
  a->bufused+=a->len;
}
static void sendBufInit(SendMsgArgs* a)
{
  a->buf=malloc(a->len*MSGBUFFER_SIZE);
  a->bufused=0;
}
static void sendBufRelease(SendMsgArgs* a) { sendBufFlush(a); free(a->buf); }

void
senderExtensionBoxSendMsg(SendMsgArgs* a)
{
  int i;
  char keyx[a->cipher->klen], *ctext = malloc(a->len);
  assert(a->k%8==0 && a->k/8<=sizeof keyx);
  memset(keyx+a->k/8,0,sizeof keyx-a->k/8);
  //for(i=0;i<k;++i) setBit(keyx,i,getBit(box+rows[i]*rowBytes,c));
  for(i=0;i<a->k/8;i++)
  {
    int j=0; char ch=0;
    for(j=7;j>=0;--j)
      ch = ((ch<<1)|getBit(a->box+a->rows[8*i+j]*a->rowBytes,a->c));
    keyx[i]=ch;
  }
  if(a->corrFun)                            // If we are doing correlated OTs
  { memset(ctext,0,a->len);                 // Decrypt zeroes
    bcipherCryptNoResize(a->cipher,keyx,a->nonce,a->opt0,ctext,a->len);
    a->corrFun(a->opt1,a->opt0,a->c,a->corrArg); // Get other option
    memxor(keyx,a->spack,a->k/8);           // Encrypt with new key
    bcipherCryptNoResize(a->cipher,keyx,a->nonce,ctext,a->opt1,a->len);
    sendBufSend(a,ctext);
  }else
  { bcipherCryptNoResize(a->cipher,keyx,a->nonce,ctext,a->opt0,a->len);
    sendBufSend(a,ctext);
    memxor(keyx,a->spack,a->k/8);
    bcipherCryptNoResize(a->cipher,keyx,a->nonce,ctext,a->opt1,a->len);
    sendBufSend(a,ctext);
  }
  a->nonce+=a->nonceDelta;
  free(ctext);
}
// mask should be the same as the one previously used to construct box[]
// All other parameters (c,nonce,box,rowBytes etc.) should match the ones the
// sender is expected to use.
typedef struct
{ BCipherRandomGen *cipher;
  const char *box;
  int n, rowBytes, *rows, k, nonce, nonceDelta, len, srcParty, c;
  char *msg;
  const char *mask; // Receiver's selections
  ProtocolTransport *trans;
  char *buf; int bufread, payloadLeft;
  bool isCorr;
  void* recver;
} RecvMsgArgs;
static void recvBufFill(RecvMsgArgs* a)
{
  int n = (a->payloadLeft<MSGBUFFER_SIZE?a->payloadLeft:MSGBUFFER_SIZE);
  transRecv(a->trans,a->srcParty,a->buf,n*a->len);
  a->bufread=0;
}

static void recvBufRecv(RecvMsgArgs* a,char* data)
{
  if(a->bufread>=a->len*MSGBUFFER_SIZE) recvBufFill(a);
  memcpy(data,a->buf+a->bufread,a->len);
  a->bufread+=a->len;
  a->payloadLeft--;
}
static void recvBufInit(RecvMsgArgs* a)
{
  a->buf=malloc(a->len*MSGBUFFER_SIZE);
  a->bufread=a->len*MSGBUFFER_SIZE;
  a->payloadLeft=(a->isCorr?a->n:2*a->n);
}
static void recvBufRelease(RecvMsgArgs* a) { free(a->buf); }

void
recverExtensionBoxRecvMsg(RecvMsgArgs* a)
{
  int i;
  bool sel = getBit(a->mask,a->c);
  char keyx[a->cipher->klen], *ctext = malloc(a->len);
  assert(a->k%8==0 && a->k/8<=sizeof keyx);
  memset(keyx+a->k/8,0,sizeof keyx-a->k/8);
  //for(i=0;i<k;++i) setBit(keyx,i,getBit(box+rows[i]*rowBytes,c));
  for(i=0;i<a->k/8;i++)
  {
    int j=0; char ch=0;
    for(j=7;j>=0;--j)
      ch = ((ch<<1)|getBit(a->box+a->rows[8*i+j]*a->rowBytes,a->c));
    keyx[i]=ch;
  }
  // For correlated OTs, just imagine you are receiving zeroes
  if(a->isCorr) memset(sel?a->msg:ctext,0,a->len);
  else recvBufRecv(a,sel?a->msg:ctext);
  recvBufRecv(a,sel?ctext:a->msg);
  bcipherCryptNoResize(a->cipher,keyx,a->nonce,a->msg,ctext,a->len);
  a->nonce+=a->nonceDelta;
  free(ctext);
}

static void* senderExtensionBoxSendMsgs_thread(void* va)
{ SendMsgArgs* a=va;
  int i;
  sendBufInit(a);
  for(i=0;i<a->n;++i)
  { senderExtensionBoxSendMsg(a);
    a->opt0+=a->len; a->opt1+=a->len; a->c++;
  }
  sendBufRelease(a);
  return NULL;
}
void
senderExtensionBoxSendMsgs(SendMsgArgs* a)
{
  if(!transCanThread(a->trans) || a->n<=OT_THREAD_THRESHOLD)
    senderExtensionBoxSendMsgs_thread(a);

  else
  { pthread_t th[OT_THREAD_COUNT];
    SendMsgArgs si[OT_THREAD_COUNT];
    int i,ndone=0,tc=OT_THREAD_COUNT;
    for(i=0;i<tc;++i)
    { si[i]=*a;
      si[i].c=ndone;
      si[i].n=(a->n-ndone+tc-i-1)/(tc-i);
      si[i].nonce=a->nonce+ndone*a->nonceDelta;
      si[i].opt0=a->opt0+a->len*ndone;
      si[i].opt1=a->opt1+a->len*ndone;
      si[i].trans=a->trans->split(a->trans);
      si[i].cipher=copyBCipherRandomGenNoKey(a->cipher);
      pthread_create(&th[i],NULL,senderExtensionBoxSendMsgs_thread,&si[i]);
      ndone+=si[i].n;
    }
    assert(ndone==a->n);
    for(i=0;i<tc;++i)
    { pthread_join(th[i],NULL);
      si[i].trans->cleanup(si[i].trans);
      releaseBCipherRandomGen(si[i].cipher);
    }
    a->nonce+=ndone*a->nonceDelta;
  }
}

static void* recverExtensionBoxRecvMsgs_thread(void* va)
{ RecvMsgArgs* a=va;
  int i;
  recvBufInit(a);
  for(i=0;i<a->n;++i)
  { recverExtensionBoxRecvMsg(a);
    a->msg+=a->len; a->c++;
  }
  recvBufRelease(a);
  return NULL;
}
void
recverExtensionBoxRecvMsgs(RecvMsgArgs* a)
{
  if(!transCanThread(a->trans) || a->n<=OT_THREAD_THRESHOLD)
    recverExtensionBoxRecvMsgs_thread(a);
  else
  {
    pthread_t th[OT_THREAD_COUNT];
    RecvMsgArgs ri[OT_THREAD_COUNT];
    int i,ndone=0,tc=OT_THREAD_COUNT;
    for(i=0;i<tc;++i)
    { ri[i]=*a;
      ri[i].c=ndone;
      ri[i].n=(a->n-ndone+tc-i-1)/(tc-i);
      ri[i].nonce=a->nonce+ndone*a->nonceDelta;
      ri[i].msg=a->msg+a->len*ndone;
      ri[i].trans=a->trans->split(a->trans);
      ri[i].cipher=copyBCipherRandomGenNoKey(a->cipher);
      pthread_create(&th[i],NULL,recverExtensionBoxRecvMsgs_thread,&ri[i]);
      ndone+=ri[i].n;
    }
    assert(ndone==a->n);
    for(i=0;i<tc;++i)
    { pthread_join(th[i],NULL);
      ri[i].trans->cleanup(ri[i].trans);
      releaseBCipherRandomGen(ri[i].cipher);
    }
    a->nonce+=ndone*a->nonceDelta;
  }
}
static int* allRows(int n)
{ int i,*res = malloc(sizeof(int[n]));
  for(i=0;i<n;++i) res[i]=i;
  return res;
}

typedef struct HonestOTExtSender
{ SenderExtensionBox* box;
  BCipherRandomGen* padder;
  size_t nonce;
} HonestOTExtSender;

typedef struct HonestOTExtRecver
{ RecverExtensionBox* box;
  BCipherRandomGen* padder;
  size_t nonce;
} HonestOTExtRecver;

#define OT_KEY_BYTES_HONEST 10
#define OT_KEY_BYTES_MAL_HHASH 20
#define OT_KEY_BYTES_MAL_BYPAIR 38
void
honestOTExtSenderInit(HonestOTExtSender* s,ProtocolDesc* pd,
                      int destParty,int keyBytes)
{ s->box = senderExtensionBoxNew(pd,destParty,keyBytes);
  s->padder = newBCipherRandomGen();
  s->nonce = 0;
}
HonestOTExtSender*
honestOTExtSenderNew(ProtocolDesc* pd,int destParty)
{ HonestOTExtSender* s = malloc(sizeof *s);
  honestOTExtSenderInit(s,pd,destParty,OT_KEY_BYTES_HONEST);
  return s;
}
void
honestOTExtRecverInit(HonestOTExtRecver* r,ProtocolDesc* pd,
                      int srcParty,int keyBytes)
{ r->box = recverExtensionBoxNew(pd,srcParty,keyBytes);
  r->padder = newBCipherRandomGen();
  r->nonce = 0;
}
HonestOTExtRecver*
honestOTExtRecverNew(ProtocolDesc* pd,int srcParty)
{ HonestOTExtRecver* r = malloc(sizeof *r);
  honestOTExtRecverInit(r,pd,srcParty,OT_KEY_BYTES_HONEST);
  return r;
}
void
honestOTExtSenderCleanup(HonestOTExtSender* s)
{ senderExtensionBoxRelease(s->box);
  releaseBCipherRandomGen(s->padder);
}
void
honestOTExtSenderRelease(HonestOTExtSender* s)
{ honestOTExtSenderCleanup(s);
  free(s);
}
void
honestOTExtRecverCleanup(HonestOTExtRecver* r)
{ recverExtensionBoxRelease(r->box);
  releaseBCipherRandomGen(r->padder);
}
void
honestOTExtRecverRelease(HonestOTExtRecver* r)
{ honestOTExtRecverCleanup(r);
  free(r);
}

void* honestOTExtSend1Of2Start(HonestOTExtSender* s,int n)
{
  SendMsgArgs* args = malloc(sizeof(SendMsgArgs));
  int rowBytes = (n+7)/8;
  const int k = 8*s->box->keyBytes;
  const int blen = s->padder->blen;
  char *box = malloc(k*rowBytes);
  int *all = allRows(k);
  senderExtensionBox(s->box,box,rowBytes);
  *args = (SendMsgArgs) {
    .cipher=s->padder, .box=box, .n=n, .rowBytes=rowBytes, .rows=all,
    .k=k, .nonce=s->nonce, .nonceDelta = 0, .c=0,
    .opt0=NULL, .opt1=NULL, .len=0,
    .destParty=s->box->destParty, .spack=s->box->spack,
    .trans=s->box->pd->trans, .corrFun=NULL, .corrArg=NULL,
    .sender=s
  };
  return args;
}

void honestOTExtSend1Of2End(SendMsgArgs* args)
{
  free(args->rows);
  free((void*)args->box);
  free(args);
}
void honestOTExtSend1Of2Chunk(void* vargs,char* opt0,char* opt1,int nchunk,
    int len,OcOtCorrelator f,void* corrArg)
{ SendMsgArgs* args = vargs;
  SendMsgArgs suba = *args;
  suba.n=nchunk;
  suba.opt0=opt0;
  suba.opt1=opt1;
  suba.len=len;
  suba.nonce=((HonestOTExtSender*)args->sender)->nonce;
  suba.nonceDelta = (len+suba.cipher->blen-1)/suba.cipher->blen;
  suba.corrFun=f;
  suba.corrArg=corrArg;
  senderExtensionBoxSendMsgs(&suba);
  ((HonestOTExtSender*)args->sender)->nonce=suba.nonce;
  args->c+=nchunk;
  if(args->c>=args->n) honestOTExtSend1Of2End(args);
}
void honestOTExtSend1Of2Skip(void* vargs)
  { ((SendMsgArgs*)vargs)->c++; }

void
honestOTExtSend1Of2_impl(
    HonestOTExtSender* s,char* opt0,char* opt1,
    int n,int len,OcOtCorrelator f,void* corrArg)
{
  void* args = honestOTExtSend1Of2Start(s,n);
  honestOTExtSend1Of2Chunk(args,opt0,opt1,n,len,f,corrArg);
}
void honestOTExtSend1Of2(HonestOTExtSender* s,const char* opt0,const char* opt1,
    int n,int len)
{
  honestOTExtSend1Of2_impl(s,(char*)opt0,(char*)opt1,n,len,NULL,NULL);
}
void honestCorrelatedOTExtSend1Of2(HonestOTExtSender* s,char* opt0,char* opt1,
    int n,int len,OcOtCorrelator f,void* corrArg)
{
  honestOTExtSend1Of2_impl(s,(char*)opt0,(char*)opt1,n,len,f,corrArg);
}

void* honestOTExtRecv1Of2Start(HonestOTExtRecver* r,const bool* sel,int n)
{
  RecvMsgArgs* args = malloc(sizeof(RecvMsgArgs));
  int rowBytes = (n+7)/8;
  const int k = 8*r->box->keyBytes;
  const int blen = r->padder->blen;
  char *box = malloc(k*rowBytes);
  int *all = allRows(k);
  char *mask = malloc(rowBytes); packBytes(mask,sel,n);
  recverExtensionBox(r->box,box,mask,rowBytes);
  *args = (RecvMsgArgs) {
    .cipher=r->padder, .box=box, .n=n, .rowBytes=rowBytes, .rows=all,
    .k=k, .nonce=r->nonce, .nonceDelta = 0, .c=0,
    .msg=NULL, .mask=mask, .len=0,
    .srcParty=r->box->srcParty,
    .trans=r->box->pd->trans, .isCorr = false,
    .recver=r
  };
  return args;
}
void honestOTExtRecv1Of2End(RecvMsgArgs* args)
{
  free((void*)args->mask);
  free(args->rows);
  free((void*)args->box);
  free(args);
}
void honestOTExtRecv1Of2Chunk(void* vargs,char* dest,int nchunk,
    int len,bool isCorr)
{ RecvMsgArgs* args = vargs;
  RecvMsgArgs suba = *args;
  suba.n=nchunk;
  suba.msg=dest;
  suba.len=len;
  suba.nonce=((HonestOTExtRecver*)args->recver)->nonce;
  suba.nonceDelta = (len+suba.cipher->blen-1)/suba.cipher->blen;
  suba.isCorr=isCorr;
  recverExtensionBoxRecvMsgs(&suba);
  ((HonestOTExtRecver*)args->recver)->nonce=suba.nonce;
  args->c+=nchunk;
  if(args->c>=args->n) honestOTExtRecv1Of2End(args);
}
void honestOTExtRecv1Of2Skip(void* vargs)
  { ((RecvMsgArgs*)vargs)->c--; }

void honestOTExtRecv1Of2_impl(HonestOTExtRecver* r,char* dest,const bool* sel,
    int n,int len,bool isCorr)
{
  void* args = honestOTExtRecv1Of2Start(r,sel,n);
  honestOTExtRecv1Of2Chunk(args,dest,n,len,isCorr);
}
void honestOTExtRecv1Of2(HonestOTExtRecver* r,char* dest,const bool* sel,
    int n,int len)
{
  honestOTExtRecv1Of2_impl(r,dest,sel,n,len,false);
}
void honestCorrelatedOTExtRecv1Of2(HonestOTExtRecver* r,char* dest,
    const bool* sel,int n,int len)
{
  honestOTExtRecv1Of2_impl(r,dest,sel,n,len,true);
}
void honestWrapperSend(void* s,const char* opt0,const char* opt1,
    int n,int len) { honestOTExtSend1Of2(s,opt0,opt1,n,len); }
void honestWrapperRecv(void* r,char* dest,const bool* sel,
    int n,int len) { honestOTExtRecv1Of2(r,dest,sel,n,len); }

OTsender honestOTExtSenderAbstract(HonestOTExtSender* s)
{ return (OTsender){.sender=s, .send=honestWrapperSend,
                    .release=(void(*)(void*))honestOTExtSenderRelease};
}
OTrecver honestOTExtRecverAbstract(HonestOTExtRecver* r)
{ return (OTrecver){.recver=r, .recv=honestWrapperRecv,
                    .release=(void(*)(void*))honestOTExtRecverRelease};
}

#define OT_EXT_PAD_ALGO GCRY_CIPHER_AES192
#define OT_EXT_PAD_KEYBYTES 32
typedef enum {
  OTExtValidation_hhash,
  OTExtValidation_byPair, OTExtValidation_byPhair
} OTExtValidation;
typedef struct OTExtSender
{ HonestOTExtSender hs;
  BCipherRandomGen* gen;
  OTExtValidation validation;
} OTExtSender;
typedef struct OTExtRecver
{ HonestOTExtRecver hr;
  BCipherRandomGen* gen;
  OTExtValidation validation;
} OTExtRecver;
OTExtSender*
otExtSenderNew_aux(ProtocolDesc* pd,int destParty,OTExtValidation v)
{ OTExtSender* s = malloc(sizeof *s);
  char dummy[OT_EXT_PAD_KEYBYTES];
  const int keyBytes = (v==OTExtValidation_hhash?OT_KEY_BYTES_MAL_HHASH
                                                :OT_KEY_BYTES_MAL_BYPAIR);
  honestOTExtSenderInit(&s->hs,pd,destParty,keyBytes);
  releaseBCipherRandomGen(s->hs.padder);
  s->hs.padder = newBCipherRandomGenByAlgoKey(OT_EXT_PAD_ALGO,dummy);
  s->gen=newBCipherRandomGen();
  s->validation=v;
  return s;
}
OTExtSender* otExtSenderNew(ProtocolDesc* pd,int destParty)
  { return otExtSenderNew_aux(pd,destParty,OTExtValidation_hhash); }
OTExtSender* otExtSenderNew_byPair(ProtocolDesc* pd,int destParty)
  { return otExtSenderNew_aux(pd,destParty,OTExtValidation_byPair); }
OTExtSender* otExtSenderNew_byPhair(ProtocolDesc* pd,int destParty)
  { return otExtSenderNew_aux(pd,destParty,OTExtValidation_byPhair); }
void otExtSenderRelease(OTExtSender* s)
{ honestOTExtSenderCleanup(&s->hs);
  releaseBCipherRandomGen(s->gen);
  free(s);
}
OTExtRecver*
otExtRecverNew_aux(ProtocolDesc* pd,int srcParty,OTExtValidation v)
{ OTExtRecver* r = malloc(sizeof *r);
  const int keyBytes = (v==OTExtValidation_hhash?OT_KEY_BYTES_MAL_HHASH
                                                :OT_KEY_BYTES_MAL_BYPAIR);
  char dummy[OT_EXT_PAD_KEYBYTES];
  honestOTExtRecverInit(&r->hr,pd,srcParty,keyBytes);
  releaseBCipherRandomGen(r->hr.padder);
  r->hr.padder = newBCipherRandomGenByAlgoKey(OT_EXT_PAD_ALGO,dummy);
  r->gen=newBCipherRandomGen();
  r->validation=v;
  return r;
}
OTExtRecver* otExtRecverNew(ProtocolDesc* pd,int srcParty)
  { return otExtRecverNew_aux(pd,srcParty,OTExtValidation_hhash); }
OTExtRecver* otExtRecverNew_byPair(ProtocolDesc* pd,int srcParty)
  { return otExtRecverNew_aux(pd,srcParty,OTExtValidation_byPair); }
OTExtRecver* otExtRecverNew_byPhair(ProtocolDesc* pd,int srcParty)
  { return otExtRecverNew_aux(pd,srcParty,OTExtValidation_byPhair); }
void otExtRecverRelease(OTExtRecver* r)
{ honestOTExtRecverCleanup(&r->hr);
  releaseBCipherRandomGen(r->gen);
  free(r);
}
void
otExtSend1Of2(OTExtSender* ss,const char* opt0,const char* opt1,
              int n,int len)
{
  if(ss->hs.box->pd->error) return;
  HonestOTExtSender* s = &ss->hs;
  const int k = 8*s->box->keyBytes;
  int rowBytes, *rows, rc;

  if(ss->validation==OTExtValidation_hhash)
    rowBytes = (n+SECURITY_CONSTANT+7)/8;
  else rowBytes = (n+7)/8;

#ifndef PHASE_TIME_UPTO_BASE_OT
  char *box = malloc(k*rowBytes);
  bool error = false;
  senderExtensionBox(s->box,box,rowBytes);
#ifndef PHASE_TIME_UPTO_EXTENSION
  if(ss->validation==OTExtValidation_hhash)
  { rows = allRows(k);
    rc = k;
    if(!senderExtensionBoxValidate_hhash(s->box,ss->gen,box,rowBytes))
      error = true;
  }else
  { rows = malloc(sizeof(int[k/2]));
    rc = k/2;
    if(!senderExtensionBoxValidate_byPair(s->box,ss->gen,rows,box,rowBytes,
            ss->validation==OTExtValidation_byPhair))
      error = true;
    int i;
    for(i=0;i<rc;++i) setBit(s->box->spack,i,s->box->S[rows[i]]);
  }
#ifndef PHASE_TIME_UPTO_VALIDATION
  const int blen = s->padder->blen;
  if(error) s->box->pd->error = OC_ERROR_OT_EXTENSION;
  else
  { SendMsgArgs args = {
      .cipher=s->padder, .box=box, .n=n, .rowBytes=rowBytes, .rows=rows,
      .k=rc, .nonce=s->nonce, .nonceDelta=(len+blen-1)/blen,
      .opt0=(char*)opt0, .opt1=(char*)opt1, .len=len,
      .destParty=s->box->destParty, .spack=s->box->spack,
      .trans=s->box->pd->trans, .corrFun=NULL, .corrArg=NULL
    };
    senderExtensionBoxSendMsgs(&args);
    s->nonce=args.nonce;
  }
#endif
  free(rows);
#endif
  free(box);
#endif
}
void
otExtRecv1Of2(OTExtRecver* rr,char* dest,const bool* sel,
              int n,int len)
{
  if(rr->hr.box->pd->error) return;
  HonestOTExtRecver* r = &rr->hr;
  const int k = 8*r->box->keyBytes;
  int rowBytes,*rows,rc;

  if(rr->validation==OTExtValidation_hhash)
    rowBytes = (n+SECURITY_CONSTANT+7)/8;
  else rowBytes = (n+7)/8;

#ifndef PHASE_TIME_UPTO_BASE_OT
  char *box = malloc(k*rowBytes);
  bool error = false;
  char *mask = malloc(rowBytes);
  randomizeBuffer(rr->gen,mask,rowBytes);
  packBytes(mask,sel,n);
  recverExtensionBox(r->box,box,mask,rowBytes);
#ifndef PHASE_TIME_UPTO_EXTENSION
  if(rr->validation==OTExtValidation_hhash)
  { rows = allRows(k);
    rc=k;
    if(!recverExtensionBoxValidate_hhash(r->box,rr->gen,box,rowBytes))
      error = true;
  }else
  { rows = malloc(sizeof(int[k/2]));
    rc=k/2;
    if(!recverExtensionBoxValidate_byPair(r->box,rr->gen,rows,
            box,mask,rowBytes,rr->validation==OTExtValidation_byPhair))
      error = true;
  }
#ifndef PHASE_TIME_UPTO_VALIDATION
  const int blen = r->padder->blen;
  if(error) r->box->pd->error = OC_ERROR_OT_EXTENSION;
  else
  {
    RecvMsgArgs args = {
      .cipher=r->padder, .box=box, .n=n, .rowBytes=rowBytes, .rows=rows,
      .k=rc, .nonce=r->nonce, .nonceDelta=(len+blen-1)/blen,
      .msg=dest, .mask=mask, .len=len,
      .srcParty=r->box->srcParty, .trans=r->box->pd->trans, .isCorr = false
    };
    recverExtensionBoxRecvMsgs(&args);
    r->nonce=args.nonce;
  }
#endif
  free(rows);
#endif
  free(mask);
  free(box);
#endif
}
#if 0
// --------------- OT-extension (assuming passive adversary) ----------------

// If it becomes much bigger than 20, we should malloc it
#define OT_KEY_BYTES_MAX 20
#define OT_KEY_BITS_MAX (8*OT_KEY_BYTES_MAX)
#define OT_KEY_BITS(p) ((p)->otKeyBytes*8)
#define OT_KEY_BYTES_HONEST 10

typedef struct HonestOTExtSender
{ BCipherRandomGen *keyblock[OT_KEY_BITS_MAX];
  BCipherRandomGen *padder;
  size_t nonce; // incremented sequentially
  bool S[OT_KEY_BITS_MAX];
  char spack[OT_KEY_BYTES_MAX]; // same as S, in packed bytes
  ProtocolDesc* pd;
  int destparty;
  int otKeyBytes;
} HonestOTExtSender;

typedef struct HonestOTExtRecver
{ BCipherRandomGen *keyblock0[OT_KEY_BITS_MAX], *keyblock1[OT_KEY_BITS_MAX];
  BCipherRandomGen *padder;
  size_t nonce; // incremented sequentially
  ProtocolDesc* pd;
  int srcparty;
  int otKeyBytes;
} HonestOTExtRecver;


HonestOTExtSender* honestOTExtSenderNew_ofKeyBytes(ProtocolDesc* pd,
    int destparty, int paddingAlgo, int otKeyBytes)
{
  int i;
  const int otKeyBits = 8*otKeyBytes;
  char keys[OT_SEEDLEN*otKeyBits];
  char dummy[BC_SEEDLEN_MAX];
  HonestOTExtSender* sender=malloc(sizeof(HonestOTExtSender));
  assert(otKeyBytes<=OT_KEY_BYTES_MAX);
  gcry_randomize(sender->spack,otKeyBytes,GCRY_STRONG_RANDOM);
  sender->pd=pd; sender->destparty=destparty; sender->otKeyBytes=otKeyBytes;
  sender->nonce=0;
  sender->padder=newBCipherRandomGenByAlgoKey(paddingAlgo,dummy);
  assert(otKeyBytes<=sender->padder->klen);
  for(i=0;i<otKeyBits;++i) sender->S[i]=(sender->spack[i/8]&(1<<i%8));

  // Do the base OTs
  NpotRecver* ot=npotRecverNew(1<<BATCH_SIZE,pd,destparty);
  npotRecv1Of2(ot,keys,sender->S,otKeyBits,OT_SEEDLEN,BATCH_SIZE);
  npotRecverRelease(ot);

  // Initialize pseudorandom generators
  for(i=0;i<otKeyBits;++i)
  { sender->keyblock[i]=newBCipherRandomGen();
    resetBCipherRandomGen(sender->keyblock[i],keys+i*OT_SEEDLEN);
  }
  return sender;

}
// Base OT is done using npotSend1Of2
HonestOTExtSender* honestOTExtSenderNew(ProtocolDesc* pd,int destparty)
{
  return honestOTExtSenderNew_ofKeyBytes(pd, destparty, GCRY_CIPHER_AES128,
                                                        OT_KEY_BYTES_HONEST);
}

void honestOTExtSenderRelease(HonestOTExtSender* sender)
{ int i;
  for(i=0;i<OT_KEY_BITS(sender);++i)
	  releaseBCipherRandomGen(sender->keyblock[i]);
  releaseBCipherRandomGen(sender->padder);
  free(sender);
}

HonestOTExtRecver* honestOTExtRecverNew_ofKeyBytes(ProtocolDesc* pd,
    int srcparty, int paddingAlgo, int otKeyBytes)
{
	int i;
  const int otKeyBits = 8*otKeyBytes;
  char keys0[OT_SEEDLEN*otKeyBits], keys1[OT_SEEDLEN*otKeyBits];
  char dummy[BC_SEEDLEN_MAX];
	HonestOTExtRecver* recver = malloc(sizeof(HonestOTExtRecver));
	gcry_randomize(keys0,OT_SEEDLEN*otKeyBits,GCRY_STRONG_RANDOM);
	gcry_randomize(keys1,OT_SEEDLEN*otKeyBits,GCRY_STRONG_RANDOM);
	recver->pd=pd; recver->srcparty=srcparty; recver->otKeyBytes=otKeyBytes;
	recver->nonce=0;
	recver->padder=newBCipherRandomGenByAlgoKey(paddingAlgo,dummy);

	// Do the base OTs
	NpotSender* ot=npotSenderNew(1<<BATCH_SIZE,pd,srcparty);
	npotSend1Of2(ot,keys0,keys1,otKeyBits,OT_SEEDLEN,BATCH_SIZE);
	npotSenderRelease(ot);

	// Initialize pseudorandom generators
	for(i=0;i<otKeyBits;++i)
	{ recver->keyblock0[i]=newBCipherRandomGenByKey(keys0+i*OT_SEEDLEN);
		recver->keyblock1[i]=newBCipherRandomGenByKey(keys1+i*OT_SEEDLEN);
	}
	return recver;
}

HonestOTExtRecver* honestOTExtRecverNew(ProtocolDesc* pd,int srcparty)
{
	return honestOTExtRecverNew_ofKeyBytes(pd, srcparty, GCRY_CIPHER_AES128,
                                                       OT_KEY_BYTES_HONEST);
}

void honestOTExtRecverRelease(HonestOTExtRecver* recver)
{ int i;
  for(i=0;i<OT_KEY_BITS(recver);++i)
  { releaseBCipherRandomGen(recver->keyblock0[i]);
    releaseBCipherRandomGen(recver->keyblock1[i]);
  }
  releaseBCipherRandomGen(recver->padder);
  free(recver);
}

void honestOTExtSend1Of2RecvCryptokey(HonestOTExtSender* s,
    char *cryptokeys,int n,int len)
{
  int i,j;
  const int bytes = (n+7)/8;
  const int keybytes = s->otKeyBytes;
  char* pseudorandom = malloc(bytes);
  orecv(s->pd,s->destparty,cryptokeys,n*keybytes);
  for(i=0;i<OT_KEY_BITS(s);++i)
  { randomizeBuffer(s->keyblock[i],pseudorandom,bytes);
    if(s->S[i]==0) for(j=0;j<n;++j)
      setBit(cryptokeys+j*keybytes,i,getBit(pseudorandom,j));
    else for(j=0;j<n;++j)
      xorBit(cryptokeys+j*keybytes,i,getBit(pseudorandom,j));
  }
  free(pseudorandom);
}

void honestOTExtSend1Of2SendMessages(HonestOTExtSender* s,
    const char* opt0,const char* opt1,
    char *cryptokeys,int n,int len)
{
  int i,j;
  const int keybytes = s->otKeyBytes;
  char* cipher = malloc(len);
  assert(OT_KEY_BITS(s)%8==0);
  for(i=0;i<n;++i)
  { bcipherCrypt(s->padder,cryptokeys+i*keybytes,keybytes,
                 s->nonce,cipher,opt0+i*len,len);
    osend(s->pd,s->destparty,cipher,len);
    for(j=0;j<keybytes;++j) cryptokeys[i*keybytes+j]^=s->spack[j];
    bcipherCrypt(s->padder,cryptokeys+i*keybytes,keybytes,
                 s->nonce,cipher,opt1+i*len,len);
    osend(s->pd,s->destparty,cipher,len);
    s->nonce++;
  }
  free(cipher);
}

void honestOTExtSend1Of2(HonestOTExtSender* s,const char* opt0,const char* opt1,
    int n,int len)
{
  char *cryptokeys = malloc(n*s->otKeyBytes);
  honestOTExtSend1Of2RecvCryptokey(s,cryptokeys,n,len);
  honestOTExtSend1Of2SendMessages(s,opt0,opt1,cryptokeys,n,len);
  free(cryptokeys);
}

void honestOTExtRecv1Of2SendCryptokey(HonestOTExtRecver* r,const bool* sel,
    char *cryptokeys0,int n,int len)
{
  int i,j;
  const int bytes = (n+7)/8;
  const int keybytes = r->otKeyBytes;
  char *cryptokeys1 = malloc(n*keybytes);
  char *pseudorandom = malloc(bytes);
  for(i=0;i<OT_KEY_BITS(r);++i)
  { randomizeBuffer(r->keyblock0[i],pseudorandom,bytes);
    for(j=0;j<n;++j) setBit(cryptokeys0+j*keybytes,i,getBit(pseudorandom,j));
    randomizeBuffer(r->keyblock1[i],pseudorandom,bytes);
    for(j=0;j<n;++j) setBit(cryptokeys1+j*keybytes,i,sel[j]^
                                                     getBit(pseudorandom,j));
  }

  for(i=0;i<n;++i) for(j=0;j<keybytes;++j)
    cryptokeys1[i*keybytes+j]^=cryptokeys0[i*keybytes+j];
  osend(r->pd,r->srcparty,cryptokeys1,n*keybytes);
  free(pseudorandom);
  free(cryptokeys1);
}

void honestOTExtRecv1Of2RecvMessages(HonestOTExtRecver* r,char* dest,
    const bool* sel,
		char *cryptokeys0,int n,int len)
{
  int i;
  char *cipher0 = malloc(len), *cipher1 = malloc(len);
  const int keybytes = r->otKeyBytes;
  for(i=0;i<n;++i)
  { orecv(r->pd,r->srcparty,cipher0,len);
    orecv(r->pd,r->srcparty,cipher1,len);
    bcipherCrypt(r->padder,cryptokeys0+i*keybytes,keybytes,r->nonce++,dest+i*len,
        (sel[i]?cipher1:cipher0),len);
  }
  free(cipher0);
  free(cipher1);
}

void honestOTExtRecv1Of2(HonestOTExtRecver* r,char* dest,const bool* sel,
    int n,int len)
{
  char *cryptokeys0 = malloc(n*r->otKeyBytes);
  honestOTExtRecv1Of2SendCryptokey(r,sel,cryptokeys0,n,len);
  honestOTExtRecv1Of2RecvMessages(r,dest,sel,cryptokeys0,n,len);
  free(cryptokeys0);
}
#undef BATCH_SIZE

// --------------- OT-extension (assuming active adversary) ----------------
typedef struct MaliciousOTExtSender
{
	HonestOTExtSender *honestSender;
	BCipherRandomGen *randomGen; // Used to jointly generate the random matrix
} MaliciousOTExtSender;

typedef struct MaliciousOTExtRecver
{
	HonestOTExtRecver *honestRecver;
	BCipherRandomGen *randomGen; // Used to jointly generate the random matrix
} MaliciousOTExtRecver;

MaliciousOTExtSender* maliciousOTExtSenderNew(ProtocolDesc* pd,int destparty)
{
	MaliciousOTExtSender *sender = malloc(sizeof(MaliciousOTExtSender*));
	sender->honestSender = honestOTExtSenderNew_ofKeyBytes(pd, destparty,
                            GCRY_CIPHER_AES256,20);
	sender->randomGen = newBCipherRandomGen();
	return sender;
}

void maliciousOTExtSenderRelease(MaliciousOTExtSender* sender)
{
	honestOTExtSenderRelease(sender->honestSender);
	releaseBCipherRandomGen(sender->randomGen);
	free(sender);
}

MaliciousOTExtRecver* maliciousOTExtRecverNew(ProtocolDesc* pd,int srcparty)
{
	MaliciousOTExtRecver*recver = malloc(sizeof(MaliciousOTExtRecver*));
	recver->honestRecver= honestOTExtRecverNew_ofKeyBytes(pd, srcparty,
                            GCRY_CIPHER_AES256,20);
	recver->randomGen = newBCipherRandomGen();
	return recver;
}

void maliciousOTExtRecverRelease(MaliciousOTExtRecver* recver)
{
	honestOTExtRecverRelease(recver->honestRecver);
	releaseBCipherRandomGen(recver->randomGen);
	free(recver);
}

bool dotProduct(const char* src1, const char* src2, int bytes)
{
	unsigned char sum=0;
	int i;
	for(i=0;i<bytes;++i) sum^=(src1[i]&src2[i]);
  while(sum>1) sum = (sum>>1)^(sum&1);
	return sum;
}

//static clock_t begin;
//static clock_t end;
//static double total = 0;


void homomorphicHash(char* dest, const char* src, const char* keyMatrix, int cryptokeyLenBytes)
{
	//begin = clock();
	int i;
	for(i=0; i<CHECK_HASH_BITS; i++)
	{
		setBit(dest,i, dotProduct(src, keyMatrix+i*cryptokeyLenBytes, cryptokeyLenBytes));
	}
	//end = clock();
	//total+=(double)(end-begin);
}

bool protocolDeviationCheckSender(MaliciousOTExtSender* s, char *cryptokeys,
		BCipherRandomGen* gen, int cryptokeyLenBytes)
{
	int i, j;
  const int matbytes = cryptokeyLenBytes*CHECK_HASH_BITS;
	char* hashFunctionMatrix = malloc(matbytes);
	char* columnKey = malloc(cryptokeyLenBytes);
	char columnKeyHash[CHECK_HASH_BYTES];
	char key0Hash[CHECK_HASH_BYTES];
	char firstRHash[CHECK_HASH_BYTES];
	bool firstRHashSet = false;
	bool rv = true;
  HonestOTExtSender *ss = s->honestSender;
  const int keybytes = ss->otKeyBytes;
	rv &= ocRandomBytes(ss->pd, gen, hashFunctionMatrix, matbytes, ss->destparty);
	for(i=0;i<OT_KEY_BITS(ss);++i)
	{
		orecv(ss->pd,ss->destparty,key0Hash,CHECK_HASH_BYTES);
		for(j=0;j<cryptokeyLenBytes*8;++j)
      setBit(columnKey,j,getBit(cryptokeys+j*keybytes, i));
		homomorphicHash(columnKeyHash, columnKey, hashFunctionMatrix, cryptokeyLenBytes);
		if(ss->S[i]==0) rv &= (memcmp(columnKeyHash, key0Hash, CHECK_HASH_BYTES)==0);
		else
		{
			for(j=0;j<CHECK_HASH_BYTES;j++) columnKeyHash[j]^=key0Hash[j];
			if(!firstRHashSet)
			{
				memcpy(firstRHash, columnKeyHash, CHECK_HASH_BYTES);
				firstRHashSet = true;
			}
			else rv &= (memcmp(columnKeyHash, firstRHash, CHECK_HASH_BYTES)==0);
		}
	}
	free(hashFunctionMatrix);
	free(columnKey);
	//fprintf(stderr, "total time: %lf\n", total/CLOCKS_PER_SEC);
	return rv;
}

bool protocolDeviationCheckRecver(MaliciousOTExtRecver* r, char *cryptokeys0,
		BCipherRandomGen* gen, int cryptokeyLenBytes){
	int i,j;
  const int matbytes = cryptokeyLenBytes*CHECK_HASH_BITS;
	char* columnKey = malloc(cryptokeyLenBytes);
	char columnKeyHash[CHECK_HASH_BYTES];
	char* hashFunctionMatrix = malloc(matbytes);
  HonestOTExtRecver* rr = r->honestRecver;
  const int keybytes = rr->otKeyBytes;
  bool rv;
	rv = ocRandomBytes(rr->pd, gen, hashFunctionMatrix, matbytes, rr->srcparty);
	for(i=0;i<OT_KEY_BITS(rr);++i)
	{
		for(j=0;j<cryptokeyLenBytes*8;++j)
      setBit(columnKey,j,getBit(cryptokeys0+j*keybytes, i));
		homomorphicHash(columnKeyHash, columnKey, hashFunctionMatrix, cryptokeyLenBytes);
		osend(rr->pd,rr->srcparty,columnKeyHash,CHECK_HASH_BYTES);
	}
	free(columnKey);
	free(hashFunctionMatrix);
  return rv;
}

bool maliciousOTExtSend1Of2(MaliciousOTExtSender* s,const char* opt0,const char* opt1,
    int n,int len)
{
  int cryptokeyLenBytes = (n+SECURITY_CONSTANT+7)/8;
  int cryptokeyLenBits = cryptokeyLenBytes*8;
  // XXX this has problems: BC_SEEDLEN doesn't make any sense here
  char *cryptokeys = malloc(cryptokeyLenBits*s->honestSender->otKeyBytes);
  bool success = true;
  honestOTExtSend1Of2RecvCryptokey(s->honestSender,cryptokeys,cryptokeyLenBits,len);
  success = protocolDeviationCheckSender(s, cryptokeys, s->randomGen, cryptokeyLenBytes);
  if(success)  honestOTExtSend1Of2SendMessages(s->honestSender,opt0,opt1,cryptokeys,n,len);
  free(cryptokeys);
  return success;
}

bool maliciousOTExtRecv1Of2(MaliciousOTExtRecver* r,char* dest,const bool* sel,
    int n,int len)
{
  int cryptokeyLenBytes = (n+SECURITY_CONSTANT+7)/8;
  int cryptokeyLenBits = cryptokeyLenBytes*8;
  const int keybytes = r->honestRecver->otKeyBytes;
  char *cryptokeys0 = malloc(cryptokeyLenBits*keybytes);
  char *cryptokeys1 = malloc(cryptokeyLenBits*keybytes);
  bool *extendedSel = malloc(cryptokeyLenBits);
  randomizeBuffer(r->randomGen,(char *)extendedSel,cryptokeyLenBits);
  int i;
  for(i=0;i<cryptokeyLenBits;i++) extendedSel[i]%=2;
  memcpy(extendedSel,sel,n);
  honestOTExtRecv1Of2SendCryptokey(r->honestRecver,extendedSel,cryptokeys0,cryptokeyLenBits,len);
  protocolDeviationCheckRecver(r, cryptokeys0, r->randomGen,cryptokeyLenBytes);
  honestOTExtRecv1Of2RecvMessages(r->honestRecver,dest,sel,cryptokeys0,n,len);
  free(cryptokeys0);
  free(cryptokeys1);
  /*return r->success;*/
  return true;
}

OTsender maliciousOTExtSenderAbstract(MaliciousOTExtSender* s)
{ return (OTsender){.sender=s, .send=maliciousOTExtSend1Of2,
                    .release=(void(*)(void*))maliciousOTExtSenderRelease};
}
OTrecver maliciousOTExtRecverAbstract(MaliciousOTExtRecver* r)
{ return (OTrecver){.recver=r, .recv=maliciousOTExtRecv1Of2,
                    .release=(void(*)(void*))maliciousOTExtRecverRelease};
}
#endif

void maliciousWrapperSend(void* s,const char* opt0,const char* opt1,
    int n,int len) { otExtSend1Of2(s,opt0,opt1,n,len); }
void maliciousWrapperRecv(void* r,char* dest,const bool* sel,
    int n,int len) { otExtRecv1Of2(r,dest,sel,n,len); }

OTsender maliciousOTExtSenderAbstract(OTExtSender* s)
{ return (OTsender){.sender=s, .send=maliciousWrapperSend,
                    .release=(void(*)(void*))otExtSenderRelease};
}
OTrecver maliciousOTExtRecverAbstract(OTExtRecver* r)
{ return (OTrecver){.recver=r, .recv=maliciousWrapperRecv,
                    .release=(void(*)(void*))otExtRecverRelease};
}

void yaoUseFullOTExt(ProtocolDesc* pd,int me)
{ YaoProtocolDesc* ypd = pd->extra;
  if(me==1) ypd->sender =
    maliciousOTExtSenderAbstract(otExtSenderNew(pd,2));
  else ypd->recver =
    maliciousOTExtRecverAbstract(otExtRecverNew(pd,1));
}
