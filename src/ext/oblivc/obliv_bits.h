#ifndef OBLIV_BITS_H
#define OBLIV_BITS_H
#define __oblivious_c

//void* memset(void* s, int c, size_t n); // Hack, had to declare memset
#include<string.h> // memset to zero
#include<stdbool.h>
// This file gets included in every C intermediate file generated from obliv-C

// import common types
#include<obliv_types_internal.h>
#include<obliv_common.h>

#define __bitsize(type) (8*sizeof(type))

// Note: these structs are often freely casted to and from OblivBit* and void*
//   Works great in C, but may fail if we add more fields here.
typedef struct { OblivBit bits[1]; } __obliv_c__bool;
typedef struct { OblivBit bits[__bitsize(char)];  } __obliv_c__char;
typedef struct { OblivBit bits[__bitsize(int)];   } __obliv_c__int;
typedef struct { OblivBit bits[__bitsize(short)]; } __obliv_c__short;
typedef struct { OblivBit bits[__bitsize(long)];  } __obliv_c__long;
typedef struct { OblivBit bits[__bitsize(long long)]; } __obliv_c__lLong;
typedef struct { OblivBit bits; } __obliv_c__float;

// Just a cast
static inline OblivBit* __obliv_c__bits(void* x) { return x; }

#define ocBitSize(type) (sizeof(type)/sizeof(__obliv_c__bool))

static const __obliv_c__bool __obliv_c__trueCond = {{{false,{true}}}};

// None of the __obliv_c__* functions are meant to be used directly
//   in a normal C program, but rather through an obliv-c program.
void __obliv_c__assignBitKnown(OblivBit* dest, bool value);
void __obliv_c__copyBit(OblivBit* dest, const OblivBit* src);
bool __obliv_c__bitIsKnown(bool* val,const OblivBit* bit);

// TODO error vars
void __obliv_c__setBitAnd(OblivBit* dest,const OblivBit* a,const OblivBit* b);
void __obliv_c__setBitOr(OblivBit* dest,const OblivBit* a,const OblivBit* b);
void __obliv_c__setBitXor(OblivBit* dest,const OblivBit* a,const OblivBit* b);
void __obliv_c__setBitNot(OblivBit* dest,const OblivBit* a);
void __obliv_c__flipBit(OblivBit* dest); // Avoids a struct copy
void __obliv_c__floatAdd(OblivBit* dest,const OblivBit* a,const OblivBit* b);

// Bitvector functions (these functions also work if dest and source point
//   to the same object).
// unconditional versions:
void __obliv_c__setSignedKnown
  (void * dest, size_t size,signed long long value);
void __obliv_c__setUnsignedKnown
  (void * dest, size_t size,unsigned long long value);
void __obliv_c__setBitsKnown(OblivBit* dest, const bool* value, size_t size);
void __obliv_c__copyBits(OblivBit* dest, const OblivBit* src, size_t size);
// allBitsKnown leave dest in an unspecified state if all bits are not known
bool __obliv_c__allBitsKnown(const OblivBit* bits, bool* dest, size_t size);

void __obliv_c__setBitwiseAnd (void* dest
                              ,const void* op1,const void* op2
                              ,size_t size);
void __obliv_c__setBitwiseOr  (void* dest
                              ,const void* op1,const void* op2
                              ,size_t size);
void __obliv_c__setBitwiseXor (void* dest
                              ,const void* op1,const void* op2
                              ,size_t size);
void __obliv_c__setBitwiseNot (void* dest,const void* op,size_t size);
void __obliv_c__setBitwiseNotInPlace (void* dest,size_t size);
void __obliv_c__setLShift (void* vdest, const void* vsrc, size_t size,
    unsigned shiftAmt);
void __obliv_c__setRShiftSigned (void* vdest, const void* vsrc, size_t size,
    unsigned shiftAmt);
void __obliv_c__setRShiftUnsigned (void* vdest, const void* vsrc, size_t size,
    unsigned shiftAmt);
void __obliv_c__setRShift (void* vdest, const void* vsrc, size_t size,
    unsigned shiftAmt,bool isSigned);

// TODO reduced bit-width optimization. First, write a version that allows
//   user to do this manually in obliv-c. Then, promote ints to it automatically
// carryIn and/or carryOut can be NULL, in which case they are ignored
// dest and carryOut must be different objects. dest may alias with either
//   op1, op2, or both. carryIn may alias carryOut.
void __obliv_c__setPlainAdd (void* vdest
                            ,const void* vop1 ,const void* vop2
                            ,size_t size);
