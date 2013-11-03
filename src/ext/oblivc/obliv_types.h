#ifndef OBLIV_TYPES_H
#define OBLIV_TYPES_H

#include<stddef.h>
#include<stdbool.h>
#include<stdint.h>  // uint64_t
#include<gcrypt.h>

typedef long long widest_t;

// These are the troublesome types that need to be included both in
//   user code and in obliv_bits.c. It is needed in obliv_bits.c because
//   the members are accessed directly there. It is needed in user code
//   because its size need to be known before the user can declare these
//   as local variables.

typedef struct ProtocolDesc ProtocolDesc;
typedef struct OblivInputs OblivInputs;
typedef struct OblivBit OblivBit;

// Checklist for adding new protocol:
//   Add a new entry in OblivBit union
//   Assign proper hooks to these callbacks in ProtocolDesc

// One extra bit for truth-table permutation
#define YAO_KEY_BITS 81
#define YAO_KEY_BYTES ((YAO_KEY_BITS+7)/8)
typedef char yao_key_t[YAO_KEY_BYTES];

struct ProtocolDesc {
  int partyCount, thisParty;
  struct ProtocolTransport* trans;
  union // a struct for each protocol-specific info
  { 
    struct { unsigned mulCount,xorCount; } debug;
  };

  void (*feedOblivInputs)(ProtocolDesc*,OblivInputs*,size_t,int);
  // Return value is true if the write was actually done
  bool (*revealOblivBits)(ProtocolDesc*,widest_t*,const OblivBit*,size_t,int);

  void (*setBitAnd)(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitOr )(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitXor)(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitNot)(ProtocolDesc*,OblivBit*,const OblivBit*);
  void (*flipBit  )(ProtocolDesc*,OblivBit*); // Sometimes avoids a struct copy
};

#define PROTOCOL_DESC(p) (&(p)->base)

typedef struct YaoProtocolDesc {
  struct ProtocolDesc base;
  yao_key_t R,I; // LSB of R needs to be 1
  uint64_t gcount;
  unsigned icount, ocount;
  void (*nonFreeGate)(struct YaoProtocolDesc*,OblivBit*,char,
      const OblivBit*,const OblivBit*);
  union { struct NpotSender* sender; struct NpotRecver* recver; };
} YaoProtocolDesc;

typedef struct DualexProtocolDesc {
  struct ProtocolDesc base;
} DualexProtocolDesc;

typedef struct ProtocolTransport ProtocolTransport;

// Channels are just our name for sockets (but we give it a new name since
//   at times we don't use tcp sockets).
struct ProtocolTransport {
  int maxParties, maxChannels, curChannel;
  ProtocolTransport* (*subtransport)(ProtocolTransport*,int);
  int (*send)(ProtocolTransport*,int,const void*,size_t);
  int (*recv)(ProtocolTransport*,int,      void*,size_t);
  void (*cleanup)(ProtocolTransport*);
};

struct OblivInputs {
  unsigned long long src;
  struct OblivBit* dest;
  size_t size;
};

// Type of the 'main' function being compiled in obliv-c
typedef void (*protocol_run)(void*);
#endif // OBLIV_TYPES_H
