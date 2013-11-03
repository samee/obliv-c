#include<assert.h>
#include<gcrypt.h>
#include<inttypes.h>
#include<pthread.h>
#include<stdbool.h>
#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>

#include<obliv_common.h>
#include<obliv_types.h>

// XXX Interface changes
// Then, newNpotRecver/Sender needs ProtocolDesc and other party ID
//   this way, send/recv will not have to change

// ---------------- Diffie Hellman Random Elt Generator ----------------------

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

#define MAXBLEN 20
#define SEEDLEN (256/8)
// Simply applies a block cipher in counter mode on zeroes
typedef struct 
{ gcry_cipher_hd_t cipher; unsigned char zeroes[MAXBLEN]; 
  size_t blen;
} BCipherRandomGen;

BCipherRandomGen* newBCipherRandomGen() 
{ 
  const int algo = GCRY_CIPHER_AES256; // change SEEDLEN if this changes
  size_t klen;
  BCipherRandomGen* gen;
  unsigned char key[32];
  int i;
  gcry_cipher_hd_t cipher;

  gen = malloc(sizeof(BCipherRandomGen)); 
  gcry_cipher_open(&cipher,algo,GCRY_CIPHER_MODE_CTR,0);
  gen->cipher = cipher;
  klen = gcry_cipher_get_algo_keylen(algo);
  assert(klen<=SEEDLEN);
  gen->blen = gcry_cipher_get_algo_blklen(algo);
  assert(klen<=sizeof(key));
  assert(gen->blen<=sizeof(gen->zeroes));
  gcry_randomize(key,klen,GCRY_STRONG_RANDOM);
  gcry_cipher_setkey(cipher,key,klen);
  for(i=0;i<gen->blen;++i) gen->zeroes[i]=0;
  return gen;
}
void releaseBCipherRandomGen(BCipherRandomGen* gen) 
{ 
  if(gen==NULL) return;
  gcry_cipher_close(gen->cipher);
  free(gen); 
}

// key is assumed to be SEEDLEN bytes long
void resetBCipherRandomGen(BCipherRandomGen* gen,char* key)
{
  gcry_cipher_reset(gen->cipher);
  gcry_cipher_setkey(gen->cipher,key,SEEDLEN);
}

void randomizeBuffer(BCipherRandomGen* gen,char* dest,size_t len)
{ unsigned char lastout[MAXBLEN];
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

// Use selected transport TODO
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

// nmax has to match the nmax in npotRecverNew
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

// len must match up with npotRecv
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

// n and len must match those on the sender side
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

// ceil(log_b(x))
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
  const int klen = SEEDLEN;
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
  char *dummy,key[SEEDLEN];
  int i;
  if(len<=HASH_BYTES) return npotRecvMany(r,dest,seli,n,len);
  dummy = malloc(len);

  npotRecvMany(r,key,seli,n,SEEDLEN);
  for(i=0;i<n;++i) orecv(r->pd,r->srcParty,i==seli?dest:dummy,len);

  gen = newBCipherRandomGen();
  resetBCipherRandomGen(gen,key);
  randomizeBuffer(gen,dummy,len);
  xorBuffer(dest,dest,dummy,len);

  releaseBCipherRandomGen(gen);
  free(dummy);
}

// Finally, I am ditching the double-pointer pattern
void npotSend1Of2Once(NpotSender* s,char* opt0,char* opt1,int n,int len)
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

void npotSend1Of2(NpotSender* s,char* opt0,char* opt1,int n,int len,
    int batchsize)
{
  int i;
  for (i=0;i+batchsize<=n;i+=batchsize) 
    npotSend1Of2Once(s,opt0+i*len,opt1+i*len,batchsize,len);
  if(i<n) npotSend1Of2Once(s,opt0+i*len,opt1+i*len,n-i,len);
}

void npotRecv1Of2(NpotRecver* r,char* dest,char* sel,int n,int len,
    int batchsize)
{
  int i,j;
  unsigned mask;
  for (i=0;i+batchsize<=n;i+=batchsize)
  { for(j=mask=0;j<batchsize;++j) mask|=((sel[i+j]==1)<<j);
    npotRecv1Of2Once(r,dest+i*len,mask,batchsize,len);
  }
  if(i<n) 
  { for(j=mask=0;i+j<n;++j) mask|=((sel[i+j]==1)<<j);
    npotRecv1Of2Once(r,dest+i*len,mask,n-i,len);
  }
}

