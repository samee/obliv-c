#ifndef OBLIV_TYPES_H
#define OBLIV_TYPES_H

#include<stddef.h>
#include<stdbool.h>
#include<stdint.h>  // uint64_t
#include<gcrypt.h>

#define OC_ERROR_NONE 0
#define OC_ERROR_OT_EXTENSION -1

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

#define YAO_KEY_BITS 80
#define YAO_KEY_BYTES ((YAO_KEY_BITS+7)/8)
#if YAO_KEY_BITS!=(YAO_KEY_BYTES*8)
#error "Yao key size needs to be a multiple of 8 bits"
#endif

typedef char yao_key_t[YAO_KEY_BYTES];

#define NNOB_KEY_BYTES 10 
typedef char nnob_key_t[NNOB_KEY_BYTES];
typedef struct NnobKey {
	nnob_key_t key;
} NnobKey;
typedef struct NnobShareAndMac {
	bool share;
	nnob_key_t mac;
} NnobShareAndMac;

struct ProtocolDesc {
  int partyCount, thisParty, error;
  struct ProtocolTransport* trans;
  union // a struct for each protocol-specific info
  { 
    struct { unsigned mulCount,xorCount; } debug;
  };

  int (*currentParty)(ProtocolDesc*);
  void (*feedOblivInputs)(ProtocolDesc*,OblivInputs*,size_t,int);
  // Return value is true if the write was actually done
  bool (*revealOblivBits)(ProtocolDesc*,widest_t*,const OblivBit*,size_t,int);

  void (*setBitAnd)(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitOr )(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitXor)(ProtocolDesc*,OblivBit*,const OblivBit*,const OblivBit*);
  void (*setBitNot)(ProtocolDesc*,OblivBit*,const OblivBit*);
  void (*flipBit  )(ProtocolDesc*,OblivBit*); // Sometimes avoids a struct copy

  void* extra;  // protocol-specific information
                // First field should be char protoType
};

#define OC_DYN_EXTRA_FUN(fname,Type1,Type2,type2Id)    \
  Type2* fname(Type1* s1)                              \
  { if(*(char*)s1->extra==(type2Id)) return s1->extra; \
    else return NULL;                                  \
  }

#define NPOT_BATCH_SIZE 7
typedef struct {
  void* sender;
  void (*send)(void*,const char*,const char*,int n,int len);
  void (*release)(void*);
} OTsender;

typedef struct {
  void* recver;
  void (*recv)(void*,char*,const bool*,int n,int len);
  void (*release)(void*);
} OTrecver;

typedef void (*OcOtCorrelator)(char*,const char*,int,void*);
typedef struct {
  void* sender;
  void (*send)(void*,char*,char*,int n,int len,OcOtCorrelator,void*);
  void (*release)(void*);
} COTsender;

typedef OTrecver COTrecver; // Strong typedef would have been nice

/* Somehow I have settled on this "allocation-on-initialization"
   convention that I never planned. It simplifies user code a bit
   (i.e. user does not have to upcast every time execProtocol
   or protocolUseStdio is used). On the other hand, downcasts
   require several pointer lookups in the place of a single,
   simple cast. This should have been avoidable, without affecting
   user code. TODO
*/
typedef struct YaoProtocolDesc {
  char protoType;
  yao_key_t R,I; // LSB of R needs to be 1
  uint64_t gcount;
  unsigned icount, ocount;
  void (*nonFreeGate)(struct ProtocolDesc*,OblivBit*,char,
      const OblivBit*,const OblivBit*);
  union { OTsender sender; OTrecver recver; };
  gcry_cipher_hd_t fixedKeyCipher;
  void* extra;
} YaoProtocolDesc;

#define OC_PD_TYPE_YAO 1
static inline OC_DYN_EXTRA_FUN(protoYaoProtocolDesc,ProtocolDesc,
                               YaoProtocolDesc,OC_PD_TYPE_YAO)
typedef struct ProtocolTransport ProtocolTransport;

// Channels are just our name for sockets (but we give it a new name since
//   at times we don't use tcp sockets).
struct ProtocolTransport {
  int maxParties;
  ProtocolTransport* (*split)(ProtocolTransport*);
  int (*send)(ProtocolTransport*,int,const void*,size_t);
  int (*recv)(ProtocolTransport*,int,      void*,size_t);
  void (*cleanup)(ProtocolTransport*);
};

struct OblivInputs {
  union {
    unsigned long long src;
    float src_f;
  };
  struct OblivBit* dest;
  size_t size;
};

// Type of the 'main' function being compiled in obliv-c
typedef void (*protocol_run)(void*);
#endif // OBLIV_TYPES_H
