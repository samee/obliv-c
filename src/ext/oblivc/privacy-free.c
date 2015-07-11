/*
   Implements "privacy-free" garbling scheme used for zero-knowledge proofs.
   Here, all obliv data is known to the evaluator, and feedObliv* does not work
   if party != 2. The output is either revealed to both parties, or just the 
   evaluator. The idea is that, this allows the evaluator (party 2) to prove
   that he knows inputs that produce a given output value, without having to
   reveal the inputs.

   Every function come in 4 flavors: 2 phases for two sides
   Protocol:
   prover=party 2
   verifier=party 1
   prover: outcomR <- random
   yao(evaluator=prover, generator=verifier)  // exec protocol_run x 2
     feedObliv only works from party 2
     input pairs are never permuted (ie. yaoKeyLsb has no significance)
     outputs to party 0 not sent out, just hashed with outcomR
   prover sends verifier: output hash commitment
   verifier sends prover: generator keys and OT decoms
   yao(generator = prover)                   // exec protocol_run x 2
     Runs everything with known secrets
     Transport is hooked, so that nothing is actually sent by generator,
       just hashed. feedObliv still reads inputs,
     evaluator doesn't do anything but provide inputs.
     reveal to party 0 comes alive:
       check transport hashes, disable all gates
       swap out transport for the real thing, actually start sending
       send outcomR
       start sending labels
       prover always return true
    Meantime, verifier is also running blanks: // exec protocol_run
      On first reveal, receive outcomR, start hashing
      On each label, check if 0 or 1 valid label, hash them with outputs
      AND return value with success

    So OT needs replay capabilities:
      sender needs to be replayed to regenerate the same traffic
      receiver needs to be replayed with same seed for providing choice

    OT interface: 
    Sender:
      senderNew()
      senderProc(...) {      // Same as generator proc, gates are unimportant
        OTsend()             // multiple times
      }
      OTRevealComm()         // OTsend forbidden after this
      senderRelease()
    Recver:
      recverNew()
      recverProc(...) {      // Same as evaluator proc, gates are unimportant
        OTrecv()             // multiple times
      }
      comm <- OTComm()
      Reset seeds, both side
      coroutine: senderProc(), recverProc() connected together
      bool <- OTCheckCommEnd(comm,newhash) // newhash modified by OTsend2/OTrecv2
      recverRelease()

      recverProc does not evaluate gates a second time: just provides OT
      choices.

      For OT extensions, OTRevealComm() should just send the sender's
      secret randomness. comm should have receiver's matrix 
      master seed, and hash of everything received. We do not need
      to recreate zero-knowledge H here.

    Gen(seed,OTsender) -> dumps out on transport, including OT
                          same seed & same OT random state (both sender and 
                          recver) --> same outgoing traffic
                          OTsender is sometimes connected to a real receiver
                          over the network, sometimes to a local fake receiver
   */

#include<bcrandom.h>
#include<obliv_bits.h>
#include<obliv_yao.h>
#include<gcrypt.h>
#include<stdio.h>

typedef struct
{
  char protoType;
  bool broadcast1;
  // Add more stuff here
} NpProtocolExtra;
#define OC_YPD_TYPE_NP 3

OC_DYN_EXTRA_FUN(ypdNpProtocolExtra,YaoProtocolDesc,
                 NpProtocolExtra,OC_YPD_TYPE_NP)
struct ProtocolDesc* ocCurrentProto(void); // copied from obliv_common.h

void ocNpProtoSetBroadcast1(ProtocolDesc* pd,bool v)
  { ypdNpProtocolExtra(protoYaoProtocolDesc(pd))->broadcast1=v; }
bool ocNpProtoGetBroadcast1(ProtocolDesc* pd)
  { return ypdNpProtocolExtra(protoYaoProtocolDesc(pd))->broadcast1; }
bool ocInNpProto(void)
{ YaoProtocolDesc* ypd = protoYaoProtocolDesc(ocCurrentProto());
  return ypd && ypdNpProtocolExtra(ypd)!=NULL;
}

// These 2 are declared in obliv.oh
void ocNpSetBroadcast1(bool v) 
  { ocNpProtoSetBroadcast1(ocCurrentProto(),v); }
bool ocNpGetBroadcast1()
  { return ocNpProtoGetBroadcast1(ocCurrentProto()); }

