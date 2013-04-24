#ifndef OBLIV_H
#define OBLIV_H
#include<obliv_bits.h>

#ifdef __GNUC__
#define inline __inline__
#endif

inline void setupOblivBool(OblivInputs* spec, OblivBit* dest, bool v)
  { __obliv_c__setupOblivBits(spec,dest,v,1); }
inline void setupOblivChar(OblivInputs* spec, OblivBit* dest, char v)
  { __obliv_c__setupOblivBits(spec,dest,v,bitsize(v)); }
inline void setupOblivInt(OblivInputs* spec, OblivBit* dest, int v)
  { __obliv_c__setupOblivBits(spec,dest,v,bitsize(v)); }
inline void setupOblivShort(OblivInputs* spec, OblivBit* dest, short v)
  { __obliv_c__setupOblivBits(spec,dest,v,bitsize(v)); }
inline void setupOblivLong(OblivInputs* spec, OblivBit* dest, long v)
  { __obliv_c__setupOblivBits(spec,dest,v,bitsize(v)); }
inline void setupOblivLLong(OblivInputs* spec, OblivBit* dest, long long v)
  { __obliv_c__setupOblivBits(spec,dest,v,bitsize(v)); }

inline void feedOblivInputs(OblivInputs* spec, size_t count, int party)
  { __obliv_c__feedOblivInputs(spec,count,party); }

inline bool revealOblivBool(OblivBit* src,int party)
  { return __obliv_c__revealOblivBool(src,party); }
inline char revealOblivChar(OblivBit* src,int party)
  { return (char)__obliv_c__revealOblivBits(src,bitsize(char),party); }
inline int revealOblivInt(OblivBit* src,int party)
  { return (int)__obliv_c__revealOblivBits(src,bitsize(int),party); }
inline short revealOblivShort(OblivBit* src,int party)
  { return (short)__obliv_c__revealOblivBits(src,bitsize(short),party); }
inline long revealOblivLong(OblivBit* src,int party)
  { return (long)__obliv_c__revealOblivBits(src,bitsize(long),party); }
inline long long revealOblivLLong(OblivBit* src,int party)
  { return (long long)__obliv_c__revealOblivBits(src,bitsize(long long)
                                                 ,party); }

#undef bitsize

#ifdef __GNUC__
#undef inline
#endif

#endif // OBLIV_H
