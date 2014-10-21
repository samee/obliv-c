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
static void dhSerialize(char* buf,gcry_mpi_point_t u,
    gcry_ctx_t ctx,gcry_mpi_t x,gcry_mpi_t y) // TODO manage these plumbings
{
  size_t s;
  unsigned char *ubuf = CAST(buf);
  const int elts = DHEltSerialBytes/2;
  gcry_mpi_ec_get_affine(x,y,u,ctx); // kinda expensive
  gcry_mpi_print(GCRYMPI_FMT_PGP,ubuf,elts,&s,x);
  while(s<elts) ubuf[s++]=0; ubuf+=elts;
  gcry_mpi_print(GCRYMPI_FMT_PGP,ubuf,elts,&s,y);
  while(s<elts) ubuf[s++]=0;
}

static void dhDeserialize(gcry_mpi_point_t* p, const char* buf)
{
  const int elts = DHEltSerialBytes/2;
  gcry_mpi_t x,y;
  gcry_mpi_scan(&x,GCRYMPI_FMT_PGP,buf,elts,NULL);
  gcry_mpi_scan(&y,GCRYMPI_FMT_PGP,buf+elts,elts,NULL);
  *p = gcry_mpi_point_snatch_set(NULL,x,y,gcry_mpi_set_ui(NULL,1));
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
void dhDebugPoint(gcry_mpi_point_t g)
{
  gcry_mpi_t gx,gy;
  gx = gcry_mpi_new(0);
  gy = gcry_mpi_new(0);
  gcry_mpi_ec_get_affine(gx,gy,g,DHCurve);
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
static void gcry_mpi_point_copy(gcry_mpi_point_t w, gcry_mpi_point_t v,
      gcry_mpi_t x,gcry_mpi_t y,gcry_mpi_t z)
{
  gcry_mpi_point_get(x,y,z,v);
  gcry_mpi_point_set(w,x,y,z);
}
// x,y,z are scratch
static void gcry_mpi_point_neg(gcry_mpi_point_t w,gcry_mpi_point_t v,
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
  gcry_mpi_point_neg(PK0,PK0,s->scratchx,s->scratchy,s->scratchz);

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
  gcry_mpi_point_copy(PK0,gk,r->scratchx,r->scratchy,r->scratchz);

  if(seli==0) { i=0; p=&gk; }
  else { i=seli-1; p=&PK0; }
  gcry_mpi_point_neg(*p,gk,r->scratchx,r->scratchy,r->scratchz);
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
  const int klen = BC_SEEDLEN;
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
  char *dummy,key[BC_SEEDLEN];
  int i;
  if(len<=HASH_BYTES) return npotRecvMany(r,dest,seli,n,len);
  dummy = malloc(len);

  npotRecvMany(r,key,seli,n,BC_SEEDLEN);
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

#define BATCH_SIZE 5

HonestOTExtSender* honestOTExtSenderNew_ofKeyBytes(ProtocolDesc* pd,
    int destparty, int otKeyBytes)
{
  int i;
  const int otKeyBits = 8*otKeyBytes;
  char keys[BC_SEEDLEN*otKeyBits];
  HonestOTExtSender* sender=malloc(sizeof(HonestOTExtSender));
  assert(otKeyBytes<=OT_KEY_BYTES_MAX);
  gcry_randomize(sender->spack,otKeyBytes,GCRY_STRONG_RANDOM);
  sender->pd=pd; sender->destparty=destparty; sender->otKeyBytes=otKeyBytes;
  sender->nonce=0;
  sender->padder=newBCipherRandomGen();
  for(i=0;i<otKeyBits;++i) sender->S[i]=(sender->spack[i/8]&(1<<i%8));

  // Do the base OTs
  NpotRecver* ot=npotRecverNew(1<<BATCH_SIZE,pd,destparty);
  npotRecv1Of2(ot,keys,sender->S,otKeyBits,BC_SEEDLEN,BATCH_SIZE);
  npotRecverRelease(ot);

  // Initialize pseudorandom generators
  for(i=0;i<otKeyBits;++i)
  { sender->keyblock[i]=newBCipherRandomGen();
    resetBCipherRandomGen(sender->keyblock[i],keys+i*BC_SEEDLEN);
  }
  return sender;

}
// Base OT is done using npotSend1Of2
HonestOTExtSender* honestOTExtSenderNew(ProtocolDesc* pd,int destparty)
{
  return honestOTExtSenderNew_ofKeyBytes(pd, destparty, OT_KEY_BYTES_HONEST);
}

void honestOTExtSenderRelease(HonestOTExtSender* sender)
{ int i;
  for(i=0;i<OT_KEY_BITS(sender);++i) 
	  releaseBCipherRandomGen(sender->keyblock[i]);
  releaseBCipherRandomGen(sender->padder);
  free(sender);
}

HonestOTExtRecver* honestOTExtRecverNew_ofKeyBytes(ProtocolDesc* pd,
    int srcparty, int otKeyBytes)
{ 
	int i;
  const int otKeyBits = 8*otKeyBytes;
  char keys0[BC_SEEDLEN*otKeyBits], keys1[BC_SEEDLEN*otKeyBits];
	HonestOTExtRecver* recver = malloc(sizeof(HonestOTExtRecver));
	gcry_randomize(keys0,BC_SEEDLEN*otKeyBits,GCRY_STRONG_RANDOM);
	gcry_randomize(keys1,BC_SEEDLEN*otKeyBits,GCRY_STRONG_RANDOM);
	recver->pd=pd; recver->srcparty=srcparty; recver->otKeyBytes=otKeyBytes;
	recver->nonce=0;
	recver->padder=newBCipherRandomGen();

	// Do the base OTs
	NpotSender* ot=npotSenderNew(1<<BATCH_SIZE,pd,srcparty);
	npotSend1Of2(ot,keys0,keys1,otKeyBits,BC_SEEDLEN,BATCH_SIZE);
	npotSenderRelease(ot);

	// Initialize pseudorandom generators
	for(i=0;i<otKeyBits;++i)
	{ recver->keyblock0[i]=newBCipherRandomGen();
		resetBCipherRandomGen(recver->keyblock0[i],keys0+i*BC_SEEDLEN);
		recver->keyblock1[i]=newBCipherRandomGen();
		resetBCipherRandomGen(recver->keyblock1[i],keys1+i*BC_SEEDLEN);
	}
	return recver;
}

HonestOTExtRecver* honestOTExtRecverNew(ProtocolDesc* pd,int srcparty)
{
	return honestOTExtRecverNew_ofKeyBytes(pd, srcparty, OT_KEY_BYTES_HONEST);
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

// setBit(a,i,v) == xorBit(a,i,v^getBit(a,i));
void setBit(char *dest,int ind,bool v)
{ char mask = (1<<ind%8);
  dest[ind/8] = (dest[ind/8]&~mask)+(v?mask:0);
}
bool getBit(const char* src,int ind) { return src[ind/8]&(1<<ind%8); }
void xorBit(char *dest,int ind,bool v) { dest[ind/8]^=(v<<ind%8); }

// Same function for encypt and decrypt. One-time pad, so don't reuse keys
// Overlapping buffers not supported
void bcipherCrypt(BCipherRandomGen* gen,const char* key,int nonce,
                  char* dest,const char* src,int n)
{
  int i;
  resetBCipherRandomGen(gen,key);
  setctrFromIntBCipherRandomGen(gen,nonce);
  randomizeBuffer(gen,dest,n);
  for(i=0;i<n;++i) dest[i]^=src[i];
}

// TODO do a malloc --> alloca wherever possible
void honestOTExtSend1Of2RecvCryptokey(HonestOTExtSender* s,
    char (*cryptokeys)[BC_SEEDLEN],int n,int len)
{
  int i,j;
  const int bytes = (n+7)/8;
  char* pseudorandom = malloc(bytes);
  orecv(s->pd,s->destparty,cryptokeys,n*BC_SEEDLEN);
  assert(OT_KEY_BITS(s)<=8*BC_SEEDLEN);
  for(i=OT_KEY_BITS(s);i<8*BC_SEEDLEN;++i) for(j=0;j<n;++j)
    setBit(cryptokeys[j],i,0);
  for(i=0;i<OT_KEY_BITS(s);++i)
  { randomizeBuffer(s->keyblock[i],pseudorandom,bytes);
    if(s->S[i]==0) for(j=0;j<n;++j) 
      setBit(cryptokeys[j],i,getBit(pseudorandom,j));
    else for(j=0;j<n;++j) 
      xorBit(cryptokeys[j],i,getBit(pseudorandom,j));
  }
  free(pseudorandom);
}

void honestOTExtSend1Of2SendMessages(HonestOTExtSender* s,
    const char* opt0,const char* opt1,
    char (*cryptokeys)[BC_SEEDLEN],int n,int len)
{
  int i,j;
  char* cipher = malloc(len);
  assert(OT_KEY_BITS(s)%8==0);
  for(i=0;i<n;++i)
  { bcipherCrypt(s->padder,cryptokeys[i],s->nonce,cipher,opt0+i*len,len);
    osend(s->pd,s->destparty,cipher,len);
    for(j=0;j<s->otKeyBytes;++j) cryptokeys[i][j]^=s->spack[j];
    bcipherCrypt(s->padder,cryptokeys[i],s->nonce,cipher,opt1+i*len,len);
    osend(s->pd,s->destparty,cipher,len);
    s->nonce++;
  }
  free(cipher);
}

void honestOTExtSend1Of2(HonestOTExtSender* s,const char* opt0,const char* opt1,
    int n,int len)
{
  char (*cryptokeys)[BC_SEEDLEN] = malloc(n*BC_SEEDLEN); // TODO structify
  honestOTExtSend1Of2RecvCryptokey(s,cryptokeys,n,len);
  honestOTExtSend1Of2SendMessages(s,opt0,opt1,cryptokeys,n,len);
  free(cryptokeys);
}

void honestOTExtRecv1Of2SendCryptokey(HonestOTExtRecver* r,const bool* sel,
    char (*cryptokeys0)[BC_SEEDLEN],int n,int len)
{
  int i,j;
  const int bytes = (n+7)/8;
  char (*cryptokeys1)[BC_SEEDLEN] = malloc(n*BC_SEEDLEN);
  char* pseudorandom = malloc(bytes);
  for(i=0;i<OT_KEY_BITS(r);++i)
  { randomizeBuffer(r->keyblock0[i],pseudorandom,bytes);
    for(j=0;j<n;++j) setBit(cryptokeys0[j],i,getBit(pseudorandom,j));
    randomizeBuffer(r->keyblock1[i],pseudorandom,bytes);
    for(j=0;j<n;++j) setBit(cryptokeys1[j],i,sel[j]^getBit(pseudorandom,j));
  }
  for(i=OT_KEY_BITS(r);i<8*BC_SEEDLEN;++i)
    for(j=0;j<n;++j)
    { setBit(cryptokeys0[j],i,0);
      setBit(cryptokeys1[j],i,0);
    }

  for(i=0;i<BC_SEEDLEN;++i) for(j=0;j<n;++j) 
    cryptokeys1[j][i]^=cryptokeys0[j][i];
  osend(r->pd,r->srcparty,cryptokeys1,n*BC_SEEDLEN);
  free(cryptokeys1);
  free(pseudorandom);
}

void honestOTExtRecv1Of2RecvMessages(HonestOTExtRecver* r,char* dest,
    const bool* sel,
		char (*cryptokeys0)[BC_SEEDLEN],int n,int len)
{
  int i;
  char *cipher0 = malloc(len), *cipher1 = malloc(len);
  for(i=0;i<n;++i) 
  { orecv(r->pd,r->srcparty,cipher0,len);
    orecv(r->pd,r->srcparty,cipher1,len);
    bcipherCrypt(r->padder,cryptokeys0[i],r->nonce++,dest+i*len,
        (sel[i]?cipher1:cipher0),len);
  }
  free(cipher0);
  free(cipher1);
}

void honestOTExtRecv1Of2(HonestOTExtRecver* r,char* dest,const bool* sel,
    int n,int len)
{
  char (*cryptokeys0)[BC_SEEDLEN] = malloc(n*BC_SEEDLEN);
  honestOTExtRecv1Of2SendCryptokey(r,sel,cryptokeys0,n,len);
  honestOTExtRecv1Of2RecvMessages(r,dest,sel,cryptokeys0,n,len);
  free(cryptokeys0);
}
#undef BATCH_SIZE

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
                            16); // XXX
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
                            16); // XXX
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

static clock_t begin;
static clock_t end;
static double total = 0;

#define CHECK_HASH_BYTES 10
#define CHECK_HASH_BITS (8*CHECK_HASH_BYTES)
#define CHECK_HASH_BITS_LOGCEIL 7
#define SECURITY_CONSTANT (2*CHECK_HASH_BITS+CHECK_HASH_BITS_LOGCEIL)


void homomorphicHash(char* dest, const char* src, const char* keyMatrix, int cryptokeyLenBytes)
{
	begin = clock();
	int i;
	char buff[CHECK_HASH_BYTES];  
	for(i=0; i<CHECK_HASH_BITS; i++) 
	{
		setBit(buff,i, dotProduct(src, keyMatrix+i*cryptokeyLenBytes, cryptokeyLenBytes));
	}
	memcpy(dest, buff, CHECK_HASH_BYTES);
	end = clock();
	total+=(double)(end-begin);
}

bool protocolDeviationCheckSender(MaliciousOTExtSender* s, char (*cryptokeys)[BC_SEEDLEN], 
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
	rv &= ocRandomBytes(ss->pd, gen, hashFunctionMatrix, matbytes, ss->destparty);
	for(i=0;i<OT_KEY_BITS(ss);++i)
	{
		orecv(ss->pd,ss->destparty,key0Hash,CHECK_HASH_BYTES);
		for(j=0;j<cryptokeyLenBytes*8;++j) setBit(columnKey,j,getBit(cryptokeys[j], i));
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
	fprintf(stderr, "total time: %lf\n", total/CLOCKS_PER_SEC);
	return rv;
}

bool protocolDeviationCheckRecver(MaliciousOTExtRecver* r, char (*cryptokeys0)[BC_SEEDLEN], 
		BCipherRandomGen* gen, int cryptokeyLenBytes){
	int i,j;
  const int matbytes = cryptokeyLenBytes*CHECK_HASH_BITS;
	char* columnKey = malloc(cryptokeyLenBytes);
	char columnKeyHash[CHECK_HASH_BYTES];
	char* hashFunctionMatrix = malloc(matbytes);
  HonestOTExtRecver* rr = r->honestRecver;
  bool rv;
	rv = ocRandomBytes(rr->pd, gen, hashFunctionMatrix, matbytes, rr->srcparty);
	for(i=0;i<OT_KEY_BITS(rr);++i)
	{
		for(j=0;j<cryptokeyLenBytes*8;++j) setBit(columnKey,j,getBit(cryptokeys0[j], i));
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
  char (*cryptokeys)[BC_SEEDLEN] = malloc(cryptokeyLenBits*BC_SEEDLEN); // TODO structify
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
  char (*cryptokeys0)[BC_SEEDLEN] = malloc(cryptokeyLenBits*BC_SEEDLEN);
  char (*cryptokeys1)[BC_SEEDLEN] = malloc(cryptokeyLenBits*BC_SEEDLEN);
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

void maliciousWrapperSend(void* s,const char* opt0,const char* opt1,
    int n,int len) { maliciousOTExtSend1Of2(s,opt0,opt1,n,len); }
void maliciousWrapperRecv(void* r,char* dest,const bool* sel,
    int n,int len) { maliciousOTExtRecv1Of2(r,dest,sel,n,len); }

OTsender maliciousOTExtSenderAbstract(MaliciousOTExtSender* s)
{ return (OTsender){.sender=s, .send=maliciousWrapperSend, 
                    .release=(void(*)(void*))maliciousOTExtSenderRelease};
}
OTrecver maliciousOTExtRecverAbstract(MaliciousOTExtRecver* r)
{ return (OTrecver){.recver=r, .recv=maliciousWrapperRecv, 
                    .release=(void(*)(void*))maliciousOTExtRecverRelease};
}

void yaoUseFullOTExt(ProtocolDesc* pd,int me)
{ YaoProtocolDesc* ypd = pd->extra;
  if(me==1) ypd->sender =
    maliciousOTExtSenderAbstract(maliciousOTExtSenderNew(pd,2));
  else ypd->recver =
    maliciousOTExtRecverAbstract(maliciousOTExtRecverNew(pd,1));
}
