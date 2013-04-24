#ifndef OBLIV_TYPES_H
#define OBLIV_TYPES_H


// These are the troublesome types that need to be included both in
//   user code and in obliv_bits.c. It is needed in obliv_bits.c because
//   the members are accessed directly there. It is needed in user code
//   because its size need to be known before the user can declare these
//   as local variables.

typedef struct {
  // private fields, do not use directly
  int sock;
  int yaoCount,xorCount;
  int partyCount, thisParty;
  // Other state for OT, random keys etc.
} ProtocolDesc;

typedef struct OblivInputs {
  // private fields, do not use directly
  unsigned long long src;
  struct OblivBit* dest;
  size_t size;
} OblivInputs;

#endif // OBLIV_TYPES_H
