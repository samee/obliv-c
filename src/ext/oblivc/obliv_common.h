#ifndef OBLIV_COMMON_H
#define OBLIV_COMMON_H

#include<obliv_types.h>

void gcryDefaultLibInit(void);

// Convenience functions
static inline int orecv(ProtocolDesc* pd,int s,void* p,size_t n)
  { return pd->trans->recv(pd,s,0,p,n); }
static inline int osend(ProtocolDesc* pd,int d,void* p,size_t n)
  { return pd->trans->send(pd,d,0,p,n); }

// Send on a particular channel, in case many exist
static inline int orecvc(ProtocolDesc* pd,int s,int c,void* p,size_t n)
  { return pd->trans->recv(pd,s,c,p,n); }
static inline int osendc(ProtocolDesc* pd,int d,int c,void* p,size_t n)
  { return pd->trans->send(pd,d,c,p,n); }

void dhRandomInit(void);
void dhRandomFinalize(void);

struct NpotSender* npotSenderNew(int nmax,ProtocolDesc* pd,int destParty);
void npotSenderRelease(struct NpotSender* s);
struct NpotRecver* npotRecverNew(int nmax,ProtocolDesc* pd,int srcParty);
void npotRecverRelease(struct NpotRecver* r);

#endif
