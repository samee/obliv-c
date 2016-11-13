/*------------------------------------------------------------------------
 / OCB Version 3 Reference Code (Optimized C)     Last modified 08-SEP-2012
 /-------------------------------------------------------------------------
 / Copyright (c) 2012 Ted Krovetz.
 /
 / Permission to use, copy, modify, and/or distribute this software for any
 / purpose with or without fee is hereby granted, provided that the above
 / copyright notice and this permission notice appear in all copies.
 /
 / THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 / WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 / MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 / ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 / WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 / ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 / OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 /
 / Phillip Rogaway holds patents relevant to OCB. See the following for
 / his patent grant: http://www.cs.ucdavis.edu/~rogaway/ocb/grant.htm
 /
 / Special thanks to Keegan McAllister for suggesting several good improvements
 /
 / Comments are welcome: Ted Krovetz <ted@krovetz.net> - Dedicated to Laurel K
 /------------------------------------------------------------------------- */
/*
 This file is part of JustGarble.

 JustGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 JustGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

 */
/*
 This file is part of TinyGarble. It is modified version of JustGarble
 under GNU license.

 TinyGarble is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TinyGarble is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TinyGarble.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRYPTO_BLOCK_H_
#define CRYPTO_BLOCK_H_

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

typedef __m128i block;
#define get_LSB(x) (*((unsigned short *)&x)&1)
#define XorBlock(x,y) _mm_xor_si128((x),(y))
#define ZeroBlock() _mm_setzero_si128()
#define CmpBlock(X, Y) (_mm_extract_epi16((_mm_cmpeq_epi64((X), (Y))),0)==0xffff)
#define MakeBlock(X,Y) _mm_set_epi64((__m64)(X), (__m64)(Y))
#define DoubleBlock(B) _mm_slli_epi64(B,1)


static inline block Double_Block(block bl) {
  const __m128i mask = _mm_set_epi32(135, 1, 1, 1);
  __m128i tmp = _mm_srai_epi32(bl, 31);
  tmp = _mm_and_si128(tmp, mask);
  tmp = _mm_shuffle_epi32(tmp, _MM_SHUFFLE(2, 1, 0, 3));
  bl = _mm_slli_epi32(bl, 1);
  return _mm_xor_si128(bl, tmp);
}
static inline block SlowDouble_Block(block bl) {
  int i;
  __m128i tmp = _mm_srai_epi32(bl, 31);
  for (i = 0; i < 1; i++) {
    const __m128i mask = _mm_set_epi32(135, 1, 1, 1);
    tmp = _mm_and_si128(tmp, mask);
    tmp = _mm_shuffle_epi32(tmp, _MM_SHUFFLE(2, 1, 0, 3));
    bl = _mm_slli_epi32(bl, 1);
  }
  return _mm_xor_si128(bl, tmp);
}

static inline block LeftShift(block bl) {
  const __m128i mask = _mm_set_epi32(0, 0, (1 << 31), 0);
  __m128i tmp = _mm_and_si128(bl, mask);
  bl = _mm_slli_epi64(bl, 1);
  return _mm_xor_si128(bl, tmp);
}

static inline block RightShift(block bl) {
  const __m128i mask = _mm_set_epi32(0, 1, 0, 0);
  __m128i tmp = _mm_and_si128(bl, mask);
  bl = _mm_slli_epi64(bl, 1);
  return _mm_xor_si128(bl, tmp);
}

#define ADD128(out, in1, in2)                      \
       __asm__("addq %2, %0; adcq %3, %1" :           \
                         "=r"(out.lo64), "=r"(out.hi64) :       \
                         "emr" (in2.lo64), "emr"(in2.hi64),     \
                         "0" (in1.lo64), "1" (in1.hi64));

#define _m128_switch_endian(x) \
  _mm_shuffle_epi8((x), _mm_set_epi8(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15))

#endif /* CRYPTO_BLOCK_H_ */
