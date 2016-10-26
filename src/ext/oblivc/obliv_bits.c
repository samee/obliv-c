// TODO I need to fix some int sizes
#include "obliv_transport.c"
#include "obliv_yao.c"
#include "obliv_nnob.c"
#include "obliv_debugProto.c"
#include <obliv_common.h>
#include <obliv_bits.h>
#include <commitReveal.h>
#include <nnob.h>
#include <assert.h>
#include <errno.h>      // libgcrypt needs ENOMEM definition
#include <inttypes.h>
#include <stdio.h>      // for protoUseStdio()
#include <string.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <gcrypt.h>


#ifndef CURRENT_PROTO
#define CURRENT_PROTO
// Right now, we do not support multiple protocols at the same time
static __thread ProtocolDesc *currentProto;
static inline bool known(const OblivBit* o) { return !o->unknown; }
#endif

void cleanupProtocol(ProtocolDesc* pd)
  { pd->trans->cleanup(pd->trans); }

void setCurrentParty(ProtocolDesc* pd, int party)
  { pd->thisParty=party; }

//-------------------- General Obliv functions --------------------------------

bool __obliv_c__revealOblivBits (widest_t* dest, const OblivBit* src
                                ,size_t size, int party)
  { return currentProto->revealOblivBits(currentProto,dest,src,size,party); }

int ocCurrentParty() { return currentProto->currentParty(currentProto); }
int ocCurrentPartyDefault(ProtocolDesc* pd) { return pd->thisParty; }

ProtocolDesc* ocCurrentProto() { return currentProto; }
void ocSetCurrentProto(ProtocolDesc* pd) { currentProto=pd; }

bool ocInDebugProto(void) { return ocCurrentProto()->extra==NULL; }

void __obliv_c__assignBitKnown(OblivBit* dest, bool value)
  { dest->knownValue = value; dest->unknown=false; }

void __obliv_c__copyBit(OblivBit* dest,const OblivBit* src)
  { if(dest!=src) *dest=*src; }

bool __obliv_c__bitIsKnown(bool* v,const OblivBit* bit)
{ if(known(bit)) *v=bit->knownValue;
  return known(bit);
}

void __obliv_c__setBitAnd(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(known(a) || known(b))
  { if(!known(a)) { const OblivBit* t=a; a=b; b=t; }
    if(a->knownValue) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,false);
  }else currentProto->setBitAnd(currentProto,dest,a,b);
}
void __obliv_c__setBitOr(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(known(a) || known(b))
  { if(!known(a)) { const OblivBit* t=a; a=b; b=t; }
    if(!a->knownValue) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,true);
  }else currentProto->setBitOr(currentProto,dest,a,b);
}
void __obliv_c__setBitXor(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  bool v;
  if(known(a) || known(b))
  { if(!known(a)) { const OblivBit* t=a; a=b; b=t; }
    v = a->knownValue;
    __obliv_c__copyBit(dest,b);
    if(v) __obliv_c__flipBit(dest);
  }else currentProto->setBitXor(currentProto,dest,a,b); 
}
void __obliv_c__setBitNot(OblivBit* dest,const OblivBit* a)
{ if(known(a)){ *dest=*a; dest->knownValue=!dest->knownValue; }
  else currentProto->setBitNot(currentProto,dest,a); 
}
void __obliv_c__flipBit(OblivBit* dest) 
{ if(known(dest)) dest->knownValue = !dest->knownValue;
  else currentProto->flipBit(currentProto,dest); 
}

void __obliv_c__setupOblivBits(OblivInputs* spec,OblivBit*  dest
                                     ,widest_t v,size_t size)
{ spec->dest=dest;
  spec->src=v;
  spec->size=size;
}

void __obliv_c__setSignedKnown
  (void* vdest, size_t size, long long signed value)
{
  OblivBit* dest=vdest;
  while(size-->0)
  { __obliv_c__assignBitKnown(dest,value&1);
    value>>=1; dest++;
  }
}
void __obliv_c__setUnsignedKnown
  (void* vdest, size_t size, long long unsigned value)
{
  OblivBit* dest=vdest;
  while(size-->0)
  { __obliv_c__assignBitKnown(dest,value&1);
    value>>=1; dest++;
  }
}
void __obliv_c__setBitsKnown(OblivBit* dest, const bool* value, size_t size)
  { while(size-->0) __obliv_c__assignBitKnown(dest++,value++); }