/*
// -------------- Test suite, belongs in a separate file ---------------------

#define ARRSIZE(a) (sizeof(a)/sizeof(*a))
#define XBYTES 6

#include<time.h>

double wallClock()
{
  struct timespec t;
  clock_gettime(CLOCK_REALTIME,&t);
  return t.tv_sec+1e-9*t.tv_nsec;
}

void clock1Of2(int argc, char* argv[])
{
  int i,n;
  if(argc<3)
  { fprintf(stderr,"Too few parameters\n");
    return;
  }

  if(argv[1][0]=='R')
  {
    NpotRecver* r;
    char *buf,*sel;
    int i,n,bs;
    clock_t lap;
    double wlap;
    sscanf(argv[2],"%d",&n);
    buf = malloc(11*n); sel = malloc(n);
    for(i=0;i<n;++i) sel[i]=0;
    for(bs=1;bs<=16;++bs)
    { lap = clock();
      wlap = wallClock();
      r = npotRecverNew(1<<bs);
      npotRecv1Of2(r,buf,sel,n,11,bs);
      npotRecverRelease(r);
      fprintf(stderr,"R For batch size %d, CPU time is %lf, wall time is %lf\n",
          bs,(clock()-lap)/(double)(CLOCKS_PER_SEC),wallClock()-wlap);
    }
    free(buf); free(sel);
  }else
  { NpotSender* s;
    char *buf0,*buf1;
    int i,n=10,bs;
    clock_t lap;
    double wlap;
    sscanf(argv[2],"%d",&n);
    buf0 = malloc(11*n); buf1 = malloc(11*n);
    for(i=0;i<n;++i) 
    { sprintf(buf0+i*11,"0 %d",i);
      sprintf(buf1+i*11,"1 %d",i);
    }
    for(bs=1;bs<=16;++bs)
    { lap = clock();
      wlap = wallClock();
      s = npotSenderNew(1<<bs);
      npotSend1Of2(s,buf0,buf1,n,11,bs);
      npotSenderRelease(s);
      fprintf(stderr,"S For batch size %d, CPU time is %lf, wall time is %lf\n",
          bs,(clock()-lap)/(double)(CLOCKS_PER_SEC),wallClock()-wlap);
    }
    free(buf0); free(buf1);
  }
}
void test1Of2(int argc, char* argv[])
{
  int i,n;
  if(argc<3)
  { fprintf(stderr,"Too few parameters\n");
    return;
  }

  if(argv[1][0]=='R')
  {
    NpotRecver* r;
    char buf[11*20],sel[20];
    int i,n;
    n = argc-2;
    if(n>20) { fprintf(stderr,"n too large, using n=20\n"); n=20; }
    for(i=0;i<n;++i) sel[i]=(argv[i+2][0]=='1');
    r = npotRecverNew(8);
    npotRecv1Of2(r,buf,sel,n,11,4);
    npotRecverRelease(r);
    for(i=0;i<n;++i) fprintf(stderr,"Element %d: %s\n",i,buf+11*i);
  }else
  { NpotSender* s;
    char buf0[11*20], buf1[11*20];
    int i,n=10;
    sscanf(argv[2],"%d",&n);
    if(n>20) { fprintf(stderr,"n too large, using n=20\n"); n=20; }
    for(i=0;i<n;++i) 
    { sprintf(buf0+i*11,"0 %d",i);
      sprintf(buf1+i*11,"1 %d",i);
    }
    s = npotSenderNew(8);
    npotSend1Of2(s,buf0,buf1,n,11,4);
    npotSenderRelease(s);
  }
}

void testBaseLarge(int argc, char* argv[])
{
  const int len = 80;
  int n,i;
  if(argc<2
      || (argv[1][0]=='R' && argc<4)
      || (argv[1][0]=='S' && argc<3)) { 
    fprintf(stderr,"Too few parameters\n"); 
    return; 
  }
  
  sscanf(argv[2],"%d",&n);
  if(n<1) n=1;

  if(argv[1][0]=='S')
  {
    char** buf;
    NpotSender* s;
    buf=malloc(sizeof(char*)*n);
    for(i=0;i<n;++i)
    { buf[i]=malloc(len*sizeof(char));
      sprintf(buf[i],"Item lsdflkdfndslkfsdlkdfdlkf"
          "dlkfldsmfkldsfsdfldsfdslkfjsdklf %d",i+5);
    }
    s = npotSenderNew(4);
    npotSendLong(s,buf,n,len);
    npotSenderRelease(s);
    for(i=0;i<n;++i) free(buf[i]);
    free(buf);
  }else
  {
    char* buf;
    NpotRecver* r;
    buf = malloc(sizeof(char)*len);
    r = npotRecverNew(4);
    sscanf(argv[3],"%d",&i);
    if(i<0 || i>=n) { fprintf(stderr,"index out of range\n"); return; }
    npotRecvLong(r,buf,i,n,len);
    buf[len-1]=0;
    fprintf(stderr,"Data received: %s\n",buf);
    npotRecverRelease(r);
    free(buf);
  }
}
void testBaseFour(int argc, char* argv[])
{
  int n = 4;
  if(argc<=2 || argv[1][0]=='S')
  { char* buf[] = {"Hello", "World", "Seven", "More!" };
    NpotSender* s = npotSenderNew(n);
    npotSend(s,buf,n,XBYTES);
    npotSenderRelease(s);
  }else
  { char buf[XBYTES];
    NpotRecver* r = npotRecverNew(n);
    npotRecv(r,buf,argv[2][0]-'0',n,XBYTES);
    npotRecverRelease(r);
    buf[XBYTES-1]=0;
    fprintf(stderr,"Data received: %s\n",buf);
  }
}

void showhex(const unsigned char* c,size_t len)
{
  size_t i;
  for(i=0;i<len;++i) fprintf(stderr,"%02x",c[i]);
  fprintf(stderr,"\n");
}

void testMpiLoad()
{
  BCipherRandomGen* gen = newBCipherRandomGen();
  gcry_mpi_t x;
  unsigned char buf[1000];
  int i;
  for(i=0;i<1000000;++i) x=dhRandomElt(gen);
  gcry_mpi_print(GCRYMPI_FMT_HEX,buf,1000,NULL,x);
  fprintf(stderr,"%s\n",buf);
  releaseBCipherRandomGen(gen);
}

int main(int argc, char* argv[])
{
  dhRandomInit();
  clock1Of2(argc,argv);
  dhRandomFinalize();
  return 0;
}
*/
