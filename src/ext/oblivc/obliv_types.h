#ifndef OBLIV_TYPES_H
#define OBLIV_TYPES_H

#include<stddef.h>

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

struct ProtocolDesc {
  // private fields, do not use directly
  int sock;
  int yaoCount,xorCount;
  int partyCount, thisParty;
  // Other state for OT, random keys etc.
  struct ProtocolTransport* trans;

  void (*feedOblivInputs)(ProtocolDesc*,OblivInputs*,size_t,int);
  widest_t (*revealOblivBits)(ProtocolDesc*,const OblivBit*,size_t,int);

  void (*setBitAnd)(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitOr )(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitXor)(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitNot)(ProtocolDesc*,OblivBit*,const OblivBit*);
  void (*flipBit  )(ProtocolDesc*,OblivBit*); // Avoids a struct copy
};

typedef struct ProtocolTransport {
  int (*send)(ProtocolDesc*,int,const void*,size_t);
  int (*recv)(ProtocolDesc*,int,      void*,size_t);
  void (*cleanup)(ProtocolDesc*);
} ProtocolTransport;

struct OblivInputs {
  // private fields, do not use directly
  unsigned long long src;
  struct OblivBit* dest;
  size_t size;
};

#endif // OBLIV_TYPES_H
