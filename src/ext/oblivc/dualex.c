#include<gcrypt.h>
#include<pthread.h>
#include<stdio.h>

#include<atomic_queue.h>
#include<commitReveal.h>
#include<obliv.h>
#include<obliv_bits.h> // needed for OblivBit.*
#include<obliv_common.h>
#include<obliv_yao.h>

// Create a temporary ProtocolTransport that uses only one of the 
//   available channels. Becomes invalid when the parent transport is cleaned up
inline static ProtocolTransport* 
  subtransport(ProtocolTransport* trans, int newChannel)
  { return trans->subtransport(trans,newChannel); }

// TODO merge these two structs, and fix confusing pd/ypd parameters
typedef struct {
  ProtocolDesc ypd;
  int thisThread;
  void (*yFeedOblivInputs)(ProtocolDesc*,OblivInputs*,size_t,int);
  gcry_md_hd_t threadhash;
  atomic_queue_t queue;
} DualexHalfPD;

typedef struct {
  DualexHalfPD* pd;
  protocol_run start;
  void* startargs;
} DualexThreadArgs;

void showKey(const char* sdata,unsigned len)
{ int i,j;
  const unsigned char* data = (const unsigned char*)sdata;
  for(i=0;i<len;i+=16)
  { for(j=0;j<16 && i+j<len;++j)
      fprintf(stderr,"%2x ",data[i+j]);
    fprintf(stderr,"\n");
  }
}

bool dualexGenrRevealOblivBits(ProtocolDesc* pdb, 
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  if(party!=0) { 
    fprintf(stderr,"Right now, Dualex protocol requires all "
      "outputs to be revealed to both parties\n"); 
    exit(1);
  }
  DualexHalfPD* pd = CAST(pdb);
  int i,j;
  widest_t destx; // result from the other round
  // Reveals to evaluator only, should always return false
  bool res = yaoGenrRevealOblivBits(pdb,dest,o,n,2);
  const char* R = ((YaoProtocolDesc*)pd->ypd.extra)->R;
  yao_key_t z = {0};
  atomic_queue_read(pd->queue,&destx);
  for(i=0;i<n;++i) if(o[i].unknown)
  { OblivBit t;
    __obliv_c__copyBit(&t,&o[i]);
    for(j=0;j<YAO_KEY_BYTES;++j) 
      t.yao.w[j]^=((o[i].yao.inverted!=(bool)(destx&(1<<i)))?R[j]:z[j]);
    gcry_md_write(pd->threadhash,t.yao.w,YAO_KEY_BYTES);
  }
  return res;
}
bool dualexEvalRevealOblivBits(ProtocolDesc* pdb,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  if(party!=0) { 
    fprintf(stderr,"Right now, Dualex protocol requires all "
      "outputs to be revealed to both parties\n"); 
    exit(1);
  }
  
  DualexHalfPD* pd = CAST(pdb);
  int i;
  for(i=0;i<n;++i) if(o[i].unknown) 
    gcry_md_write(pd->threadhash,o[i].yao.w,YAO_KEY_BYTES);

  // Reveals to evaluator only, should always return true
  bool res = yaoEvalRevealOblivBits(pdb,dest,o,n,2);
  atomic_queue_write(pd->queue,dest);
  return res;
}

void dualexFeedOblivInputs(ProtocolDesc* pdb,OblivInputs* oi,size_t n,int party)
{
  DualexHalfPD* pd = CAST(pdb);
  if(pd->thisThread==2) party=3-party;
  pd->yFeedOblivInputs(pdb,oi,n,party);
}

int flipParty(ProtocolDesc* pd) { return 3-pd->thisParty; }

void* dualexThread(void* varg)
{ DualexThreadArgs* arg = varg;
  DualexHalfPD* pd = arg->pd;
  setupYaoProtocol(&pd->ypd,true);
  int role = pd->ypd.thisParty;

  if(pd->thisThread==2) pd->ypd.currentParty = flipParty;

  pd->yFeedOblivInputs = pd->ypd.feedOblivInputs;
  //yaoUseNpot(&pd->ypd,role);
  yaoUseFullOTExt(&pd->ypd,role);

  pd->ypd.feedOblivInputs = dualexFeedOblivInputs;
  // In this function, pd->ypd.thisParty == 1 always means generator
  pd->ypd.revealOblivBits = (role==1?dualexGenrRevealOblivBits
                                    :dualexEvalRevealOblivBits);
  mainYaoProtocol(&pd->ypd,true,arg->start,arg->startargs);
  yaoReleaseOt(&pd->ypd,role);
  cleanupYaoProtocol(&arg->pd->ypd);
  return NULL;
}

void gcryDefaultLibInit(void);

#define HASH_ALGO GCRY_MD_SHA256
#define HASH_LEN 32

bool dualexEqualityCheck(ProtocolDesc* pd,gcry_md_hd_t h1,gcry_md_hd_t h2)
{
  gcry_md_hd_t h;
  gcry_md_open(&h,HASH_ALGO,0);
  gcry_md_write(h,gcry_md_read(h1,0),HASH_LEN);
  gcry_md_write(h,gcry_md_read(h2,0),HASH_LEN);
  BCipherRandomGen* gen = newBCipherRandomGen();
  bool res = ocPrivateEqualityCheck_halfAuth(pd,gen,
      gcry_md_read(h,0),HASH_LEN,3-pd->thisParty);
  releaseBCipherRandomGen(gen);
  gcry_md_close(h);
  return res;
}

bool execDualexProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
  DualexHalfPD round1,round2;
  ProtocolTransport *trans = pd->trans;
  atomic_queue_t q;
  pthread_t t1,t2;
  DualexThreadArgs targ1 = {.pd=&round1, .start=start, .startargs=arg };
  DualexThreadArgs targ2 = {.pd=&round2, .start=start, .startargs=arg };

  setCurrentParty(&round1.ypd,pd->thisParty);
  setCurrentParty(&round2.ypd,3-pd->thisParty);
  round1.thisThread = 1; round2.thisThread = 2;

  // Assign transport channels: assumes Yao protocol never invokes 
  //   setSubtransport, only uses the default channel
  round1.ypd.trans = subtransport(trans,0);
  round2.ypd.trans = subtransport(trans,1);

  // These will be used in the final equality tests
  gcryDefaultLibInit();
  gcry_md_open(&round1.threadhash,HASH_ALGO,0);
  gcry_md_open(&round2.threadhash,HASH_ALGO,0);
  atomic_queue_init(&q,256,sizeof(widest_t));
  round1.queue = round2.queue = q;

  pthread_create(&t1,NULL,dualexThread,&targ1);
  pthread_create(&t2,NULL,dualexThread,&targ2);

  pthread_join(t1,NULL); 
  pthread_join(t2,NULL);

  pd->error = (round1.ypd.error?round1.ypd.error:round2.ypd.error);
  atomic_queue_release(q);
  bool res = dualexEqualityCheck(pd,round1.threadhash,round2.threadhash);

  gcry_md_close(round1.threadhash);
  gcry_md_close(round2.threadhash);
  return !pd->error && res;
}