void __obliv_c__copyBits(OblivBit* dest, const OblivBit* src, size_t size)
  { if(dest!=src) while(size-->0) __obliv_c__copyBit(dest++,src++); }
bool __obliv_c__allBitsKnown(const OblivBit* bits, bool* dest, size_t size)
{ while(size-->0) if(!__obliv_c__bitIsKnown(dest++,bits++)) return false;
  return true;
}

void bitwiseOp(OblivBit* dest,const OblivBit* a,const OblivBit* b,size_t size
              ,void (*f)(OblivBit*,const OblivBit*,const OblivBit*))
  { while(size-->0) f(dest++,a++,b++); }


void __obliv_c__setBitwiseAnd (void* vdest
                              ,const void* vop1,const void* vop2
                              ,size_t size)
  { bitwiseOp(vdest,vop1,vop2,size,__obliv_c__setBitAnd); }

void __obliv_c__setBitwiseOr  (void* vdest
                              ,const void* vop1,const void* vop2
                              ,size_t size)
  { bitwiseOp(vdest,vop1,vop2,size,__obliv_c__setBitOr); }

void __obliv_c__setBitwiseXor (void* vdest
                              ,const void* vop1,const void* vop2
                              ,size_t size)
  { bitwiseOp(vdest,vop1,vop2,size,__obliv_c__setBitXor); }

void __obliv_c__setBitwiseNot (void* vdest,const void* vop,size_t size)
{ OblivBit *dest = vdest;
  const OblivBit *op = vop;
  while(size-->0) __obliv_c__setBitNot(dest++,op++); 
}

void __obliv_c__setBitwiseNotInPlace (void* vdest,size_t size)
{ OblivBit *dest=vdest; 
  while(size-->0) __obliv_c__flipBit(dest++); 
}

void __obliv_c__setLShift (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt)
{
  int i;
  OblivBit* dest=vdest;
  const OblivBit* src=vsrc;
  for(i=n-1;i>=(signed)shiftAmt;--i) __obliv_c__copyBit(dest+i,src+i-shiftAmt);
  for(;i>=0;--i) __obliv_c__assignBitKnown(dest+i,false);
}
void __obliv_c__setRShift (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt,bool isSigned)
{
  int i;
  OblivBit* dest=vdest;
  const OblivBit* src=vsrc;
  for(i=shiftAmt;i<n;++i) __obliv_c__copyBit(dest+i-shiftAmt,src+i);
  (isSigned?__obliv_c__setSignExtend
           :__obliv_c__setZeroExtend)(dest,n,dest,n-shiftAmt);
}
void __obliv_c__setRShiftSigned (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt)
  { __obliv_c__setRShift(vdest,vsrc,n,shiftAmt,true); }
void __obliv_c__setRShiftUnsigned (void* vdest, const void* vsrc, size_t n,
    unsigned shiftAmt)
  { __obliv_c__setRShift(vdest,vsrc,n,shiftAmt,false); }

// carryIn and/or carryOut can be NULL, in which case they are ignored
void __obliv_c__setBitsAdd (void* vdest,void* carryOut
                           ,const void* vop1,const void* vop2
                           ,const void* carryIn
                           ,size_t size)
{
  OblivBit carry,bxc,axc,t;
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  size_t skipLast;
  if(size==0)
  { if(carryIn && carryOut) __obliv_c__copyBit(carryOut,carryIn);
    return;
  }
  if(carryIn) __obliv_c__copyBit(&carry,carryIn);
  else __obliv_c__assignBitKnown(&carry,0);
  // skip AND on last bit if carryOut==NULL
  skipLast = (carryOut==NULL);
  while(size-->skipLast)
  { __obliv_c__setBitXor(&axc,op1,&carry);
    __obliv_c__setBitXor(&bxc,op2,&carry);
    __obliv_c__setBitXor(dest,op1,&bxc);
    __obliv_c__setBitAnd(&t,&axc,&bxc);
    __obliv_c__setBitXor(&carry,&carry,&t);
    ++dest; ++op1; ++op2;
  }
  if(carryOut) __obliv_c__copyBit(carryOut,&carry);
  else
  { __obliv_c__setBitXor(&axc,op1,&carry);
    __obliv_c__setBitXor(dest,&axc,op2);
  }
}

