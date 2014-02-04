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
bool ocRandomBytes(ProtocolDesc* pd,BCipherRandomGen* gen,
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
