#include <obliv.h>

struct OblivBit {
  bool val;
  bool known;
  // would later have garbled keys, invert state etc.
};

// Right now, we do not support multiple protocols at the same time
ProtocolDesc currentProto;

void setCurrentParty(ProtocolDesc* pd, int party)
  { pd->thisParty=party; }

void execOblivProtocol(ProtocolDesc* pd, protocol_run start, void* arg)
{
  currentProto = *pd;
  currentProto.yaoCount = currentProto.xorCount = 0;
  start(arg);
}

void __obliv_c__assignBitKnown(OblivBit* dest, bool value)
  { dest->val = value; dest->known=true; }

bool __obliv_c__bitIsKnown(const OblivBit* bit,bool* v)
{ if(bit->known) *v=bit->val;
  return bit->known;
}

// TODO all sorts of identical parameter optimizations
// Implementation note: remember that all these pointers may alias each other
void __obliv_c__setBitAnd(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(a->known || b->known)
  { if(!a->known) { const OblivBit* t=a; a=b; b=t; }
    if(a->val) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,false);
  }
  else
  { dest->val = a->val && b->val;
    dest->known = false;
    currentProto.yaoCount++;
  }
}

void __obliv_c__setBitOr(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(a->known || b->known)
  { if(!a->known) { const OblivBit* t=a; a=b; b=t; }
    if(!a->val) __obliv_c__copyBit(dest,b);
    else __obliv_c__assignBitKnown(dest,true);
  }
  else
  { dest->val = a->val || b->val;
    dest->known = false;
    currentProto.yaoCount++;
  }
}
void __obliv_c__setBitXor(OblivBit* dest,const OblivBit* a,const OblivBit* b)
{
  if(a->known || b->known)
  { if(!a->known) { const OblivBit* t=a; a=b; b=t; }
    __obliv_c__copyBit(dest,b);
    if(a->val) __obliv_c__setBitNotInplace(dest);
  }
  else
  { dest->val = a->val != b->val;
    dest->known = false;
    currentProto.xorCount++;
  }
}
void __obliv_c__setBitNot(OblivBit* dest,const OblivBit* a)
{
  dest->val = !a->val;
  dest->known = a->known;
}
void __obliv_c__setBitNotInplace(OblivBit* dest) { dest->val = !dest->val; }

void __obliv_c__feedOblivBool(OblivBit* dest,int party,bool a)
{ if(party!=currentProto.thisParty) return;
  dest->known=false;
  dest->val=a;
}

bool __obliv_c__revealOblivBool(OblivBit* dest,int party)
{ if(party!=currentProto.thisParty) return false;
  else return dest->val;
}

int __obliv_c__currentParty() { return currentProto.thisParty; }

void __obliv_c__setSignedKnown
  (OblivBit* dest, size_t size, long long signed value)
{
  while(size--)
  { __obliv_c__assignBitKnown(dest,value&1);
    value>>=1; dest++;
  }
}
void __obliv_c__setUnsignedKnown
  (OblivBit* dest, size_t size, long long unsigned value)
{
  while(size--)
  { __obliv_c__assignBitKnown(dest,value&1);
    value>>=1; dest++;
  }
}
void __obliv_c__setBitsKnown(OblivBit* dest, const bool* value, size_t size)
  { while(size--) __obliv_c__assignBitKnown(dest++,value++); }
void __obliv_c__copyBits(OblivBit* dest, OblivBit* src, size_t size)
  { while(size--) __obliv_c__copyBit(dest++,src++); }
bool __obliv_c__allBitsKnown(const OblivBit* bits, bool* dest, size_t size)
{ while(size--) if(!__obliv_c__bitIsKnown(bits++,dest++)) return false;
  return true;
}

void bitwiseOp(OblivBit* dest,const OblivBit* a,const OblivBit* b,size_t size
              ,void (*f)(OblivBit*,const OblivBit*,const OblivBit*))
  { while(size--) f(dest++,a++,b++); }


void __obliv_c__setBitwiseAnd (OblivBit* dest
                              ,const OblivBit* op1,const OblivBit* op2
                              ,size_t size)
  { bitwiseOp(dest,op1,op2,size,__obliv_c__setBitAnd); }

void __obliv_c__setBitwiseOr  (OblivBit* dest
                              ,const OblivBit* op1,const OblivBit* op2
                              ,size_t size);
  { bitwiseOp(dest,op1,op2,size,__obliv_c__setBitOr); }

void __obliv_c__setBitwiseXor (OblivBit* dest
                              ,const OblivBit* op1,const OblivBit* op2
                              ,size_t size);
  { bitwiseOp(dest,op1,op2,size,__obliv_c__setBitXor); }

void __obliv_c__setBitwiseNot (OblivBit* dest,const OblivBit* op,size_t size)
  { while(size--) __obliv_c__setBitNot(dest++,op++); }

void __obliv_c__setBitwiseNotInPlace (OblivBit* dest,size_t size)
  { while(size--) __obliv_c__setBitNotInPlace(dest++); }