void __obliv_c__setPlainAdd (void* vdest
                            ,const void* vop1 ,const void* vop2
                            ,size_t size)
  { __obliv_c__setBitsAdd (vdest,NULL,vop1,vop2,NULL,size); }

void __obliv_c__setBitsSub (void* vdest, void* borrowOut
                           ,const void* vop1,const void* vop2
                           ,const void* borrowIn,size_t size)
{
  OblivBit borrow,bxc,bxa,t;
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  size_t skipLast;
  if(size==0)
  { if(borrowIn && borrowOut) __obliv_c__copyBit(borrowOut,borrowIn);
    return;
  }
  if(borrowIn) __obliv_c__copyBit(&borrow,borrowIn);
  else __obliv_c__assignBitKnown(&borrow,0);
  // skip AND on last bit if borrowOut==NULL
  skipLast = (borrowOut==NULL);
  while(size-->skipLast)
  { // c = borrow; a = op1; b=op2; borrow = (b+c)(b+a)+c
    __obliv_c__setBitXor(&bxa,op1,op2);
    __obliv_c__setBitXor(&bxc,&borrow,op2);
    __obliv_c__setBitXor(dest,&bxa,&borrow);
    __obliv_c__setBitAnd(&t,&bxa,&bxc);
    __obliv_c__setBitXor(&borrow,&borrow,&t);
    ++dest; ++op1; ++op2;
  }
  if(borrowOut) __obliv_c__copyBit(borrowOut,&borrow);
  else
  { __obliv_c__setBitXor(&bxa,op1,op2);
    __obliv_c__setBitXor(dest,&bxa,&borrow);
  }
}

void __obliv_c__setPlainSub (void* vdest
                            ,const void* vop1 ,const void* vop2
                            ,size_t size)
  { __obliv_c__setBitsSub (vdest,NULL,vop1,vop2,NULL,size); }

#define MAX_BITS (8*sizeof(widest_t))
// dest = c?src:0;
static void setZeroOrVal (OblivBit* dest
    ,const OblivBit* src ,size_t size ,const OblivBit* c)
{
  int i;
  for(i=0;i<size;++i) __obliv_c__setBitAnd(dest+i,src+i,c);
}
void __obliv_c__condNeg (const void* vcond, void* vdest
                        ,const void* vsrc, size_t n)
{
  int i;
  OblivBit c,t, *dest = vdest;
  const OblivBit *src = vsrc;
  __obliv_c__copyBit(&c,vcond);
  for(i=0;i<n-1;++i)
  { __obliv_c__setBitXor(dest+i,src+i,vcond);  // flip first
    __obliv_c__setBitXor(&t,dest+i,&c);        // then, conditional increment
    __obliv_c__setBitAnd(&c,&c,dest+i);
    __obliv_c__copyBit(dest+i,&t);
  }
  __obliv_c__setBitXor(dest+i,src+i,vcond);
  __obliv_c__setBitXor(dest+i,dest+i,&c);
}

// get absolute value and sign
static void setAbs (void* vdest, void* vsign, const void* vsrc, size_t n)
{
  __obliv_c__copyBit(vsign,((const OblivBit*)vsrc)+n-1);
  __obliv_c__condNeg(vsign,vdest,vsrc,n);
}

void __obliv_c__setNeg (void* vdest, const void* vsrc, size_t n)
{
  __obliv_c__condNeg(&__obliv_c__trueCond,vdest,vsrc,n);
}

void __obliv_c__setMul (void* vdest
                       ,const void* vop1 ,const void* vop2
                       ,size_t size)
{
  const OblivBit *op1=vop1, *op2=vop2;
  OblivBit temp[MAX_BITS]={},sum[MAX_BITS]={};
  int i;
  assert(size<=MAX_BITS);
  __obliv_c__setUnsignedKnown(sum,0,size);
  for(i=0;i<size;++i)
  { setZeroOrVal(temp,op1,size-i,op2+i);
    __obliv_c__setPlainAdd(sum+i,sum+i,temp,size-i);
  }
  __obliv_c__copyBits(vdest,sum,size);
}

