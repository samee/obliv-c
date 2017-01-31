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

#include "crypto/OT_extension.h"

#include <openssl/evp.h>
#include <iostream>
#include <cstring>
#include "crypto/OT.h"
#include "crypto/BN.h"
#include "tcpip/tcpip.h"
#include "util/common.h"
#include "util/log.h"

using std::endl;

#define SEC_K_BIT ((uint32_t)128)

const EVP_MD *md = nullptr;
int HashInit() {
  if (md == nullptr) {
    OpenSSL_add_all_digests();
    md = EVP_sha256();
    if (md == nullptr) {
      return FAILURE;
    }
  }
  return SUCCESS;
}

void HashFinish() {
  EVP_cleanup();
}

int HashBN(BIGNUM* r, uint32_t bits, uint32_t tweak, const BIGNUM* v) {
  if (bits > 256) {
    LOG(ERROR) << "Can not provide more than 256bit hash." << endl;
    return FAILURE;
  }

  EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
  if (mdctx == nullptr) {
    LOG(ERROR) << "Can not create EVP_MD_CTX" << endl;
    return FAILURE;
  }
  BN_CHECK(EVP_DigestInit_ex(mdctx, md, NULL));
  BN_CHECK(EVP_DigestUpdate(mdctx, &tweak, sizeof(uint32_t)));

  int v_words = BN_num_bytes(v);
  int diff_words = v_words - (int) SEC_K_BIT / 8;
  if (diff_words <= 0) {
    BN_CHECK(EVP_DigestUpdate(mdctx, v->d, v_words));
    for (int i = 0; i < -diff_words; i++) {
      char c = 0;
      BN_CHECK(EVP_DigestUpdate(mdctx, &c, 1));
    }
  } else {
    BN_CHECK(EVP_DigestUpdate(mdctx, v->d+diff_words, SEC_K_BIT/8));
  }
  unsigned char md_value[EVP_MAX_MD_SIZE];
  uint32_t md_len;
  BN_CHECK(EVP_DigestFinal_ex(mdctx, md_value, &md_len));
  EVP_MD_CTX_destroy(mdctx);

  BN_CHECK(BN_bin2bn(md_value, bits / 8, r));

  return SUCCESS;
}

int SwitchRowColumnBN(const BIGNUM* const * v, uint32_t v_len,
                      uint32_t v_bit_len, BIGNUM*** w) {
  BIGNUM** _w = new BIGNUM*[v_bit_len];
  for (uint32_t i = 0; i < v_bit_len; i++) {
    _w[i] = BN_new();
    BN_CHECK(BN_full_one(_w[i], v_len));
    for (uint32_t k = 0; k < v_len; k++) {
      if (BN_is_bit_set(v[k], i)) {
        BN_CHECK(BN_set_bit(_w[i], k));
      } else {
        BN_CHECK(BN_clear_bit(_w[i], k));
      }
    }
  }

  *w = _w;
  return SUCCESS;
}

int SwitchRowColumnBNPair(const BIGNUM* const * const * v, uint32_t v_len,
                          uint32_t v_bit_len, BIGNUM**** w) {
  BIGNUM*** _w = new BIGNUM**[v_bit_len];
  for (uint32_t i = 0; i < v_bit_len; i++) {
    _w[i] = new BIGNUM*[2];
    for (uint32_t j = 0; j < 2; j++) {
      _w[i][j] = BN_new();
      BN_CHECK(BN_full_one(_w[i][j], v_len));
      for (uint32_t k = 0; k < v_len; k++) {
        if (BN_is_bit_set(v[k][j], i)) {
          BN_CHECK(BN_set_bit(_w[i][j], k));
        } else {
          BN_CHECK(BN_clear_bit(_w[i][j], k));
        }
      }
    }
  }

  *w = _w;
  return SUCCESS;
}

