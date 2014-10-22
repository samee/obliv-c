#pragma once
#include<bcrandom.h>
#include<obliv_types.h>

// every function here assumes dhRandomInit() has been called

#define COMMIT_HASH_ALGO GCRY_MD_SHA256
#define COMMIT_HASH_BYTES 32

// 64-bit key for commitment hash
#define COMMIT_KEY_SIZE 8

// Simply hash-based commitment
typedef struct 
{ const void* data;
  int n,destParty;
  char key[COMMIT_KEY_SIZE];
  ProtocolDesc* pd;
} OcCommitter;

// TODO global BCipherRandomGen needed here

// OcCommitter holds a pointer to data, which must not be freed
//   before ocRevealCommit(); Performs a fixed-size commitment
OcCommitter* ocSendCommit(ProtocolDesc* pd,BCipherRandomGen* gen, 
                           const void* data,int size,int destParty);
void ocRevealCommit(OcCommitter* com);

typedef struct {
  unsigned char hash[COMMIT_HASH_BYTES]; 
  ProtocolDesc* pd;
  int srcParty;
} OcCommitment;
OcCommitment* ocRecvCommit(ProtocolDesc* pd,BCipherRandomGen* gen,int srcParty);
// dest holds the revealed value previously committed, if dest is not NULL
bool ocCheckCommit(OcCommitment* com,int size,void* dest);

/* Escrowed exchange of n bytes of data. By "escrowed", we mean neither party
   should know what the other party is sending before sending out his own data.
   src: data that is sent; dest: data received
   Returns false iff the other party cheated (sent us bogus data after receiving
   data from our side).
   If party == pd->thisParty, simply performs a memcpy from dest to src
   */
bool ocXchgBytes(ProtocolDesc* pd,BCipherRandomGen* gen,
                 const void* src,void* dest,int n,int party);

/* Checks equality of n bytes of data. This is NOT a private equality check:
   both party will know what the other party sends. This simply performs an
   ocXchgBytes() followed by a comparison.
   */
bool ocEqualityCheck(ProtocolDesc* pd,BCipherRandomGen* gen,
                     const void* data,int n,int party);

/* A secure coin-tossing protocol, producing n bytes of random data, known
   to both parties. Both party simply generates random data, which is 
   exchanged in ocXcghBytes() and then xored. Both parties receive the same
   result.
   Returns false iff the other party cheated (might have forced the result to
   some particular value)
   */
bool ocRandomBytes(ProtocolDesc* pd,BCipherRandomGen* gen,
                   void* dest,int n,int party);

/* Privacy guaranteed. But if adversary claims data unequal, we believe 
   without proof. Claims of equality, on the other hand, are actually checked.
   So when we obtain "false", it could either mean unequal, or mean that the
   adversary is malicious and knows the true equality result --- this protocol
   can't tell the difference.
   */
bool ocPrivateEqualityCheck_halfAuth(ProtocolDesc* pd,BCipherRandomGen* gen,
    const void* data, int n,int party);
