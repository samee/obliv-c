#include<string.h>
#include<gcrypt.h>

#include<bcrandom.h>
#include<commitReveal.h>
#include<obliv.h>

OcCommitter* ocSendCommit(ProtocolDesc* pd,BCipherRandomGen* gen, 
                          const void* data,int size,int destParty)
{
  OcCommitter* rv = malloc(sizeof(OcCommitter));
  gcry_md_hd_t h;
  randomizeBuffer(gen,rv->key,COMMIT_KEY_SIZE);
  gcry_md_open(&h,COMMIT_HASH_ALGO,0);
  rv->n = size;
  rv->data = data;
  rv->destParty = destParty;
  rv->pd = pd;
  gcry_md_write(h,&pd->thisParty,sizeof(pd->thisParty));
  gcry_md_write(h,rv->key,COMMIT_KEY_SIZE);
  gcry_md_write(h,data,size);
  gcry_md_final(h);
  osend(pd,destParty,gcry_md_read(h,0),COMMIT_HASH_BYTES);
  gcry_md_close(h);
  return rv;
}
void ocRevealCommit(OcCommitter* com)
{
  osend(com->pd,com->destParty,com->key,COMMIT_KEY_SIZE);
  osend(com->pd,com->destParty,com->data,com->n);
  free(com);
}

OcCommitment* ocRecvCommit(ProtocolDesc* pd,BCipherRandomGen* gen,int srcParty)
{
  OcCommitment* rv = malloc(sizeof(OcCommitment));
  orecv(pd,srcParty,rv->hash,COMMIT_HASH_BYTES);
  rv->pd = pd; rv->srcParty = srcParty;
  return rv;
}
bool ocCheckCommit(OcCommitment* com,int size,void* dest)
{
  int i;
  char key[COMMIT_KEY_SIZE];
  unsigned char *buf,*hash2;
  bool rv = true;
  if(dest) buf=dest; else buf=malloc(size);
  gcry_md_hd_t h;
  gcry_md_open(&h,COMMIT_HASH_ALGO,0);
  orecv(com->pd,com->srcParty,key,COMMIT_KEY_SIZE);
  orecv(com->pd,com->srcParty,buf,size);
  gcry_md_write(h,&com->srcParty,sizeof(com->srcParty));
  gcry_md_write(h,key,COMMIT_KEY_SIZE);
  gcry_md_write(h,buf,size);
  gcry_md_final(h);
  hash2 = gcry_md_read(h,0);
  for(i=0;i<COMMIT_HASH_BYTES;++i) rv &= (com->hash[i]==hash2[i]);
  gcry_md_close(h);
  if(!dest) free(buf);
  free(com);
  return rv;
}

bool ocXchgBytes(ProtocolDesc* pd,BCipherRandomGen* gen,
                 const void* src,void* dest,int n,int party)
{
  if(pd->thisParty<party)
  { OcCommitter* com = ocSendCommit(pd,gen,src,n,party);
    orecv(pd,party,dest,n);
    ocRevealCommit(com);
  }else if(pd->thisParty>party)
  { OcCommitment* com = ocRecvCommit(pd,gen,party);
    osend(pd,party,src,n);
    return ocCheckCommit(com,n,dest);
  }else memcpy(dest,src,n);
  return true;
}

// Oh please remember: this is NOT a private equality check
bool ocEqualityCheck(ProtocolDesc* pd,BCipherRandomGen* gen,
                     const void* data,int n,int party)
{
  char* buf = malloc(n);
  bool rv;
  rv = ocXchgBytes(pd,gen,data,buf,n,party);
  // we can break early: not private
  if(rv) rv = (memcmp(buf,data,n)==0);
  free(buf);
  return rv;
}
bool ocRandomBytes_impl(ProtocolDesc* pd,BCipherRandomGen* gen,
                        void* dest,int n,int party)
{
  char* buf = malloc(n);
  int i;
  bool rv;
  randomizeBuffer(gen,dest,n);
  rv = ocXchgBytes(pd,gen,dest,buf,n,party);
  if(rv) for(i=0;i<n;++i) *((char*)dest+i) ^= buf[i];
  free(buf);
  return rv;
}
bool ocRandomBytes(ProtocolDesc* pd,BCipherRandomGen* gen,
                   void* dest,int n,int party)
{
  if(n<=BC_SEEDLEN_DEFAULT)
    return ocRandomBytes_impl(pd,gen,dest,n,party);
  char buf[BC_SEEDLEN_DEFAULT];
  if(!ocRandomBytes_impl(pd,gen,buf,sizeof(buf),party)) return false;
  randomizeBufferByKey(buf,dest,n);
  return true;
}

