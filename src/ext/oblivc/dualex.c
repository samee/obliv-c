#include<pthread.h>
#include<stdio.h>

#include<obliv.h>
#include<obliv_common.h>

// Create a temporary ProtocolTransport that uses only one of the 
//   available channels. Becomes invalid when the parent transport is cleaned up
inline static ProtocolTransport* 
  subtransport(ProtocolTransport* trans, int newChannel)
  { return trans->subtransport(trans,newChannel); }

// Yao protocol functions to be reused here
extern void setupYaoProtocol(ProtocolDesc* pd);
extern void mainYaoProtocol(ProtocolDesc* pd, protocol_run start, void* arg);
extern bool yaoGenrRevealOblivBits(ProtocolDesc* pd,
                widest_t* dest,const OblivBit* o,size_t n,int party);
extern bool yaoEvalRevealOblivBits(ProtocolDesc* pd,
                widest_t* dest,const OblivBit* o,size_t n,int party);
extern void yaoGenrFeedOblivInputs(ProtocolDesc* pd
               ,OblivInputs* oi,size_t n,int src);
extern void yaoEvalFeedOblivInputs(ProtocolDesc* pd
               ,OblivInputs* oi,size_t n,int src);

bool dualexGenrRevealOblivBits(ProtocolDesc* pd, 
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  if(party!=0) { 
    fprintf(stderr,"Right now, Dualex protocol requires all "
      "outputs to be revealed to both parties\n"); 
    exit(1);
  }
  // Reveals to evaluator only, should always return false
  return yaoGenrRevealOblivBits(pd,dest,o,n,2);
}
bool dualexEvalRevealOblivBits(ProtocolDesc* pd,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  if(party!=0) { 
    fprintf(stderr,"Right now, Dualex protocol requires all "
      "outputs to be revealed to both parties\n"); 
    exit(1);
  }
  // Reveals to evaluator only, should always return true
  return yaoEvalRevealOblivBits(pd,dest,o,n,2);
}

// TODO merge these two structs, and fix confusing pd/ypd parameters
typedef struct {
  ProtocolDesc ypd;
  int thisThread;
  void (*yFeedOblivInputs)(ProtocolDesc*,OblivInputs*,size_t,int);
} DualexHalfPD;

typedef struct {
  DualexHalfPD* pd;
  protocol_run start;
  void* startargs;
} DualexThreadArgs;

void dualexFeedOblivInputs(ProtocolDesc* pdb,OblivInputs* oi,size_t n,int party)
{
  DualexHalfPD* pd = CAST(pdb);
  if(pd->thisThread==2) party=3-party;
  pd->yFeedOblivInputs(pdb,oi,n,party);
}

void* dualexThread(void* varg)
{ DualexThreadArgs* arg = varg;
  DualexHalfPD* pd = arg->pd;
  setupYaoProtocol(&pd->ypd);
  pd->yFeedOblivInputs = pd->ypd.feedOblivInputs;
  pd->ypd.feedOblivInputs = dualexFeedOblivInputs;
  // In this function, pd->ypd.thisParty == 1 always means generator
  pd->ypd.revealOblivBits = (pd->ypd.thisParty==1
                            ?dualexGenrRevealOblivBits:dualexEvalRevealOblivBits);
  mainYaoProtocol(&pd->ypd,arg->start,arg->startargs);
  free(arg->pd->ypd.extra);
  return NULL;
}

void execDualexProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
  DualexHalfPD round1,round2;
  ProtocolTransport *trans = pd->trans;
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

  pthread_create(&t1,NULL,dualexThread,&targ1);
  pthread_create(&t2,NULL,dualexThread,&targ2);

  pthread_join(t1,NULL);
  pthread_join(t2,NULL);
}
