#define _GNU_SOURCE  // for qsort_r
#include<assert.h>
#include<bcrandom.h>
#include<commitReveal.h>
#include<gcrypt.h>
#include<obliv_bits.h>
#include<obliv_common.h>
#include<obliv_psi.h>
#include<stdlib.h>
#include<sort_r.h> // from https://github.com/noporpoise/sort_r

#define HASH_ALGO GCRY_MD_SHA1
#define HASH_BITS 160
#define HASH_BYTES (HASH_BITS/8)


void ocPsiResultRelease(OcPsiResult* res)
{
  free(res->indices);
  free(res);
}

static
OcPsiResult* ocPsiResultNew(int n)
{
  OcPsiResult* res = malloc(sizeof(OcPsiResult));
  res->indices = malloc(n*sizeof(int));
  res->n = 0;
  return res;
}

static
unsigned* shuffleMem(BCipherRandomGen* gen,
                     void* varr,size_t n,size_t len)
{
  char* arr = varr;
  unsigned i, *order = malloc(n*sizeof(unsigned));
  char *temp = malloc(n*len);
  bcRandomPermutation(gen,order,n);
  for(i=0;i<n;++i) memcpy(temp+i*len,arr+order[i]*len,len);
  memcpy(arr,temp,n*len);
  free(temp);
  return order;
}

// Names imported from ot.c
void dhSend(gcry_mpi_point_t u,ProtocolDesc* pd,int party,
                   gcry_ctx_t ctx,gcry_mpi_t x, gcry_mpi_t y);
gcry_mpi_point_t dhRecv(ProtocolDesc* pd,int party);

// Sends out src, receives into dest.
// src,dest: arrays of gcry_mpi_t. New mpi objects are created and assigned
//           to elements of dest.
static
void xchgPoints(ProtocolDesc* pd, gcry_ctx_t ec,
                gcry_mpi_point_t* src, size_t ns,
                gcry_mpi_point_t* dest,size_t nd)
{
  assert(src!=dest);
  gcry_mpi_t x = gcry_mpi_new(0), y = gcry_mpi_new(0); // scratch for dhSend()
  unsigned i;
  if(pd->thisParty==1)
  { for(i=0;i<ns;++i) dhSend(src[i],pd,2,ec,x,y);
    for(i=0;i<nd;++i) dest[i] = dhRecv(pd,2);
  }else
  { for(i=0;i<nd;++i) dest[i] = dhRecv(pd,1);
    for(i=0;i<ns;++i) dhSend(src[i],pd,1,ec,x,y);
  }
  gcry_mpi_release(x);
  gcry_mpi_release(y);
}

typedef struct
{ gcry_mpi_t ax, ay, bx, by;
  gcry_ctx_t ec;
} PointCompareArgs;

static PointCompareArgs* pcaNew(gcry_ctx_t ec)
{ PointCompareArgs *pca;
  pca = malloc(sizeof(*pca));
  pca->ec = ec;
  pca->ax = gcry_mpi_new(0); pca->ay = gcry_mpi_new(0);
  pca->bx = gcry_mpi_new(0); pca->by = gcry_mpi_new(0);
  return pca;
}
static void pcaRelease(PointCompareArgs* pca)
{
  gcry_mpi_release(pca->ax); gcry_mpi_release(pca->ay);
  gcry_mpi_release(pca->bx); gcry_mpi_release(pca->by);
  free(pca);
}

static
int compare_points(const void* va, const void* vb, void* vargs)
{
  int c;
  PointCompareArgs *args = vargs;
  const gcry_mpi_point_t *a = va;
  const gcry_mpi_point_t *b = vb;
  gcry_mpi_ec_get_affine(args->ax,args->ay,*a,args->ec);
  gcry_mpi_ec_get_affine(args->bx,args->by,*b,args->ec);
  c = gcry_mpi_cmp(args->ax,args->bx); if(c) return c;
  c = gcry_mpi_cmp(args->ay,args->by);       return c;
}

// stdlib.h binary search didn't have an "arg" parameter
static
bool bsearchPoint(gcry_mpi_point_t p,gcry_mpi_point_t* data,size_t n,
                  PointCompareArgs* args)
{
  size_t st=0,en=n,mid;
  while(st<en)
  { mid=(st+en)/2;
    if(compare_points(data+mid,&p,args)<0) st=mid+1;
    else en=mid;
  }
  return st<n && compare_points(data+st,&p,args)==0;
}

/*
extern void dhDebugPoint(gcry_mpi_point_t p);

void debugPoints(gcry_mpi_point_t* res,size_t n)
{
  size_t i;
  for(i=0;i<n;++i)
  { fprintf(stderr,"%2zd:\n",i);
    dhDebugPoint(res[i]);
  }
}
*/

