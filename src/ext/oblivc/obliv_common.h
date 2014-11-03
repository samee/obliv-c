#ifndef OBLIV_COMMON_H
#define OBLIV_COMMON_H

#include<obliv_types.h>

// Because I am evil and I do not like 
// Java-style redundant "say the type twice" practice
#define CAST(p) ((void*)p)

void gcryDefaultLibInit(void);

// Convenience functions
static inline int transSend(ProtocolTransport* t,int d,const void* p,size_t n)
  { return t->send(t,d,p,n); }
static inline int transRecv(ProtocolTransport* t,int s,void* p,size_t n)
  { return t->recv(t,s,p,n); }
static inline int osend(ProtocolDesc* pd,int d,const void* p,size_t n)
  { return transSend(pd->trans,d,p,n); }
static inline int orecv(ProtocolDesc* pd,int s,void* p,size_t n)
  { return transRecv(pd->trans,s,p,n); }

// Maybe these 5 lines should move to bcrandom.h
#define DHCurveName "secp192r1"
#define DHEltBits 192
#define DHEltSerialBytes (((DHEltBits+7)/8+2)*2)

void dhRandomInit(void);
void dhRandomFinalize(void);

static inline void otSenderRelease(OTsender* sender)
  { sender->release(sender->sender); }
static inline void otRecverRelease(OTrecver* recver)
  { recver->release(recver->recver); }

struct NpotSender* npotSenderNew(int nmax,ProtocolDesc* pd,int destParty);
void npotSenderRelease(struct NpotSender* s);
OTsender npotSenderAbstract(struct NpotSender* s);
struct NpotRecver* npotRecverNew(int nmax,ProtocolDesc* pd,int srcParty);
void npotRecverRelease(struct NpotRecver* r);
OTrecver npotRecverAbstract(struct NpotRecver* r);

void npotSend1Of2Once(struct NpotSender* s,const char* opt0,const char* opt1,
    int n,int len);
void npotRecv1Of2Once(struct NpotRecver* r,char* dest
                     ,unsigned mask,int n,int len);
void npotSend1Of2(struct NpotSender* s,const char* opt0,const char* opt1,
    int n,int len,int batchsize);
void npotRecv1Of2(struct NpotRecver* r,char* dest,const bool* sel,int n,int len,
    int batchsize);

struct HonestOTExtRecver* honestOTExtRecverNew(ProtocolDesc* pd,int srcparty);
void honestOTExtRecverRelease(struct HonestOTExtRecver* recver);
void honestOTExtRecv1Of2(struct HonestOTExtRecver* r,char* dest,const bool* sel,
    int n,int len);
OTrecver honestOTExtRecverAbstract(struct HonestOTExtRecver* r);

struct HonestOTExtSender* honestOTExtSenderNew(ProtocolDesc* pd,int destparty);
void honestOTExtSenderRelease(struct HonestOTExtSender* sender);
void honestOTExtSend1Of2(struct HonestOTExtSender* s,
    const char* opt0,const char* opt1,int n,int len);
OTsender honestOTExtSenderAbstract(struct HonestOTExtSender* s);

struct OTExtSender;
struct OTExtRecver;
struct OTExtRecver* otExtRecverNew(ProtocolDesc* pd,int srcparty);
struct OTExtRecver* otExtRecverNew_byPair(ProtocolDesc* pd,int srcparty);
void otExtRecverRelease(struct OTExtRecver* recver);
void otExtRecv1Of2(struct OTExtRecver* r,char* dest,const bool* sel,
    int n,int len);
OTrecver maliciousOTExtRecverAbstract(struct OTExtRecver* r);

struct OTExtSender* otExtSenderNew(ProtocolDesc* pd,int destparty);
struct OTExtSender* otExtSenderNew_byPair(ProtocolDesc* pd,int destparty);
void otExtSenderRelease(struct OTExtSender* sender);
void otExtSend1Of2(struct OTExtSender* s,
    const char* opt0,const char* opt1,int n,int len);
OTsender maliciousOTExtSenderAbstract(struct OTExtSender* s);

// Overrides ypd so that we are not using semi-honest OT
void yaoUseFullOTExt(ProtocolDesc* pd,int me);
void yaoUseNpot(ProtocolDesc* pd,int me);
void yaoReleaseOt(ProtocolDesc* pd,int me); // Used with yaoUseNpot

// setBit(a,i,v) == xorBit(a,i,v^getBit(a,i));
static inline void setBit(char *dest,int ind,bool v)
{ char mask = (1<<ind%8);
  dest[ind/8] = (dest[ind/8]&~mask)+(v?mask:0);
}
static inline bool getBit(const char* src,int ind) 
  { return src[ind/8]&(1<<ind%8); }
static inline void xorBit(char *dest,int ind,bool v) 
  { dest[ind/8]^=(v<<ind%8); }
static inline void memxor (void* dest, const void* src, size_t n)
{ size_t i;
  for(i=0;i+sizeof(uint64_t)<=n;i+=sizeof(uint64_t))
    *((uint64_t*)((char*)dest+i)) ^= *((uint64_t*)((char*)src+i));
  for(;i<n;++i) ((char*)dest)[i]^=((const char*)src)[i];
}


#endif
