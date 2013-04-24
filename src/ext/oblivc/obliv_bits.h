#ifndef OBLIV_BITS_H
#define OBLIV_BITS_H
#include<stddef.h> // size_t
#include<stdbool.h>
// This file gets included in every C intermediate file generated from obliv-C

// TODO make parameter orders more consistent: dest, src, size, cond etc.

// import common types
#include<obliv_types.h>

typedef long long widest_t;
typedef struct OblivBit {
  // private fields, do not use directly
  bool val;
  bool known;
  // would later have garbled keys, invert state etc.
} OblivBit;

// Type of the 'main' function being compiled in obliv-c
typedef void (*protocol_run)(void*);

#define bitsize(type) (8*sizeof(type))

typedef OblivBit __obliv_c__bool[1];
typedef OblivBit __obliv_c__char[bitsize(char)];
typedef OblivBit __obliv_c__int[bitsize(int)];
typedef OblivBit __obliv_c__short[bitsize(short)];
typedef OblivBit __obliv_c__long[bitsize(long)];
typedef OblivBit __obliv_c__lLong[bitsize(long long)];

// TODO protocol initialization functions
void setCurrentParty(ProtocolDesc* pd, int party);
void execOblivProtocol(ProtocolDesc* pd, protocol_run start, void* arg);

// None of the __obliv_c__* functions are meant to be used directly
//   in a normal C program, but rather through an obliv-c program.
void __obliv_c__assignBitKnown(OblivBit* dest, bool value);
void __obliv_c__copyBit(OblivBit* dest, const OblivBit* src);
bool __obliv_c__bitIsKnown(const OblivBit* bit,bool* val);

// Begin protocol-specific zone
// TODO error vars
void __obliv_c__setBitAnd(OblivBit* dest,const OblivBit* a,const OblivBit* b);
void __obliv_c__setBitOr(OblivBit* dest,const OblivBit* a,const OblivBit* b);
void __obliv_c__setBitXor(OblivBit* dest,const OblivBit* a,const OblivBit* b);
void __obliv_c__setBitNot(OblivBit* dest,const OblivBit* a);
void __obliv_c__flipBit(OblivBit* dest); // Avoids a struct copy

/*
// Obliv input/output
// Hate the inconsistency: it is Bit everywhere but here it is Bool
// feedOblivInput ignores its bool parameter if party != thisParty
// revealOblivOutput always returns false if party != thisParty
void __obliv_c__feedOblivBool(OblivBit* dest,int party,bool a);
void __obliv_c__feedOblivBits(OblivBit* dest,int party
                             ,const bool* src,size_t size);
void __obliv_c__setupOblivBits(OblivInputs* spec,OblivBit*  dest
                              ,widest_t v,size_t size);
void __obliv_c__feedOblivInputs(OblivInputs* spec,size_t count,int party);
// Output is simpler
bool __obliv_c__revealOblivBool(OblivBit* dest,int party);
widest_t __obliv_c__revealOblivBits(OblivBit* dest,size_t size,int party); 
  // cast out result
  */
// End protocol-specific zone

// Careful with this function: obliv things must be done in-sync by all parties
// Therefore actions in if(currentParty==me) {...} must not touch obliv data
//   This is not checked by the compiler in any way; you have been warned
int  __obliv_c__currentParty();

// Bitvector functions (these functions also work if dest and source point
//   to the same object).
// unconditional versions:
void __obliv_c__setSignedKnown
  (OblivBit* dest, size_t size,signed long long value);
void __obliv_c__setUnsignedKnown
  (OblivBit* dest, size_t size,unsigned long long value);
void __obliv_c__setBitsKnown(OblivBit* dest, const bool* value, size_t size);
void __obliv_c__copyBits(OblivBit* dest, const OblivBit* src, size_t size);
// allBitsKnown leave dest in an unspecified state if all bits are not known
bool __obliv_c__allBitsKnown(const OblivBit* bits, bool* dest, size_t size);

void __obliv_c__setBitwiseAnd (OblivBit* dest
                              ,const OblivBit* op1,const OblivBit* op2
                              ,size_t size);
void __obliv_c__setBitwiseOr  (OblivBit* dest
                              ,const OblivBit* op1,const OblivBit* op2
                              ,size_t size);
void __obliv_c__setBitwiseXor (OblivBit* dest
                              ,const OblivBit* op1,const OblivBit* op2
                              ,size_t size);
void __obliv_c__setBitwiseNot (OblivBit* dest,const OblivBit* op,size_t size);
void __obliv_c__setBitwiseNotInPlace (OblivBit* dest,size_t size);

// TODO reduced bit-width optimization. First, write a version that allows
//   user to do this manually in obliv-c. Then, promote ints to it automatically
// carryIn and/or carryOut can be NULL, in which case they are ignored
// dest and carryOut must be different objects. dest may alias with either
//   op1, op2, or both. carryIn may alias carryOut.
void __obliv_c__setBitsAdd (OblivBit* dest,OblivBit* carryOut
                           ,const OblivBit* op1,const OblivBit* op2
                           ,const OblivBit* carryIn
                           ,size_t size);
// Similar restrictions as setBitsAdd
void __obliv_c__setBitsSub (OblivBit* dest,OblivBit* borrowOut
                           ,const OblivBit* op1,const OblivBit* op2
                           ,const OblivBit* borrowIn,size_t size);
void __obliv_c__setSignExtend (OblivBit* dest, size_t dsize
                              ,const OblivBit* src, size_t ssize);
void __obliv_c__setZeroExtend (OblivBit* dest, size_t dsize
                              ,const OblivBit* src, size_t ssize);
void __obliv_c__ifThenElse (OblivBit* dest, const OblivBit* tsrc
                           ,const OblivBit* fsrc, size_t size
                           ,const OblivBit* cond);
void __obliv_c__setLessThanUnit (OblivBit* ltOut
                                ,const OblivBit* op1, const OblivBit* op2
                                ,size_t size, const OblivBit* ltIn);
void __obliv_c__setLessThan (OblivBit* dest
                            ,const OblivBit* op1,const OblivBit* op2
                            ,size_t size);
void __obliv_c__setEqualTo (OblivBit* dest
                           ,const OblivBit* op1,const OblivBit* op2
                           ,size_t size);
void __obliv_c__setNotEqual (OblivBit* dest
                            ,const OblivBit* op1,const OblivBit* op2
                            ,size_t size);

// Conditionals (TODO other operators) that may be faster at times
void __obliv_c__condAdd(const OblivBit* c,OblivBit* dest
                       ,const OblivBit* x,size_t size);

void __obliv_c__condSub(const OblivBit* c,OblivBit* dest
                       ,const OblivBit* x,size_t size);
#endif // OBLIV_BITS_H