// WARNING: This protocol provides absolutely no authenticity for inequality. 
// It is possible for a malicious party to learn equality, while he convinces
// the honest party that they were not really equal (but not the other way
// around, so equality really means equal). Actually, don't use it for anything
// other than semi-honest adversaries
// data: should have ni elements of len chars each
// ni  : # elements in data (I, as in me)
// nu  : # elements in data by the other person (U, as in you)
// len : length of each element, in bytes (same for both parties)
OcPsiResult* execPsiProtocol_DH(ProtocolDesc* pd,
                char** data,int ni,int nu,int len)
{
  dhRandomInit();
  pd->error = 0;
  pd->currentParty = ocCurrentPartyDefault;
  ocSetCurrentProto(pd); // used by broadcast
  BCipherRandomGen* gen = newBCipherRandomGen();
  gcry_ctx_t curve;
  gcry_mpi_ec_new(&curve,NULL,DHCurveName);
  gcry_mpi_point_t *mine = malloc(ni*sizeof(gcry_mpi_point_t));
  gcry_mpi_point_t *your = malloc(nu*sizeof(gcry_mpi_point_t));
  gcry_mpi_point_t g = gcry_mpi_ec_get_point("g",curve,1);
  gcry_mpi_t a = dhRandomExp(gen);
  gcry_mpi_t t;
  gcry_mpi_ec_mul(g,a,g,curve);
  char* hash = malloc(1+HASH_BYTES);
  int i;
  hash[0] = 0; // required for gcry_mpi_scan() to work
  for(i=0;i<ni;++i)
  {
    gcry_md_hash_buffer(HASH_ALGO,hash+1,data[i],len);
    gcry_mpi_scan(&t,GCRYMPI_FMT_STD,hash,1+HASH_BYTES,NULL);
    mine[i] = gcry_mpi_point_new(0);
    gcry_mpi_ec_mul(mine[i],t,g,curve);
    gcry_mpi_release(t);
  }
  unsigned *order = shuffleMem(gen,mine,ni,sizeof(*mine));
  xchgPoints(pd,curve,mine,ni,your,nu);
  // Why can't gcry_mpi_scan reuse memory if they are of the same size?!
  for(i=0;i<ni;++i) gcry_mpi_point_release(mine[i]);
  for(i=0;i<nu;++i) gcry_mpi_ec_mul(your[i],a,your[i],curve);
  xchgPoints(pd,curve,your,nu,mine,ni);

  PointCompareArgs *args = pcaNew(curve);
#ifdef _WIN32
  sort_r(your,nu,nu*sizeof(gcry_mpi_point_t),compare_points,args);
#else
  qsort_r(your,nu,sizeof(*your),compare_points,args);
#endif
  OcPsiResult* res = ocPsiResultNew(ni<nu?ni:nu);
  for(i=0;i<ni;++i) if(bsearchPoint(mine[i],your,nu,args))
    res->indices[res->n++] = order[i];
  res->indices = realloc(res->indices,res->n);
  pcaRelease(args);

  for(i=0;i<ni;++i) gcry_mpi_point_release(mine[i]);
  for(i=0;i<nu;++i) gcry_mpi_point_release(your[i]);

  free(order);
  gcry_mpi_release(a);
  free(hash);
  gcry_mpi_point_release(g);
  free(mine); free(your);
  gcry_ctx_release(curve);
  releaseBCipherRandomGen(gen);
  return res;
}


/* All-pair set intersection using OT
   1. Shuffle elements (remember ordering)
   2. OT (Party 1 to 2): len*n2 instances of size n1*k, xor over len
   3. OT (Party 2 to 1): len*n1 instances of size n2*k, xor over len
   4. xor the two to obtain n1*n2 pieces of k-bit strings
   5. commitXchg of size (k*n1*n2)
   6. Report i in n1 if n1==i has any match among n2

   Optimization: the OT lengths can be much shorter (with PRGs)
*/
#define HASHBITS 80 // security param
#define HASHBYTES (HASHBITS/8)
static const char zeroes[HASHBYTES]={};

// dest needs to be of length 8*n
static void unpackData(bool* dest, char** src, size_t n,size_t len)
{
  size_t i,j; int k;
  for(i=0;i<n;++i) for(j=0;j<len;++j)
  { char ch = src[i][j];
    for(k=0;k<8;++k) dest[8*(i*len+j)+k] = (ch&(1<<j));
  }
}