void __obliv_c__setPlainSub (void* vdest
                            ,const void* vop1 ,const void* vop2
                            ,size_t size);
void __obliv_c__setBitsAdd (void* dest,void* carryOut
                           ,const void* op1,const void* op2
                           ,const void* carryIn
                           ,size_t size);
void __obliv_c__setNeg (void* vdest, const void* vsrc, size_t n);
void __obliv_c__condNeg (const void* vcond, void* vdest
                        ,const void* vsrc, size_t n);
// Fun fact: product of n-bit numbers, when truncated to n-bits, is the 
//   same whether the numbers are signed or unsigned
void __obliv_c__setMul (void* vdest
                       ,const void* vop1 ,const void* vop2
                       ,size_t size);
void __obliv_c__setDivModUnsigned (void* vquot, void* vrem
                                  ,const void* vop1, const void* vop2
                                  ,size_t size);
void __obliv_c__setDivModSigned (void* vquot, void* vrem
                                ,const void* vop1, const void* vop2
                                ,size_t size);
void __obliv_c__setDivUnsigned (void* vdest
                               ,const void* vop1 ,const void* vop2
                               ,size_t size);
void __obliv_c__setModUnsigned (void* vdest
                               ,const void* vop1 ,const void* vop2
                               ,size_t size);
void __obliv_c__setDivSigned (void* vdest
                             ,const void* vop1 ,const void* vop2
                             ,size_t size);
void __obliv_c__setModSigned (void* vdest
                             ,const void* vop1 ,const void* vop2
                             ,size_t size);
// Similar restrictions as setBitsAdd
void __obliv_c__setBitsSub (void* dest,void* borrowOut
                           ,const void* op1,const void* op2
                           ,const void* borrowIn,size_t size);
void __obliv_c__setSignExtend (void* dest, size_t dsize
                              ,const void* src, size_t ssize);
void __obliv_c__setZeroExtend (void* dest, size_t dsize
                              ,const void* src, size_t ssize);
void __obliv_c__ifThenElse (void* dest, const void* tsrc
                           ,const void* fsrc, size_t size
                           ,const void* cond);
void __obliv_c__setLessThanUnit (OblivBit* ltOut
                                ,const OblivBit* op1, const OblivBit* op2
                                ,size_t size, const OblivBit* ltIn);
void __obliv_c__setLessThanUnsigned (void* dest
                                    ,const void* op1,const void* op2
                                    ,size_t size);
void __obliv_c__setLessOrEqualUnsigned (void* dest
                                       ,const void* op1,const void* op2
                                       ,size_t size);
void __obliv_c__setLessThanSigned (void* dest
                                  ,const void* op1,const void* op2
                                  ,size_t size);
void __obliv_c__setLessOrEqualSigned (void* dest
                                     ,const void* op1,const void* op2
                                     ,size_t size);
void __obliv_c__setEqualTo (void* dest
                           ,const void* op1,const void* op2
                           ,size_t size);
void __obliv_c__setNotEqual (void* dest
                            ,const void* op1,const void* op2
                            ,size_t size);
void __obliv_c__setLogicalNot (void* dest,const void* op,size_t size);

void __obliv_c__condAssign (const void* cond, void* dest, const void* src, 
                            size_t size);
// Conditionals (TODO other operators) that may be faster at times
static inline 
void __obliv_c__condAssignKnown(const void* cond, void* dest, size_t size
                               ,widest_t val)
{
  OblivBit ov[__bitsize(widest_t)];
  __obliv_c__setSignedKnown(ov,size,val);
  __obliv_c__ifThenElse(dest,ov,dest,size,cond);
}

// TODO condIncr, condDecr
void __obliv_c__condAdd(const void* c,void* dest
                       ,const void* x,size_t size);

void __obliv_c__condSub(const void* c,void* dest
                       ,const void* x,size_t size);

// Special bit, that tells the compiler to start inserting memsets
// This way we don't insert memsets into header files, specially before
// memset has even been defined. The value of this variable doesn't matter, 
// only the fact that it's here
static const int __obliv_c__enable_zero_out = 1;
#endif // OBLIV_BITS_H
