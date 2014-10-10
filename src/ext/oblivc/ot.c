#include<assert.h>
#include<gcrypt.h>
#include<inttypes.h>
#include<pthread.h>
#include<stdbool.h>
#include<stdint.h>
#include<stdlib.h>

#include<bcrandom.h>
#include<obliv_common.h>
#include<obliv_types.h>

// ---------------- Diffie Hellman Random Elt Generator ----------------------

// Yuck, custom DH code. Some day, I'll switch to using a sane ECC
// implementation

// Prime p, copied from RFC3526, 2048-bit group, ID 14
#define DHEltBits 2048
#define DHEltSerialBytes ((DHEltBits+7)/8+2)
static const char DHModPString[] 
= "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"
  "29024E088A67CC74020BBEA63B139B22514A08798E3404DD"
  "EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
  "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
  "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D"
  "C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"
  "83655D23DCA3AD961C62F356208552BB9ED529077096966D"
  "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
  "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9"
  "DE2BCBF6955817183995497CEA956AE515D2261898FA0510"
  "15728E5A8AACAA68FFFFFFFFFFFFFFFF";
static gcry_mpi_t DHModP,DHModPMinus3;
static gcry_mpi_t gcryMpiZero, gcryMpiOne;
// Generator is 2

BCipherRandomGen* newBCipherRandomGen() 
{ 
  const int algo = GCRY_CIPHER_AES128; // change BC_SEEDLEN if this changes
  size_t klen;
  BCipherRandomGen* gen;
  unsigned char key[16];
  int i;
  gcry_cipher_hd_t cipher;

  gcryDefaultLibInit();
  gen = malloc(sizeof(BCipherRandomGen)); 
  gcry_cipher_open(&cipher,algo,GCRY_CIPHER_MODE_CTR,0);
  gen->cipher = cipher;
  klen = gcry_cipher_get_algo_keylen(algo);
  assert(klen<=BC_SEEDLEN);
  gen->blen = gcry_cipher_get_algo_blklen(algo);
  assert(klen<=sizeof(key));
  assert(gen->blen<=sizeof(gen->zeroes));
  assert(gen->blen>=sizeof(uint64_t)); // used in setctrFromIntBCipherRandomGen
  gcry_randomize(key,klen,GCRY_STRONG_RANDOM);
  gcry_cipher_setkey(cipher,key,klen);
  for(i=0;i<gen->blen;++i) gen->zeroes[i]=gen->ctr[i]=0;
  return gen;
}
void releaseBCipherRandomGen(BCipherRandomGen* gen) 
{ 
  if(gen==NULL) return;
  gcry_cipher_close(gen->cipher);
  free(gen); 
}

// key is assumed to be BC_SEEDLEN bytes long
void resetBCipherRandomGen(BCipherRandomGen* gen,const char* key)
{
  gcry_cipher_reset(gen->cipher);
  gcry_cipher_setkey(gen->cipher,key,BC_SEEDLEN);
}
void setctrFromIntBCipherRandomGen(BCipherRandomGen* gen,uint64_t ctr)
{
  const int isz = sizeof(ctr);
  memcpy(gen->ctr,&ctr,isz);
  memcpy(gen->ctr+isz,gen->zeroes,gen->blen-isz);
  gcry_cipher_setctr(gen->cipher,gen->ctr,gen->blen);
}

void randomizeBuffer(BCipherRandomGen* gen,char* dest,size_t len)
{ unsigned char lastout[BC_MAXBLEN];
  int i;
  const size_t blen = gen->blen;
  for(i=0;i+blen<=len;i+=blen)
    gcry_cipher_encrypt(gen->cipher,(unsigned char*)dest+i,blen,
        gen->zeroes,blen);
  if(i<len)
  { gcry_cipher_encrypt(gen->cipher,lastout,blen,gen->zeroes,blen);
    memcpy(dest+i,lastout,len-i); // discard last few bits
  }
}

