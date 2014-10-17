#include<bcrandom.h>
#include<gcrypt.h>

#define HASH_ALGO GCRY_MD_SHA1
#define HASH_BITS 160
#define HASH_BYTES (HASH_BITS/8)

typedef struct OcPsiResult
{ int n;
  int *indices;
} OcPsiResult;

void ocPsiResultRelease(OcPsiResult* res)
{
  free(res->indices);
  free(res);
}

// Return value should be released with gcry_mpi_release()
// Assumes dhRandomInit() called
gcry_mpi_t randomSecret()
{ BCipherRandomGen* gen = newBCipherRandomGen();
  gcry_mpi_t res = dhRandomExp(gen);
  releaseBCipherRandomGen(gen);
  return res;
}
void execPsiProtocol_DH(ProtocolDesc* pd,char** data,int n,int len)
{
  dhRandomInit();
  gcry_ctx_t curve;
  gcry_mpi_ec_new(&curve,NULL,DHCurveName);
  gcry_mpi_point_t *mine = malloc(n*sizeof(gcry_mpi_point_t));
  gcry_mpi_point_t *your = malloc(n*sizeof(gcry_mpi_point_t));
  gcry_mpi_point_t g = gcry_mpi_ec_get_point("g",curve,1);
  gcry_mpi_t a = randomSecret();
  gcry_mpi_t t;
  gcry_mpi_ec_mul(g,a,g,curve);
  char* hash = malloc(1+HASH_BYTES);
  int i;
  hash[0] = 0; // required for gcry_mpi_scan() to work
  for(i=0;i<n;++i) 
  {
    gcry_md_hash_buffer(HASH_ALGO,hash+1,HASH_BYTES);
    gcry_mpi_scan(&t,GCRYMPI_FMT_STD,hash,1+HASH_BYTES,NULL);
    mine[i] = gcry_mpi_point_new(0);
    gcry_mpi_ec_mul(mine[i],t,g,curve);
    gcry_mpi_release(t);
  }
  int *order = shufflePoints(mine); // order[i] = j <==> oldmine_j == newmine_i
  xchgPoints(pd,mine,your,n); // sends out mine, allocates and receives your
  for(i=0;i<n;++i)
  { // Why can't gcry_mpi_scan reuse memory if they are of the same size?!
    gcry_mpi_point_release(mine[i]);
    gcry_mpi_ec_mul(your[i],a,your[i],curve);
  }
  xchgPoints(pd,your,mine,n);
  sortPoints(your,n);
  OcPsiResult* res = ocPsiResultNew(n);
  for(i=0;i<n;++i) if(bSearch(mine[i],your,n)) 
    res->indices[res->n++] = order[i];
  res->indices = realloc(res->indices,res->n);

  for(i=0;i<n;++i) 
  { gcry_mpi_point_release(mine[i]);
    gcry_mpi_point_release(your[i]);
  }
  free(order);
  gcry_mpi_release(t);
  gcry_mpi_release(a);
  free(hash);
  gcry_mpi_point_release(g);
  free(mine); free(your);
  gcry_ctx_release(curve);
}