/* k := HASHBYTES
   Send random data through nu*len x OT_(k*ni)
   Produces a matrix where 
   result[i][j] == hash(datai[i],j)                   // length k
                == xor-over-l(sent[j][datai[i][l]])   //
   TODO get better names/better abstraction boundaries

   Okay PL wizards: why don't ints ever have types?! They are all indices,
   and are supposed to appear in different, but specific, places. This is an
   easy thing to do automatically!
*/                 
char* sendHashes(ProtocolDesc* pd, int destParty, BCipherRandomGen* gen,
                 char** datai,int ni,int nu,int len)
{
  const int k = HASHBYTES;
  const int blen = 8*len;
  char *send0 = malloc(nu*blen*BC_SEEDLEN_DEFAULT), *send1 = malloc(nu*blen*BC_SEEDLEN_DEFAULT);
  randomizeBuffer(gen,send0,nu*blen*BC_SEEDLEN_DEFAULT);
  randomizeBuffer(gen,send1,nu*blen*BC_SEEDLEN_DEFAULT);
  OTsender sender 
    =  honestOTExtSenderAbstract(honestOTExtSenderNew(pd,destParty));
  sender.send(sender.sender,send0,send1,nu*blen,BC_SEEDLEN_DEFAULT);
  otSenderRelease(&sender);

  char *option0 = malloc(nu*blen*ni*k);
  char *option1 = malloc(nu*blen*ni*k);
  int i,j,l;
  for(i=0;i<nu;++i) for(j=0;j<blen;++j)
  { randomizeBufferByKey(send0+(i*blen+j)*BC_SEEDLEN_DEFAULT,
      option0+(i*blen+j)*(ni*k),ni*k);
    randomizeBufferByKey(send1+(i*blen+j)*BC_SEEDLEN_DEFAULT,
      option1+(i*blen+j)*(ni*k),ni*k);
  }
  free(send0); free(send1);
  char* res = calloc(ni*nu,k);
  for(i=0;i<ni;++i) for(j=0;j<nu;++j)
    for(l=0;l<blen;++l) 
    { int ii = i*nu+j;
      memxor(res+ii*k,(getBit(datai[i],l)?option1:option0)+(ii*blen+l)*k,k);
    }
  free(option0); free(option1);
  return res;
}
char* recvHashes(ProtocolDesc* pd, int srcParty, 
                 char** datai,int ni,int nu,int len)
{
  const int k = HASHBYTES;
  const int blen = 8*len;
  char *recv = malloc(ni*blen*BC_SEEDLEN_DEFAULT);
  bool *sel = malloc(ni*blen*sizeof(bool));
  unpackData(sel,datai,ni,len);
  OTrecver recver
    = honestOTExtRecverAbstract(honestOTExtRecverNew(pd,srcParty));
  recver.recv(recver.recver,recv,sel,ni*blen,BC_SEEDLEN_DEFAULT);
  otRecverRelease(&recver);
  free(sel);

  char *expanded = malloc(ni*blen*nu*k);
  int i,j,l;
  for(i=0;i<ni;++i) for(j=0;j<blen;++j)
    randomizeBufferByKey(recv+(i*blen+j)*BC_SEEDLEN_DEFAULT,
        expanded+(i*blen+j)*(nu*k),nu*k);
  free(recv);
  char* res = calloc(ni*nu,k);
  for(i=0;i<nu;++i) for(j=0;j<ni;++j)
    for(l=0;l<blen;++l)
    { int ii = i*ni+j;
      memxor(res+ii*k,expanded+(ii*blen+l)*k,k);
    }
  free(expanded);
  return res;
}

// Performs step 2,3,4,5 above
char* xchgHashes(ProtocolDesc* pd,
    BCipherRandomGen* gen,char** data,int ni,int nu,int len)
{
  char *hashi,*hashu;
  if(pd->thisParty==1)
  { hashi = sendHashes(pd,2,gen,data,ni,nu,len);
    hashu = recvHashes(pd,2,data,ni,nu,len);
  }else
  { hashu = recvHashes(pd,1,data,ni,nu,len);
    hashi = sendHashes(pd,1,gen,data,ni,nu,len);
  }
  memxor(hashi,hashu,ni*nu*HASHBYTES);
  ocXchgBytes(pd,gen,hashi,hashu,HASHBYTES*ni*nu,3-pd->thisParty);
  memxor(hashi,hashu,ni*nu*HASHBYTES);
  free(hashu);
  return hashi;
}

OcPsiResult* execPsiProtocol_allPair(ProtocolDesc* pd,
                    char** data_input,int ni,int nu,int len)
{
  dhRandomInit();
  pd->currentParty = ocCurrentPartyDefault;
  ocSetCurrentProto(pd); // used by broadcast
  BCipherRandomGen* gen = newBCipherRandomGen();
  int i,j;
  char** data = malloc(ni*sizeof(char*));
  for(i=0;i<ni;++i) data[i]=data_input[i];
  unsigned* order = shuffleMem(gen,data,ni,sizeof(*data));
  char* hashes = xchgHashes(pd,gen,data,ni,nu,len);

  OcPsiResult* res = ocPsiResultNew(ni<nu?ni:nu);
  for(i=0;i<ni;++i) for(j=0;j<nu;++j) 
    if(memcmp(hashes+(i*nu+j)*HASHBYTES,zeroes,HASHBYTES)==0)
    { res->indices[res->n++]=order[i];
      break;
    }
  res->indices = realloc(res->indices,res->n);
  free(hashes);
  free(order);
  free(data);
  releaseBCipherRandomGen(gen);
  return res;
}
