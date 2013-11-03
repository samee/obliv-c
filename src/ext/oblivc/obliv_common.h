#ifndef OBLIV_COMMON_H
#define OBLIV_COMMON_H

#include<obliv_types.h>

// Because I am evil and I do not like 
// Java-style redundant "say the type twice" practice
#define CAST(p) ((void*)p)

void gcryDefaultLibInit(void);

// Convenience functions
static inline int orecv(ProtocolDesc* pd,int s,void* p,size_t n)
  { return pd->trans->recv(pd->trans,s,p,n); }
static inline int osend(ProtocolDesc* pd,int d,void* p,size_t n)
  { return pd->trans->send(pd->trans,d,p,n); }

void dhRandomInit(void);
void dhRandomFinalize(void);

struct NpotSender* npotSenderNew(int nmax,ProtocolDesc* pd,int destParty);
void npotSenderRelease(struct NpotSender* s);
struct NpotRecver* npotRecverNew(int nmax,ProtocolDesc* pd,int srcParty);
void npotRecverRelease(struct NpotRecver* r);

void npotSend1Of2Once(struct NpotSender* s,char* opt0,char* opt1,int n,int len);
void npotRecv1Of2Once(struct NpotRecver* r,char* dest
                     ,unsigned mask,int n,int len);
#endif
