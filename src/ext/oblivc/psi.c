#define _GNU_SOURCE  // for qsort_r
#include<assert.h>
#include<bcrandom.h>
#include<gcrypt.h>
#include<obliv_bits.h>
#include<obliv_psi.h>
#include<stdlib.h>

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
unsigned* shufflePoints(BCipherRandomGen* gen,
                        gcry_mpi_point_t* arr,size_t n)
{
  unsigned i, *order = malloc(n*sizeof(unsigned));
  gcry_mpi_point_t *temp = malloc(n*sizeof(gcry_mpi_point_t));
  bcRandomPermutation(gen,order,n);
  for(i=0;i<n;++i) temp[i]=arr[order[i]];
  memcpy(arr,temp,n*sizeof(gcry_mpi_point_t));
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

// data: should have ni elements of len chars each
// ni  : # elements in data (I, as in me)
// nu  : # elements in data by the other person (U, as in you)
// len : length of each element, in bytes (same for both parties)
OcPsiResult* execPsiProtocol_DH(ProtocolDesc* pd,
                char** data,int ni,int nu,int len)
{
  dhRandomInit();
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
  unsigned *order = shufflePoints(gen,mine,ni);
  xchgPoints(pd,curve,mine,ni,your,nu);
  // Why can't gcry_mpi_scan reuse memory if they are of the same size?!
  for(i=0;i<ni;++i) gcry_mpi_point_release(mine[i]);
  for(i=0;i<nu;++i) gcry_mpi_ec_mul(your[i],a,your[i],curve);
  xchgPoints(pd,curve,your,nu,mine,ni);

  PointCompareArgs *args = pcaNew(curve);
  qsort_r(your,nu,sizeof(*your),compare_points,args);
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