// All parameters have equal number of bits
void __obliv_c__setDivModUnsigned (void* vquot, void* vrem
                                  ,const void* vop1, const void* vop2
                                  ,size_t n)
{
  const OblivBit *op1=vop1, *op2=vop2;
  OblivBit overflow[MAX_BITS]; // overflow[i] = does it overflow on op2<<i
  OblivBit temp[MAX_BITS],rem[MAX_BITS],quot[MAX_BITS],b;
  int i;
  assert(n<=MAX_BITS);
  __obliv_c__copyBits(rem,op1,n);
  __obliv_c__assignBitKnown(overflow,false);
  for(i=1;i<n;++i) __obliv_c__setBitOr(overflow+i,overflow+i-1,op2+n-i);
  for(i=n-1;i>=0;--i)
  {
    __obliv_c__setBitsSub(temp,&b,rem+i,op2,NULL,n-i);
    __obliv_c__setBitOr(&b,&b,overflow+i);
    __obliv_c__ifThenElse(rem+i,rem+i,temp,n-i,&b);
    __obliv_c__setBitNot(quot+i,&b);
  }
  if(vrem)  __obliv_c__copyBits(vrem,rem,n);
  if(vquot) __obliv_c__copyBits(vquot,quot,n);
}

void __obliv_c__setDivModSigned (void* vquot, void* vrem
                                ,const void* vop1, const void* vop2
                                ,size_t n)
{
  OblivBit neg1,neg2;
  OblivBit op1[MAX_BITS],op2[MAX_BITS];
  setAbs(op1,&neg1,vop1,n);
  setAbs(op2,&neg2,vop2,n);
  __obliv_c__setDivModUnsigned(vquot,vrem,op1,op2,n);
  __obliv_c__setBitXor(&neg2,&neg2,&neg1);
  if(vrem)  __obliv_c__condNeg(&neg1,vrem,vrem,n);
  if(vquot) __obliv_c__condNeg(&neg2,vquot,vquot,n);
}
void __obliv_c__setDivUnsigned (void* vdest
                               ,const void* vop1 ,const void* vop2
                               ,size_t n)
{
  __obliv_c__setDivModUnsigned(vdest,NULL,vop1,vop2,n);
}
void __obliv_c__setModUnsigned (void* vdest
                               ,const void* vop1 ,const void* vop2
                               ,size_t n)
{
  __obliv_c__setDivModUnsigned(NULL,vdest,vop1,vop2,n);
}
void __obliv_c__setDivSigned (void* vdest
                             ,const void* vop1 ,const void* vop2
                             ,size_t n)
{
  __obliv_c__setDivModSigned(vdest,NULL,vop1,vop2,n);
}
void __obliv_c__setModSigned (void* vdest
                             ,const void* vop1 ,const void* vop2
                             ,size_t n)
{
  __obliv_c__setDivModSigned(NULL,vdest,vop1,vop2,n);
}
void __obliv_c__setSignExtend (void* vdest, size_t dsize
                              ,const void* vsrc, size_t ssize)
{
  if(ssize>dsize) ssize=dsize;
  OblivBit *dest = vdest;
  __obliv_c__copyBits(vdest,vsrc,ssize);
  const OblivBit* s = ((const OblivBit*)vsrc)+ssize-1;
  dsize-=ssize;
  dest+=ssize;
  while(dsize-->0) __obliv_c__copyBit(dest++,s);
}
void __obliv_c__setZeroExtend (void* vdest, size_t dsize
                              ,const void* vsrc, size_t ssize)
{
  if(ssize>dsize) ssize=dsize;
  OblivBit *dest = vdest;
  __obliv_c__copyBits(vdest,vsrc,ssize);
  dsize-=ssize;
  dest+=ssize;
  while(dsize-->0) __obliv_c__assignBitKnown(dest++,0);
}

void __obliv_c__ifThenElse (void* vdest, const void* vtsrc
                           ,const void* vfsrc, size_t size
                           ,const void* vcond)
{
  // copying out vcond because it could be aliased by vdest
  OblivBit x,a,c=*(const OblivBit*)vcond;
  OblivBit *dest=vdest;
  const OblivBit *tsrc=vtsrc, *fsrc=vfsrc;
  while(size-->0)
  { __obliv_c__setBitXor(&x,tsrc,fsrc);
    __obliv_c__setBitAnd(&a,&c,&x);
    __obliv_c__setBitXor(dest,&a,fsrc);
    ++dest; ++fsrc; ++tsrc;
  }
}