void npGenrFeedOblivInputs(ProtocolDesc* pd,OblivInputs* oi,size_t n,int src)
{
  if(src!=2 && !(src==1 && ocNpProtoGetBroadcast1(pd)))
  { fprintf(stderr,"Error: feedObliv src must be 2 in np protocol\n"); 
    return; 
  }
  if(src==2) yaoGenrFeedOblivInputs(pd,oi,n,src);
  else // broadcast value, no privacy
  { OIBitSrc it = oiBitSrc(oi,n);
    for(;hasBit(&it);nextBit(&it))
    { bool v = curBit(&it);
      OblivBit* dest = curDestBit(&it);
      osend(pd,2,&v,sizeof(bool));
      dest->unknown = false;
      dest->knownValue = v;
    }
  }
}

void npEvalFeedOblivInputs(ProtocolDesc* pd,OblivInputs* oi,size_t n,int src)
{
  if(src!=2 && !(src==1 && ocNpProtoGetBroadcast1(pd))) 
  { fprintf(stderr,"Error: feedObliv src must be 2 in np protocol\n"); 
    return; 
  }
  if(src==2) yaoEvalFeedOblivInputs(pd,oi,n,src);
  else
  { OIBitSrc it = oiBitSrc(oi,n);
    for(;hasBit(&it);nextBit(&it))
    { OblivBit* dest = curDestBit(&it);
      dest->unknown = false;
      orecv(pd,1,&dest->knownValue,sizeof(bool));
    }
  }
}

// Similar to yaoGenrRevealOblivBits. Refactor? TODO
bool npGenrRevealOblivBits(ProtocolDesc* pd,
    widest_t* dest,const OblivBit* o,size_t n,int party)
{
  if(party!=0 && party!=2) 
  { fprintf(stderr,"Error: revealObliv cannot reveal "
                   "to party %d in NP protocol\n",party);
    return false;
  }
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  YaoProtocolDesc *ypd = pd->extra;
  for(i=0;i<n;++i) if(o[i].unknown)
    flipflags |= (o[i].yao.inverted?1LL<<i:0);
  // Assuming little endian
  // TODO receive hash
  if(party == 0) { orecv(pd,2,&rv,bc); rv^=flipflags; }
  for(i=0;i<n;++i) if(!o[i].unknown && o[i].knownValue)
    rv |= (1LL<<i);
  ypd->ocount+=n;
  if(party==0) { *dest=rv; return true; }
  else return false;
}
// Similar to yaoEvalRevealOblivBits. Refactor? TODO
bool npEvalRevealOblivBits
  (ProtocolDesc* pd,widest_t* dest,const OblivBit* o,size_t n,int party)
{ 
  if(party!=0 && party!=2) 
  { fprintf(stderr,"Error: revealObliv cannot reveal "
                   "to party %d in NP protocol\n",party);
    return false;
  }
  int i,bc=(n+7)/8;
  widest_t rv=0, flipflags=0;
  YaoProtocolDesc* ypd = pd->extra;
  for(i=0;i<n;++i) if(o[i].unknown)
    flipflags |= (o[i].yao.value?1LL<<i:0);
  // Assuming little endian
  // TODO send hash, not data
  if(party == 0) osend(pd,1,&flipflags,bc);
  for(i=0;i<n;++i) if(!o[i].unknown && o[i].knownValue)
    rv |= (1LL<<i);
  rv|=flipflags;
  ypd->ocount+=n;
  *dest=rv;
  return true;
}

void npGenerateGate(ProtocolDesc* pd, OblivBit* r,
    bool ac, bool bc, bool rc, const OblivBit* a, const OblivBit* b)
{
  YaoProtocolDesc* ypd = pd->extra;
  if(a->yao.inverted) ac=!ac;
  if(b->yao.inverted) bc=!bc;

  yao_key_t row,t,wr,wb1;
  const char *wa0 = a->yao.w, *wb0 = b->yao.w;
  yaoKeyCopy(wb1,wb0); yaoKeyXor(wb1,ypd->R);

  yaoSetHalfMask(ypd,row,wb0,ypd->gcount);
  yaoSetHalfMask(ypd,t  ,wb1,ypd->gcount);
  yaoKeyCopy(wr,(bc?t:row));
  yaoKeyXor (row,t);
  yaoKeyXor (row,wa0);
  if(ac) yaoKeyXor(row,ypd->R);
  osend(pd,2,row,YAO_KEY_BYTES);
  if(rc) yaoKeyXor(wr ,ypd->R); // I could have just set inverted=true instead
  ypd->gcount++;

  // r may alias a and b, so modify at the end
  yaoKeyCopy(r->yao.w,wr);
  r->yao.inverted = false; r->unknown = true;
}
void npEvaluateGate(ProtocolDesc* pd, OblivBit* r,
    bool ac, bool bc, bool rc, const OblivBit* a, const OblivBit* b)
{
  YaoProtocolDesc* ypd = pd->extra;
  if(a->yao.value) ac=!ac;
  if(b->yao.value) bc=!bc;

  yao_key_t wr;
  // XXX serious question: why do I keep sending ypd->gcount separately?
  if(!bc) yaoSetHalfMask(ypd,r->yao.w,b->yao.w,ypd->gcount);
  else 
  { yaoSetHalfMask(ypd,wr,b->yao.w,ypd->gcount);
    yaoKeyXor(wr,a->yao.w);
    orecv(pd,1,r->yao.w,YAO_KEY_BYTES);
    yaoKeyXor(r->yao.w,wr);
  }

  ypd->gcount++;
  r->yao.value = ((ac&&bc) != rc);
  r->unknown = true; // known to me, but unknown to generator
}

