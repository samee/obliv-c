/* ====================================================================
 * Copyright (c) 1998-2002 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
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

#ifndef CRYPTO_AES_H_
#define CRYPTO_AES_H_

#include <xmmintrin.h>              /* SSE instructions and _mm_malloc */
#include <emmintrin.h>              /* SSE2 instructions               */
#include <wmmintrin.h>

typedef struct {
  __m128i rd_key[15];
  int rounds;
} AES_KEY;
#define ROUNDS(ctx) ((ctx)->rounds)

#define EXPAND_ASSIST(v1,v2,v3,v4,shuff_const,aes_const)                    \
    v2 = _mm_aeskeygenassist_si128(v4,aes_const);                           \
    v3 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v3),              \
                                         _mm_castsi128_ps(v1), 16));        \
    v1 = _mm_xor_si128(v1,v3);                                              \
    v3 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v3),              \
                                         _mm_castsi128_ps(v1), 140));       \
    v1 = _mm_xor_si128(v1,v3);                                              \
    v2 = _mm_shuffle_epi32(v2,shuff_const);                                 \
    v1 = _mm_xor_si128(v1,v2)

#define EXPAND192_STEP(idx,aes_const)                                       \
    EXPAND_ASSIST(x0,x1,x2,x3,85,aes_const);                                \
    x3 = _mm_xor_si128(x3,_mm_slli_si128 (x3, 4));                          \
    x3 = _mm_xor_si128(x3,_mm_shuffle_epi32(x0, 255));                      \
    kp[idx] = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(tmp),        \
                                              _mm_castsi128_ps(x0), 68));   \
    kp[idx+1] = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(x0),       \
                                                _mm_castsi128_ps(x3), 78)); \
    EXPAND_ASSIST(x0,x1,x2,x3,85,(aes_const*2));                            \
    x3 = _mm_xor_si128(x3,_mm_slli_si128 (x3, 4));                          \
    x3 = _mm_xor_si128(x3,_mm_shuffle_epi32(x0, 255));                      \
    kp[idx+2] = x0; tmp = x3

inline void AES128KeyExpansion(const unsigned char *userkey, void *key) {
  __m128i x0, x1, x2;
  __m128i *kp = (__m128i *) key;
  kp[0] = x0 = _mm_loadu_si128((__m128i *) userkey);
  x2 = _mm_setzero_si128();
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 1);
  kp[1] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 2);
  kp[2] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 4);
  kp[3] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 8);
  kp[4] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 16);
  kp[5] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 32);
  kp[6] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 64);
  kp[7] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 128);
  kp[8] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 27);
  kp[9] = x0;
  EXPAND_ASSIST(x0, x1, x2, x0, 255, 54);
  kp[10] = x0;
}

inline void AES192KeyExpansion(const unsigned char *userkey, void *key) {
  __m128i x0, x1, x2, x3, tmp, *kp = (__m128i *) key;
  kp[0] = x0 = _mm_loadu_si128((__m128i *) userkey);
  tmp = x3 = _mm_loadu_si128((__m128i *) (userkey + 16));
  x2 = _mm_setzero_si128();
  EXPAND192_STEP(1, 1);
  EXPAND192_STEP(4, 4);
  EXPAND192_STEP(7, 16);
  EXPAND192_STEP(10, 64);
}

inline void AES256KeyExpansion(const unsigned char *userkey, void *key) {
  __m128i x0, x1, x2, x3, *kp = (__m128i *) key;
  kp[0] = x0 = _mm_loadu_si128((__m128i *) userkey);
  kp[1] = x3 = _mm_loadu_si128((__m128i *) (userkey + 16));
  x2 = _mm_setzero_si128();
  EXPAND_ASSIST(x0, x1, x2, x3, 255, 1);
  kp[2] = x0;
  EXPAND_ASSIST(x3, x1, x2, x0, 170, 1);
  kp[3] = x3;
  EXPAND_ASSIST(x0, x1, x2, x3, 255, 2);
  kp[4] = x0;
  EXPAND_ASSIST(x3, x1, x2, x0, 170, 2);
  kp[5] = x3;
  EXPAND_ASSIST(x0, x1, x2, x3, 255, 4);
  kp[6] = x0;
  EXPAND_ASSIST(x3, x1, x2, x0, 170, 4);
  kp[7] = x3;
  EXPAND_ASSIST(x0, x1, x2, x3, 255, 8);
  kp[8] = x0;
  EXPAND_ASSIST(x3, x1, x2, x0, 170, 8);
  kp[9] = x3;
  EXPAND_ASSIST(x0, x1, x2, x3, 255, 16);
  kp[10] = x0;
  EXPAND_ASSIST(x3, x1, x2, x0, 170, 16);
  kp[11] = x3;
  EXPAND_ASSIST(x0, x1, x2, x3, 255, 32);
  kp[12] = x0;
  EXPAND_ASSIST(x3, x1, x2, x0, 170, 32);
  kp[13] = x3;
  EXPAND_ASSIST(x0, x1, x2, x3, 255, 64);
  kp[14] = x0;
}

