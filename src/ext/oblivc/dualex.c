#include<pthread.h>
#include<stdio.h>

#include<obliv.h>

// Create a temporary ProtocolTransport that uses only one of the 
//   available channels. Becomes invalid when the parent transport is cleaned up
inline static ProtocolTransport* 
  subtransport(ProtocolTransport* trans, int newChannel)
  { return trans->subtransport(trans,newChannel); }

// Yao protocol functions to be reused here
extern void setupYaoProtocol(YaoProtocolDesc* pd);
extern void mainYaoProtocol(YaoProtocolDesc* pd, protocol_run start, void* arg);
extern bool yaoGenrRevealOblivBits(ProtocolDesc* pdsuper,
                widest_t* dest,const OblivBit* o,size_t n,int party);
extern bool yaoEvalRevealOblivBits(ProtocolDesc* pdsuper,
                widest_t* dest,const OblivBit* o,size_t n,int party);

bool dualexGenrRevealOblivBits(ProtocolDesc* pdb, 
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  if(party!=0) { 
    fprintf(stderr,"Right now, Dualex protocol requires all "
      "outputs to be revealed to both parties\n"); 
    exit(1);
  }
  // Reveals to evaluator only, should always return false
  return yaoGenrRevealOblivBits(pdb,dest,o,n,2);
}
bool dualexEvalRevealOblivBits(ProtocolDesc* pdb,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  if(party!=0) { 
    fprintf(stderr,"Right now, Dualex protocol requires all "
      "outputs to be revealed to both parties\n"); 
    exit(1);
  }
  // Reveals to evaluator only, should always return true
  return yaoEvalRevealOblivBits(pdb,dest,o,n,2);
}

typedef struct {
  YaoProtocolDesc* pd;
  protocol_run start;
  void* startargs;
  int thisThread;
} DualexHalf;

void* dualexThread(void* varg)
{ DualexHalf* arg = varg;
  ProtocolDesc* pdb = PROTOCOL_DESC(arg->pd);
  setupYaoProtocol(arg->pd);
  pdb->revealOblivBits = (pdb->thisParty==arg->thisThread
                         ?dualexGenrRevealOblivBits:dualexEvalRevealOblivBits);
  mainYaoProtocol(arg->pd,arg->start,arg->startargs);
  return NULL;
}

void execDualexProtocol(DualexProtocolDesc* pd, protocol_run start, void* arg)
{
  ProtocolDesc *pdb = PROTOCOL_DESC(pd);
  YaoProtocolDesc round1,round2;
  ProtocolTransport *trans = PROTOCOL_DESC(pd)->trans;
  pthread_t t1,t2;
  DualexHalf targ1 = {.pd = &round1, .start = start
                     ,.startargs = arg, .thisThread = 1};
  DualexHalf targ2 = {.pd = &round2, .start = start
                     ,.startargs = arg, .thisThread = 2};

  setCurrentParty(PROTOCOL_DESC(&round1),pdb->thisParty);
  setCurrentParty(PROTOCOL_DESC(&round2),3-pdb->thisParty);

  // Assign transport channels: assumes Yao protocol never invokes 
  //   setSubtransport, only uses the default channel
  PROTOCOL_DESC(&round1)->trans = subtransport(trans,1);
  PROTOCOL_DESC(&round2)->trans = subtransport(trans,2);

  pthread_create(&t1,NULL,dualexThread,&targ1);
  pthread_create(&t2,NULL,dualexThread,&targ2);

  pthread_join(t1,NULL);
  pthread_join(t2,NULL);
}