// allocates and returns a new DH element in range [2,p-2]
static gcry_mpi_t dhRandomElt(BCipherRandomGen* gen)
{
  char out[1+(DHEltBits+7)/8];
  const int outsize = sizeof(out)/sizeof(*out);
  int islarge;
  gcry_mpi_t x;
  out[0]=0;  // needed for unsigned numbers in GCRYMPI_FMT_STD
  do
  { randomizeBuffer(gen,out+1,outsize-1);
    gcry_mpi_scan(&x,GCRYMPI_FMT_STD,out,outsize,NULL);
    islarge = gcry_mpi_cmp(x,DHModPMinus3);
  } while(islarge>=0);
  gcry_mpi_add_ui(x,x,2);
  return x;
}

static pthread_once_t dhRandomInitDone = PTHREAD_ONCE_INIT;
// Needs to be invoked before any other functions here
static void dhRandomInitAux(void) 
{
  gcryDefaultLibInit();
  gcryMpiZero = gcry_mpi_set_ui(NULL,0);
  gcryMpiOne  = gcry_mpi_set_ui(NULL,1);
  gcry_mpi_scan(&DHModP,GCRYMPI_FMT_HEX,DHModPString,0,NULL);
  DHModPMinus3 = gcry_mpi_new(0);
  gcry_mpi_sub_ui(DHModPMinus3,DHModP,3);
}

void dhRandomInit(void) { pthread_once(&dhRandomInitDone,dhRandomInitAux); }

// It would be nice if somebody called this at the end of main, but it
//   simply frees memory before exit.
void dhRandomFinalize(void)
{
  // if(!dhRandomInitDone) return;
  gcry_mpi_release(DHModP);
  gcry_mpi_release(DHModPMinus3);
  gcry_mpi_release(gcryMpiZero);
  gcry_mpi_release(gcryMpiOne);
}

static void dhSerialize(char* buf,gcry_mpi_t x)
{
  size_t i;
  gcry_mpi_print(GCRYMPI_FMT_PGP,(unsigned char*)buf,DHEltSerialBytes,&i,x);
  while(i<DHEltSerialBytes) buf[i++]=0;
}

static void dhDeserialize(gcry_mpi_t* p, const char* buf)
{
  gcry_mpi_scan(p,GCRYMPI_FMT_PGP,buf,DHEltSerialBytes,NULL);
}

static void dhSend(gcry_mpi_t x,ProtocolDesc* pd,int party)
{
  char buf[DHEltSerialBytes];
  dhSerialize(buf,x);
  osend(pd,party,buf,DHEltSerialBytes);
}
// Allocates a new gcry_mpi_t, and returns it
static gcry_mpi_t dhRecv(ProtocolDesc* pd,int party)
{ char buf[DHEltSerialBytes];
  gcry_mpi_t x;
  orecv(pd,party,buf,DHEltSerialBytes);
  dhDeserialize(&x,buf);
  return x;
}

/*
static void dhDebug(gcry_mpi_t x)
{
  unsigned char buff[520];
  gcry_mpi_print(GCRYMPI_FMT_HEX,buff,520,NULL,x);
  fprintf(stderr,"%s\n",buff);
}
*/

static void xorBuffer(char* dest,const char* x,const char* y,size_t len)
{ int i;
  for(i=0;i<len;++i) dest[i]=x[i]^y[i];
}

#define HASH_BYTES 32
// adds an extra counter R 
//   For 2-party, this could be slightly faster by shoving i inside R
static void oneTimePad(char* dest,const char* src,size_t n,gcry_mpi_t k,
    uint64_t R,int i)
{
  char sb[DHEltSerialBytes+sizeof(R)+sizeof(i)];
  int sz=0;
  char digest[HASH_BYTES];
  assert(n<=HASH_BYTES);
  memcpy(sb+sz,&R,sizeof(R)); sz+=sizeof(R); // careful,
  memcpy(sb+sz,&i,sizeof(i)); sz+=sizeof(i); //   endianness
  dhSerialize(sb+sz,k); sz+=DHEltSerialBytes;
  gcry_md_hash_buffer(GCRY_MD_SHA256, digest, sb, sz);
  xorBuffer(dest,src,digest,n);
}