void __obliv_c__condAssign (const void* cond,void* dest, const void* src, 
                            size_t size)
{
  __obliv_c__ifThenElse (dest,src,dest,size,cond);
}

// ltOut and ltIn may alias here
void __obliv_c__setLessThanUnit (OblivBit* ltOut
                                ,const OblivBit* op1, const OblivBit* op2
                                ,size_t size, const OblivBit* ltIn)
{
  // (a+b)(a+1)b + (a+b+1)c = b(a+1)+(a+b+1)c = ab+b+c+ac+bc = (a+b)(b+c)+c
  OblivBit t,x;
  __obliv_c__copyBit(ltOut,ltIn);
  while(size-->0)
  { __obliv_c__setBitXor(&x,op1,op2);
    __obliv_c__setBitXor(&t,op2,ltOut);
    __obliv_c__setBitAnd(&t,&t,&x);
    __obliv_c__setBitXor(ltOut,&t,ltOut);
    op1++; op2++;
  }
}

// assumes size >= 1
void __obliv_c__setLessThanSigned (void* vdest
                                  ,const void* vop1,const void* vop2
                                  ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1 = vop1, *op2 = vop2;
  __obliv_c__assignBitKnown(dest,0);
  __obliv_c__setLessThanUnit(dest,op1,op2,size-1,dest);
  __obliv_c__setLessThanUnit(dest,op2+size-1,op1+size-1,1,dest);
}

void __obliv_c__setLessOrEqualSigned (void* vdest
                                         ,const void* vop1, const void* vop2
                                         ,size_t size)
{
  __obliv_c__setLessThanSigned(vdest,vop2,vop1,size);
  __obliv_c__flipBit(vdest);
}

void __obliv_c__setLessThanUnsigned (void* vdest
                                    ,const void* vop1,const void* vop2
                                    ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1 = vop1, *op2 = vop2;
  __obliv_c__assignBitKnown(dest,0);
  __obliv_c__setLessThanUnit(dest,op1,op2,size,dest);
}

void __obliv_c__setLessOrEqualUnsigned (void* vdest
                                       ,const void* vop1, const void* vop2
                                       ,size_t size)
{
  __obliv_c__setLessThanUnsigned(vdest,vop2,vop1,size);
  __obliv_c__flipBit(vdest);
}

void __obliv_c__setEqualTo (void* vdest
                           ,const void* vop1,const void* vop2
                           ,size_t size)
{
  OblivBit *dest=vdest;
  const OblivBit *op1 = vop1, *op2 =  vop2;
  __obliv_c__setNotEqual(dest,op1,op2,size);
  __obliv_c__flipBit(dest);
}

void __obliv_c__setNotEqual (void* vdest
                            ,const void* vop1,const void* vop2
                            ,size_t size)
{
  OblivBit t;
  OblivBit *dest=vdest;
  const OblivBit *op1=vop1, *op2=vop2;
  __obliv_c__assignBitKnown(dest,0);
  while(size-->0)
  { __obliv_c__setBitXor(&t,op1++,op2++);
    __obliv_c__setBitOr(dest,dest,&t);
  }
}
void __obliv_c__setLogicalNot (void* vdest,const void* vop,size_t size)
{ OblivBit t;
  OblivBit *dest=vdest;
  const OblivBit *op=vop;
  __obliv_c__assignBitKnown(&t,0);
  while(size-->0) __obliv_c__setBitOr(&t,&t,op++);
  __obliv_c__setBitNot(dest,&t);
}

void __obliv_c__condAdd(const void* vc,void* vdest
                       ,const void* vx,size_t size)
{ OblivBit t[size];
  int i;
  for(i=0;i<size;++i) __obliv_c__setBitAnd(t+i,vc,((OblivBit*)vx)+i);
  __obliv_c__setBitsAdd(vdest,NULL,vdest,t,NULL,size);
}
void __obliv_c__condSub(const void* vc,void* vdest
                       ,const void* vx,size_t size)
{ OblivBit t[size];
  int i;
  for(i=0;i<size;++i) __obliv_c__setBitAnd(t+i,vc,((OblivBit*)vx)+i);
  __obliv_c__setBitsSub(vdest,NULL,vdest,t,NULL,size);
}

// ---- Translated versions of obliv.oh functions ----------------------

