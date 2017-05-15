#ifndef OBLIV_H
#define OBLIV_H

#include<obliv_types.h>
#include<obliv_psi.h>

// This header should only have functions directly used by the user,
//   not functions that will appear in the generated C code.
//   Moreover, including this function in an obliv-C file should still
//   keep it a valid obliv-C file after preprocessing. So e.g., no magic
//   conversion from obliv int to OblivBits* inside inline functions.
//   In fact, user code should never be aware of OblivBits type.
void protocolUseStdio(ProtocolDesc*);
void protocolUseTcp2P(ProtocolDesc* pd,int sock,bool isClient);
void protocolUseTcp2PProfiled(ProtocolDesc* pd,int sock,bool isClient);
void protocolUseTcp2PKeepAlive(ProtocolDesc* pd,int sock,bool isClient);
void protocolAddSizeCheck(ProtocolDesc* pd);
// The old sockCount parameter (was the last param) is no longer used.
int protocolConnectTcp2P(ProtocolDesc* pd,const char* server,const char* port);
int protocolAcceptTcp2P(ProtocolDesc* pd,const char* port);
int protocolConnectTcp2PProfiled(ProtocolDesc* pd,const char* server,const char* port);
int protocolAcceptTcp2PProfiled(ProtocolDesc* pd,const char* port);
void cleanupProtocol(ProtocolDesc*);

void setCurrentParty(ProtocolDesc* pd, int party);
void execDebugProtocol(ProtocolDesc* pd, protocol_run start, void* arg);
void execFloatProtocol(ProtocolDesc* pd, protocol_run start, void* arg);
void execNetworkStressProtocol(ProtocolDesc* pd, int bytecount,
                               protocol_run start, void* arg);
void execYaoProtocol(ProtocolDesc* pd, protocol_run start, void* arg);
void execYaoProtocol_noHalf(ProtocolDesc* pd, protocol_run start, void* arg);
bool execDualexProtocol(ProtocolDesc* pd, protocol_run start, void* arg);
bool execNpProtocol(ProtocolDesc* pd, protocol_run start, void* arg);
bool execNpProtocol_Bcast1(ProtocolDesc* pd, protocol_run start, void* arg);

void execNnobProtocol(ProtocolDesc* pd, protocol_run start, void* arg, int numOTs, bool useAltOTExt);

size_t tcp2PBytesSent(ProtocolDesc* pd);
size_t tcp2PFlushCount(ProtocolDesc* pd);

uint64_t yaoGateCount(void);

#endif // OBLIV_H