void npGenerateAnd(ProtocolDesc* pd, OblivBit* r, 
                   const OblivBit* a, const OblivBit* b)
  { npGenerateGate(pd,r,0,0,0,a,b); }
void npGenerateOr (ProtocolDesc* pd, OblivBit* r, 
                   const OblivBit* a, const OblivBit* b)
  { npGenerateGate(pd,r,1,1,1,a,b); }
void npEvaluateAnd(ProtocolDesc* pd, OblivBit* r, 
                   const OblivBit* a, const OblivBit* b)
  { npEvaluateGate(pd,r,0,0,0,a,b); }
void npEvaluateOr (ProtocolDesc* pd, OblivBit* r, 
                   const OblivBit* a, const OblivBit* b)
  { npEvaluateGate(pd,r,1,1,1,a,b); }
void npSetBitXor (ProtocolDesc* pd, OblivBit* r,
                  const OblivBit* a, const OblivBit* b)
{
  OblivBit t;
  yaoKeyCopy(t.yao.w,a->yao.w);
  yaoKeyXor (t.yao.w,b->yao.w);
  t.yao.inverted = (a->yao.inverted!=b->yao.inverted);
  t.unknown=true;
  *r=t;
}
void npFlipBit(ProtocolDesc* pd, OblivBit* r)
  { r->yao.inverted=!r->yao.inverted; }
void npSetBitNot(ProtocolDesc* pd, OblivBit* r, const OblivBit* a)
  { *r=*a; npFlipBit(pd,r); }

// XXX Just implements privacy-free garbling, still need to do verify
static bool execNpProtocolAux(ProtocolDesc* pd, bool bcast1, 
                              protocol_run start, void* arg)
{
  int me = pd->thisParty;
  // XXX Hmm, much of setupYaoProtocol, mainYaoProtocol is redundant.
  setupYaoProtocol(pd,true);
  YaoProtocolDesc* ypd = pd->extra;
  NpProtocolExtra* npx = malloc(sizeof(NpProtocolExtra));
  ypd->extra=npx;
  npx->protoType = OC_YPD_TYPE_NP;
  npx->broadcast1 = bcast1;
  // override protocol methods
  pd->feedOblivInputs = (me==1?npGenrFeedOblivInputs:npEvalFeedOblivInputs);
  pd->revealOblivBits = (me==1?npGenrRevealOblivBits:npEvalRevealOblivBits);
  pd->setBitAnd = (me==1?npGenerateAnd:npEvaluateAnd);
  pd->setBitOr  = (me==1?npGenerateOr :npEvaluateOr );
  pd->setBitXor = npSetBitXor;
  pd->setBitNot = npSetBitNot;
  pd->flipBit   = npFlipBit;

  yaoUseNpot(pd,me);
  mainYaoProtocol(pd,false,start,arg);
  // XXX Preserve everything in mainYaoProtocol
  // Send out secrets: OTReveal, yao secrets
  // Whacky transport: <details>
  // Receiver threads out: <more>
  yaoReleaseOt(pd,me);
  free(npx);
  cleanupYaoProtocol(pd);

  return true; // success = not cheating (arg may have other outs)
}

bool execNpProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
  { return execNpProtocolAux(pd,false,start,arg); }
bool execNpProtocol_Bcast1(ProtocolDesc* pd, protocol_run start, void* arg)
  { return execNpProtocolAux(pd,true,start,arg); }