typedef struct NpotSender
{ gcry_mpi_t r,gr,*Cr;
  uint64_t R;
  int nmax;
  BCipherRandomGen* gen;
  ProtocolDesc* pd;
  int destParty;
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
  s->r = dhRandomElt(s->gen);
  s->gr = gcry_mpi_set_ui(NULL,2);
  gcry_mpi_powm(s->gr,s->gr,s->r,DHModP);
  s->nmax = nmax;
  s->R = 0;
  s->pd = pd; s->destParty = destParty;

  dhSend(s->gr,s->pd,s->destParty);
  s->Cr = malloc(sizeof(gcry_mpi_t)*(nmax-1));
  for(i=0;i<nmax-1;++i)
  { s->Cr[i] = dhRandomElt(s->gen);
    dhSend(s->Cr[i],s->pd,s->destParty);
    gcry_mpi_powm(s->Cr[i],s->Cr[i],s->r,DHModP);
  }
  return s;
}

typedef struct { gcry_mpi_t PK0; } NpotSenderState;

static void npotSend_roundRecvKey(NpotSender* s,NpotSenderState* q)
  { q->PK0 = dhRecv(s->pd,s->destParty); }

static void npotSend_roundSendData(NpotSender* s,NpotSenderState* q,
    char** arr,int n,int len)
{
  int i;
  char buf[HASH_BYTES];
  gcry_mpi_t PK0,PKi;
  PK0 = q->PK0;

  PKi = gcry_mpi_new(DHEltSerialBytes);
  gcry_mpi_powm(PK0,PK0,s->r,DHModP);
  oneTimePad(buf,arr[0],len,PK0,s->R,0);
  osend(s->pd,s->destParty,buf,len);
  gcry_mpi_invm(PK0,PK0,DHModP);

  for(i=1;i<n;++i)
  { gcry_mpi_mulm(PKi,PK0,s->Cr[i-1],DHModP);
    oneTimePad(buf,arr[i],len,PKi,s->R,i);
    osend(s->pd,s->destParty,buf,len);
  }
  s->R++;
  gcry_mpi_release(PK0);
  gcry_mpi_release(PKi);
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
  gcry_mpi_release(s->gr);
  for(i=0;i<s->nmax-1;++i) gcry_mpi_release(s->Cr[i]);
  free(s->Cr);
  releaseBCipherRandomGen(s->gen);
  free(s);
}

typedef struct NpotRecver
{
  gcry_mpi_t gr,*C;
  int nmax;
  uint64_t R;
  BCipherRandomGen* gen;
  ProtocolDesc* pd;
  int srcParty;
} NpotRecver;

// nmax must match with that on the sender side
// See comment for npotSenderNew()
NpotRecver* npotRecverNew(int nmax,ProtocolDesc* pd,int srcParty)
{
  int i;
  NpotRecver* r;
  r = malloc(sizeof(NpotRecver));
  r->nmax = nmax;
  r->R = 0;
  r->gr = dhRecv(pd,srcParty);
  r->C = malloc(sizeof(gcry_mpi_t)*(nmax-1));
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
  gcry_mpi_t gk,PK0,*p;
  int i;
  q->seli=seli;
  q->n=n;
  q->k = dhRandomElt(r->gen);
  gk = gcry_mpi_set_ui(NULL,2);
  gcry_mpi_powm(gk,gk,q->k,DHModP);
  PK0 = gcry_mpi_copy(gk);

  if(seli==0) { i=0; p=&gk; }
  else { i=seli-1; p=&PK0; }
  gcry_mpi_invm(*p,gk,DHModP);
  gcry_mpi_mulm(*p,*p,r->C[i],DHModP);
  dhSend(PK0,r->pd,r->srcParty);

  gcry_mpi_release(gk);
  gcry_mpi_release(PK0);
}