int OTExtSendBN(const BIGNUM* const * const * m, uint32_t m_bitlen,
                uint32_t m_len, int connfd) {

  // 0. generate random k-bit s
  BIGNUM* S = BN_new();
  BN_rand(S, SEC_K_BIT, -1, 0);

  BIGNUM** Q = new BIGNUM*[SEC_K_BIT];
  for (uint32_t i = 0; i < SEC_K_BIT; i++) {
    Q[i] = BN_new();
  }
  BN_new();

  CHECK(OTRecvBN(S, SEC_K_BIT, connfd, Q));

  BIGNUM** Q_rotate;
  CHECK(SwitchRowColumnBN(Q, SEC_K_BIT, m_len, &Q_rotate));

  BIGNUM* h0 = BN_new();
  BIGNUM* h1 = BN_new();
  BIGNUM* QS = BN_new();
  BIGNUM* y0 = BN_new();
  BIGNUM* y1 = BN_new();
  for (uint32_t i = 0; i < m_len; i++) {
    CHECK(HashBN(h0, m_bitlen, i, Q_rotate[i]));
    BN_CHECK(BN_xor(QS, SEC_K_BIT, Q_rotate[i], S));
    CHECK(HashBN(h1, m_bitlen, i, QS));
    BN_CHECK(BN_xor(y0, m_bitlen, h0, m[i][0]));
    BN_CHECK(BN_xor(y1, m_bitlen, h1, m[i][1]));
    CHECK(SendBN(connfd, y0));
    CHECK(SendBN(connfd, y1));
  }
  BN_free(h0);
  BN_free(h1);
  BN_free(QS);
  BN_free(y0);
  BN_free(y1);

  // free memory
  for (uint32_t i = 0; i < m_len; i++) {
    BN_free(Q_rotate[i]);
  }
  delete[] Q_rotate;
  for (uint32_t i = 0; i < SEC_K_BIT; i++) {
    BN_free(Q[i]);
  }
  delete[] Q;
  BN_free(S);
  return SUCCESS;
}
int OTExtRecvBN(const BIGNUM *sel, uint32_t m_bitlen, uint32_t m_len,
                int connfd, BIGNUM** m) {

  // 0.generate m_len random k-bits T ([0]) and T^r ([1])
  BIGNUM*** T = new BIGNUM**[m_len];
  for (uint32_t i = 0; i < m_len; i++) {
    T[i] = new BIGNUM*[2];
    for (uint32_t j = 0; j < 2; j++) {
      T[i][j] = BN_new();
    }
    BN_rand(T[i][0], SEC_K_BIT, -1, 0);
    int sel_bit = BN_is_bit_set(sel, i);
    if (sel_bit == 1) {
      BN_CHECK(BN_copy(T[i][1], T[i][0]));
      BN_CHECK(BN_invert(T[i][1], SEC_K_BIT));
    } else {
      BN_CHECK(BN_copy(T[i][1], T[i][0]));
    }
  }

  BIGNUM*** T_rotate;
  CHECK(SwitchRowColumnBNPair(T, m_len, SEC_K_BIT, &T_rotate));

  CHECK(OTSendBN(T_rotate, SEC_K_BIT, connfd));

  BIGNUM* h = BN_new();
  BIGNUM* y0 = BN_new();
  BIGNUM* y1 = BN_new();
  for (uint32_t i = 0; i < m_len; i++) {
    CHECK(RecvBN(connfd, y0));
    CHECK(RecvBN(connfd, y1));
    CHECK(HashBN(h, m_bitlen, i, T[i][0]));

    m[i] = BN_new();
    if (BN_is_bit_set(sel, i) == 0) {
      BN_CHECK(BN_xor(m[i], m_bitlen, y0, h));

    } else {
      BN_CHECK(BN_xor(m[i], m_bitlen, y1, h));
    }
  }
  BN_free(h);
  BN_free(y0);
  BN_free(y1);

  // free memory
  for (uint32_t i = 0; i < SEC_K_BIT; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      BN_free(T_rotate[i][j]);
    }
    delete[] T_rotate[i];
  }
  delete[] T_rotate;

  for (uint32_t i = 0; i < m_len; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      BN_free(T[i][j]);
    }
    delete[] T[i];
  }
  delete[] T;
  return SUCCESS;
}

int OTExtSend(const block* const * m, uint32_t m_len, int connfd) {
  BIGNUM*** bn_m = new BIGNUM**[m_len];
  for (uint32_t i = 0; i < m_len; i++) {
    bn_m[i] = new BIGNUM*[2];
    for (uint32_t j = 0; j < 2; j++) {
      bn_m[i][j] = BN_new();
      BlockToBN(bn_m[i][j], m[i][j]);
    }
  }
  int ret = OTExtSendBN(bn_m, sizeof(block) * 8, m_len, connfd);
  for (uint32_t i = 0; i < m_len; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      BN_free(bn_m[i][j]);
    }
    delete[] bn_m[i];
  }
  delete[] bn_m;
  return ret;
}

int OTExtRecv(const bool *sel, uint32_t m_len, int connfd, block* m) {
  int ret;
  BIGNUM *bn_sel = BN_new();
  BIGNUM** bn_m = new BIGNUM*[m_len];
  for (uint32_t i = 0; i < m_len; ++i) {
    bn_m[i] = BN_new();
    if (sel[i]) {
      BN_CHECK(BN_set_bit(bn_sel, i));
    }
  }
  if ((ret = OTExtRecvBN(bn_sel, sizeof(block) * 8, m_len, connfd, bn_m))
      == FAILURE) {
    return FAILURE;
  }
  for (uint32_t i = 0; i < m_len; ++i) {
    BNToBlock(bn_m[i], &m[i]);
    BN_free(bn_m[i]);
  }
  BN_free(bn_sel);
  delete[] bn_m;
  return ret;
}