// TODO remove __obliv_c prefix and make these functions static/internal
void setupOblivBool(OblivInputs* spec, __obliv_c__bool* dest, bool v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,1); }
void setupOblivChar(OblivInputs* spec, __obliv_c__char* dest, char v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivInt(OblivInputs* spec, __obliv_c__int* dest, int v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivShort(OblivInputs* spec, __obliv_c__short* dest, short v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivLong(OblivInputs* spec, __obliv_c__long* dest, long v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivLLong(OblivInputs* spec, __obliv_c__lLong* dest, long long v)
  { __obliv_c__setupOblivBits(spec,dest->bits,v,__bitsize(v)); }
void setupOblivFloat(OblivInputs* spec, __obliv_c__float* dest, float v)
  {   OblivBit b;
      b.floatValue = v;
      dest->bits = b; }

void feedOblivInputs(OblivInputs* spec, size_t count, int party)
  { currentProto->feedOblivInputs(currentProto,spec,count,party); }

#define feedOblivFun(t,ot,tname) \
    __obliv_c__##ot feedObliv##tname (t v,int party) \
    { __obliv_c__##ot rv; \
      OblivInputs spec; \
      setupObliv##tname(&spec,&rv,v); \
      feedOblivInputs(&spec,1,party); \
      return rv; \
    }\
    void feedObliv##tname##Array(__obliv_c__##ot dest[],const t src[],size_t n,\
                             int party)\
    {\
      int i,p = protoCurrentParty(currentProto);\
      OblivInputs *specs = malloc(n*sizeof*specs);\
      for(i=0;i<n;++i) setupObliv##tname(specs+i,dest+i,p==party?src[i]:0);\
      feedOblivInputs(specs,n,party);\
      free(specs);\
    }

feedOblivFun(bool,bool,Bool)
feedOblivFun(char,char,Char)
feedOblivFun(short,short,Short)
feedOblivFun(int,int,Int)
feedOblivFun(long,long,Long)
feedOblivFun(long long,lLong,LLong)
__obliv_c__float feedOblivFloat(float v, int party)
{
    __obliv_c__float rv;
    OblivInputs spec;
    setupOblivFloat(&spec, &rv, v);
    return rv;
}

#undef feedOblivFun

// TODO pass const values by ref later
bool revealOblivBool(bool* dest,__obliv_c__bool src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,1,party)) 
    { *dest=(bool)wd; return true; }
  return false;
}
bool revealOblivChar(char* dest, __obliv_c__char src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,__bitsize(char),party)) 
    { *dest=(char)wd; return true; }
  return false;
}
bool revealOblivInt(int* dest, __obliv_c__int src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,__bitsize(int),party)) 
    { *dest=(int)wd; return true; }
  return false;
}
bool revealOblivShort(short* dest, __obliv_c__short src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,__bitsize(short),party)) 
    { *dest=(short)wd; return true; }
  return false;
}
bool revealOblivLong(long* dest, __obliv_c__long src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,__bitsize(long),party)) 
    { *dest=(long)wd; return true; }
  return false;
}
bool revealOblivLLong(long long* dest, __obliv_c__lLong src,int party)
{ widest_t wd;
  if(__obliv_c__revealOblivBits(&wd,src.bits,__bitsize(long long),party)) 
    { *dest=(long long)wd; return true; }
  return false;
}
bool revealOblivFloat(float *dest, __obliv_c__float src, int party)
{
    *dest = src.bits.floatValue;
    return true;
}

static void broadcastBits(int source,void* p,size_t n)
{
  int i;
  if(ocCurrentParty()!=source) orecv(currentProto,source,p,n);
  else for(i=1;i<=currentProto->partyCount;++i) if(i!=source)
      osend(currentProto,i,p,n);
}

// TODO fix data width
bool ocBroadcastBool(bool v,int source)
{
  char t = v;
  broadcastBits(source,&t,1);
  return t;
}
#define broadcastFun(t,tname)           \
  t ocBroadcast##tname(t v, int source)   \
  { broadcastBits(source,&v,sizeof(v)); \
    return v;                           \
  }
broadcastFun(char,Char)
broadcastFun(int,Int)
broadcastFun(short,Short)
broadcastFun(long,Long)
broadcastFun(long long,LLong)
#undef broadcastFun