static void npotRecv_roundRecvData(NpotRecver* r,NpotRecverState* q,char* dest,
    int len)
{
  int i;
  char selbuf[HASH_BYTES], dummybuf[HASH_BYTES];
  gcry_mpi_t gkr;
  gkr = gcry_mpi_new(DHEltSerialBytes);
  gcry_mpi_powm(gkr,r->gr,q->k,DHModP);
  for(i=0;i<q->n;++i) orecv(r->pd,r->srcParty,i==q->seli?selbuf:dummybuf,len);
  oneTimePad(dest,selbuf,len,gkr,r->R,q->seli);
  r->R++;
  gcry_mpi_release(gkr);
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
  gcry_mpi_release(r->gr);
  for(i=0;i<r->nmax-1;++i) gcry_mpi_release(r->C[i]);
  free(r->C);
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

#define OT_KEY_BYTES 10
#define OT_KEY_BITS (8*OT_KEY_BYTES)

typedef struct HonestOTExtSender
{ BCipherRandomGen *keyblock[OT_KEY_BITS];
  BCipherRandomGen *padder;
  size_t nonce; // incremented sequentially
  bool S[OT_KEY_BITS];
  char spack[OT_KEY_BYTES]; // same as S, in packed bytes
  ProtocolDesc* pd;
  int destparty;
} HonestOTExtSender;

typedef struct HonestOTExtRecver
{ BCipherRandomGen *keyblock0[OT_KEY_BITS], *keyblock1[OT_KEY_BITS];
  BCipherRandomGen *padder;
  size_t nonce; // incremented sequentially
  ProtocolDesc* pd;
  int srcparty;
} HonestOTExtRecver;

#define BATCH_SIZE 5
// Base OT is done using npotSend1Of2
HonestOTExtSender* honestOTExtSenderNew(ProtocolDesc* pd,int destparty)
{
  int i;
  char keys[BC_SEEDLEN*OT_KEY_BITS];
  HonestOTExtSender* sender=malloc(sizeof(HonestOTExtSender));
  gcry_randomize(sender->spack,OT_KEY_BYTES,GCRY_STRONG_RANDOM);
  sender->pd=pd; sender->destparty=destparty;
  sender->nonce=0;
  sender->padder=newBCipherRandomGen();
  for(i=0;i<OT_KEY_BITS;++i)  sender->S[i]=(sender->spack[i/8]&(1<<i%8));

  // Do the base OTs
  NpotRecver* ot=npotRecverNew(1<<BATCH_SIZE,pd,destparty);
  npotRecv1Of2(ot,keys,sender->S,OT_KEY_BITS,BC_SEEDLEN,BATCH_SIZE);
  npotRecverRelease(ot);

  // Initialize pseudorandom generators
  for(i=0;i<OT_KEY_BITS;++i)
  { sender->keyblock[i]=newBCipherRandomGen();
    resetBCipherRandomGen(sender->keyblock[i],keys+i*BC_SEEDLEN);
  }
  return sender;
}
void honestOTExtSenderRelease(HonestOTExtSender* sender)
{ int i;
  for(i=0;i<OT_KEY_BITS;++i) releaseBCipherRandomGen(sender->keyblock[i]);
  releaseBCipherRandomGen(sender->padder);
  free(sender);
}

HonestOTExtRecver* honestOTExtRecverNew(ProtocolDesc* pd,int srcparty)
{ int i;
  char keys0[BC_SEEDLEN*OT_KEY_BITS], keys1[BC_SEEDLEN*OT_KEY_BITS];
  HonestOTExtRecver* recver = malloc(sizeof(HonestOTExtRecver));
  gcry_randomize(keys0,BC_SEEDLEN*OT_KEY_BITS,GCRY_STRONG_RANDOM);
  gcry_randomize(keys1,BC_SEEDLEN*OT_KEY_BITS,GCRY_STRONG_RANDOM);
  recver->pd=pd; recver->srcparty=srcparty;
  recver->nonce=0;
  recver->padder=newBCipherRandomGen();

  // Do the base OTs
  NpotSender* ot=npotSenderNew(1<<BATCH_SIZE,pd,srcparty);
  npotSend1Of2(ot,keys0,keys1,OT_KEY_BITS,BC_SEEDLEN,BATCH_SIZE);
  npotSenderRelease(ot);

  // Initialize pseudorandom generators
  for(i=0;i<OT_KEY_BITS;++i)
  { recver->keyblock0[i]=newBCipherRandomGen();
    resetBCipherRandomGen(recver->keyblock0[i],keys0+i*BC_SEEDLEN);
    recver->keyblock1[i]=newBCipherRandomGen();
    resetBCipherRandomGen(recver->keyblock1[i],keys1+i*BC_SEEDLEN);
  }
  return recver;
}
void honestOTExtRecverRelease(HonestOTExtRecver* recver)
{ int i;
  for(i=0;i<OT_KEY_BITS;++i)
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

void honestOTExtSend1Of2(HonestOTExtSender* s,const char* opt0,const char* opt1,
    int n,int len)
{
  int i,j;
  const int bytes = (n+7)/8;
  char (*cryptokeys)[BC_SEEDLEN] = malloc(n*BC_SEEDLEN); // TODO structify
  char* pseudorandom = malloc(bytes);
  char* cipher = malloc(len);
  orecv(s->pd,s->destparty,cryptokeys,n*BC_SEEDLEN);
  assert(OT_KEY_BITS<=8*BC_SEEDLEN);
  for(i=OT_KEY_BITS;i<8*BC_SEEDLEN;++i) for(j=0;j<n;++j)
    setBit(cryptokeys[j],i,0);
  for(i=0;i<OT_KEY_BITS;++i)
  { randomizeBuffer(s->keyblock[i],pseudorandom,bytes);
    if(s->S[i]==0) for(j=0;j<n;++j) 
      setBit(cryptokeys[j],s->nonce,getBit(pseudorandom,j));
    else for(j=0;j<n;++j) 
      xorBit(cryptokeys[j],s->nonce,getBit(pseudorandom,j));
    s->nonce++;
  }
  for(i=0;i<n;++i)
  { bcipherCrypt(s->padder,cryptokeys[i],i,cipher,opt0+i*len,len);
    osend(s->pd,s->destparty,cipher,len);
    for(j=0;j<BC_SEEDLEN;++j) cryptokeys[i][j]^=s->spack[j];
    bcipherCrypt(s->padder,cryptokeys[i],i,cipher,opt1+i*len,len);
    osend(s->pd,s->destparty,cipher,len);
  }
  free(cipher);
  free(pseudorandom);
  free(cryptokeys);
}
void honestOTExtRecv1Of2(HonestOTExtRecver* r,char* dest,const bool* sel,
    int n,int len)
{
  int i,j;
  const int bytes = (n+7)/8;
  char (*cryptokeys0)[BC_SEEDLEN] = malloc(n*BC_SEEDLEN);
  char (*cryptokeys1)[BC_SEEDLEN] = malloc(n*BC_SEEDLEN);
  char* pseudorandom = malloc(bytes);
  char *cipher0 = malloc(len), *cipher1 = malloc(len);
  for(i=0;i<OT_KEY_BITS;++i)
  { randomizeBuffer(r->keyblock0[i],pseudorandom,bytes);
    for(j=0;j<n;++j) setBit(cryptokeys0[j],i,getBit(pseudorandom,j));
    randomizeBuffer(r->keyblock1[i],pseudorandom,bytes);
    for(j=0;j<n;++j) setBit(cryptokeys1[j],i,sel[j]^getBit(pseudorandom,j));
  }
  for(i=OT_KEY_BITS;i<8*BC_SEEDLEN;++i)
    for(j=0;j<n;++j) setBit(cryptokeys0[j],i,0);
    // Corresponding bits of cryptokeys1 remains garbage. They are ignored
    // by the sender
  for(i=0;i<BC_SEEDLEN;++i) for(j=0;j<n;++j) 
    cryptokeys1[j][i]^=cryptokeys0[j][i];
  osend(r->pd,r->srcparty,cryptokeys1,n*BC_SEEDLEN);
  for(i=0;i<n;++i) 
  { orecv(r->pd,r->srcparty,cipher0,len);
    orecv(r->pd,r->srcparty,cipher1,len);
    bcipherCrypt(r->padder,cryptokeys0[i],r->nonce++,dest+i*len,
        (sel[i]?cipher1:cipher0),len);
  }
  free(cryptokeys0);
  free(cryptokeys1);
  free(pseudorandom);
  free(cipher0);
  free(cipher1);
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