inline int AESSetEncryptKey(const unsigned char *userKey, const int bits,
                               AES_KEY *key) {
  if (bits == 128) {
    AES128KeyExpansion(userKey, key);
  } else if (bits == 192) {
    AES192KeyExpansion(userKey, key);
  } else if (bits == 256) {
    AES256KeyExpansion(userKey, key);
  }
#if (OCB_KEY_LEN == 0)
  key->rounds = 6 + bits / 32;
#endif
  return 0;
}

inline void AESSetDecryptKeyFast(AES_KEY *dkey, const AES_KEY *ekey) {
  int j = 0;
  int i = ROUNDS(ekey);
#if (OCB_KEY_LEN == 0)
  dkey->rounds = i;
#endif
  dkey->rd_key[i--] = ekey->rd_key[j++];
  while (i)
    dkey->rd_key[i--] = _mm_aesimc_si128(ekey->rd_key[j++]);
  dkey->rd_key[i] = ekey->rd_key[j];
}

inline int AESSetDecryptKey(const unsigned char *userKey, const int bits,
                               AES_KEY *key) {
  AES_KEY temp_key;
  AESSetEncryptKey(userKey, bits, &temp_key);
  AESSetDecryptKeyFast(key, &temp_key);
  return 0;
}

inline void AESEncrypt(const unsigned char *in, unsigned char *out,
                        const AES_KEY *key) {
  int j, rnds = ROUNDS(key);
  const __m128i *sched = ((__m128i *) (key->rd_key));
  __m128i tmp = _mm_load_si128((__m128i *) in);
  tmp = _mm_xor_si128(tmp, sched[0]);
  for (j = 1; j < rnds; j++)
    tmp = _mm_aesenc_si128(tmp, sched[j]);
  tmp = _mm_aesenclast_si128(tmp, sched[j]);
  _mm_store_si128((__m128i *) out, tmp);
}

inline void AESDecrypt(const unsigned char *in, unsigned char *out,
                        const AES_KEY *key) {
  int j, rnds = ROUNDS(key);
  const __m128i *sched = ((__m128i *) (key->rd_key));
  __m128i tmp = _mm_load_si128((__m128i *) in);
  tmp = _mm_xor_si128(tmp, sched[0]);
  for (j = 1; j < rnds; j++)
    tmp = _mm_aesdec_si128(tmp, sched[j]);
  tmp = _mm_aesdeclast_si128(tmp, sched[j]);
  _mm_store_si128((__m128i *) out, tmp);
}

inline void AESEcbEncryptBlks(__m128i *blks, unsigned nblks, AES_KEY *key) {
  unsigned i, j, rnds = ROUNDS(key);
  const __m128i *sched = ((__m128i *) (key->rd_key));
  for (i = 0; i < nblks; ++i)
    blks[i] = _mm_xor_si128(blks[i], sched[0]);
  for (j = 1; j < rnds; ++j)
    for (i = 0; i < nblks; ++i)
      blks[i] = _mm_aesenc_si128(blks[i], sched[j]);
  for (i = 0; i < nblks; ++i)
    blks[i] = _mm_aesenclast_si128(blks[i], sched[j]);
}

inline void AESEcbDecryptBlks(__m128i *blks, unsigned nblks, AES_KEY *key) {
  unsigned i, j, rnds = ROUNDS(key);
  const __m128i *sched = ((__m128i *) (key->rd_key));
  for (i = 0; i < nblks; ++i)
    blks[i] = _mm_xor_si128(blks[i], sched[0]);
  for (j = 1; j < rnds; ++j)
    for (i = 0; i < nblks; ++i)
      blks[i] = _mm_aesdec_si128(blks[i], sched[j]);
  for (i = 0; i < nblks; ++i)
    blks[i] = _mm_aesdeclast_si128(blks[i], sched[j]);
}

#endif /* CRYPTO_AES_H_ */