// Copied from ot.c
void dhSerialize(char* buf,gcry_mpi_point_t u,
    gcry_ctx_t ctx,gcry_mpi_t x,gcry_mpi_t y);
void dhDeserialize(gcry_mpi_point_t* p, const char* buf);

static bool
ocXchgPoints(ProtocolDesc* pd, BCipherRandomGen* gen, gcry_ctx_t ctx,
    gcry_mpi_point_t* src, gcry_mpi_point_t* dest,int n,int party)
{
  char buf1[n][DHEltSerialBytes], buf2[n][DHEltSerialBytes];
  gcry_mpi_t x = gcry_mpi_new(0), y = gcry_mpi_new(0);
  int i;
  for(i=0;i<n;++i) dhSerialize(buf1[i],src[i],ctx,x,y);
  bool res = ocXchgBytes(pd,gen,buf1,buf2,sizeof(buf1),party);
  if(res) for(i=0;i<n;++i) dhDeserialize(dest+i,buf2[i]);
  gcry_mpi_release(x); gcry_mpi_release(y);
  return res;
}
// simply compares to ec points
static bool
ocPointsEqual(gcry_ctx_t curve,gcry_mpi_point_t a, gcry_mpi_point_t b)
{
  gcry_mpi_t ax = gcry_mpi_new(0);
  gcry_mpi_t ay = gcry_mpi_new(0);
  gcry_mpi_t bx = gcry_mpi_new(0);
  gcry_mpi_t by = gcry_mpi_new(0);
  gcry_mpi_ec_get_affine(ax,ay,a,curve);
  gcry_mpi_ec_get_affine(bx,by,b,curve);
  bool eq;
  eq = !gcry_mpi_cmp(ax,bx);
  if(eq) eq = !gcry_mpi_cmp(ay,by);
  gcry_mpi_release(ax);
  gcry_mpi_release(ay);
  gcry_mpi_release(bx);
  gcry_mpi_release(by);
  return eq;
}
static gcry_mpi_t mpiFromHash(const char* data,size_t len)
{
  char hash[COMMIT_HASH_BYTES+1];
  hash[0]=0;
  gcry_md_hash_buffer(COMMIT_HASH_ALGO,hash+1,data,len);
  gcry_mpi_t res;
  gcry_mpi_scan(&res,GCRYMPI_FMT_STD,hash,1+COMMIT_HASH_BYTES,NULL);
  return res;
}
bool ocPrivateEqualityCheck_halfAuth(ProtocolDesc* pd,BCipherRandomGen* gen,
    const void* data, int n,int party)
{
  bool res = false;
  gcry_ctx_t curve;
  gcry_mpi_ec_new(&curve,NULL,DHCurveName);
  gcry_mpi_point_t g = gcry_mpi_ec_get_point("g",curve,1);
  gcry_mpi_point_t yab, xab;

  gcry_mpi_t x = mpiFromHash(data,n), a = dhRandomExp(gen);
  gcry_mpi_ec_mul(g,x,g,curve);
  gcry_mpi_ec_mul(g,a,g,curve);

  if(!ocXchgPoints(pd,gen,curve,&g,&yab,1,party)) goto error1;
  gcry_mpi_ec_mul(yab,a,yab,curve);
  if(!ocXchgPoints(pd,gen,curve,&yab,&xab,1,party)) goto error2;
  res = ocPointsEqual(curve,xab,yab);
  gcry_mpi_point_release(xab);
error2:
  gcry_mpi_point_release(yab);
error1:
  gcry_mpi_release(x); gcry_mpi_release(a);
  gcry_mpi_point_release(g);
  gcry_ctx_release(curve);
  return res;
}
