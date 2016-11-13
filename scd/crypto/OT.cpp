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

#include "crypto/OT.h"

#include <iostream>
#include "crypto/BN.h"
#include "tcpip/tcpip.h"
#include "util/common.h"
#include "util/log.h"

using std::endl;

int OTSendBN(const BIGNUM* const * const * m, uint32_t m_len, int connfd) {

  BN_CTX *ctx;
  ctx = BN_CTX_new();
  if (ctx == nullptr) {
    LOG(ERROR) << "allocating ctx failed." << endl;
  }

  // 0. check the vector size
  CHECK(SendData(connfd, &m_len, sizeof(uint32_t)));

  // 1.0. generate RSA key
  BIGNUM * rsa_exponent = BN_new();
  BN_CHECK(BN_set_word(rsa_exponent, RSA_F4));
  RSA *rsa = RSA_new();
  BN_CHECK(RSA_generate_key_ex(rsa, RSA_BITS, rsa_exponent, nullptr));
  BN_CHECK(RSA_check_key(rsa));
  BN_free(rsa_exponent);

  // 1.1. send public portion to Bob (receiver)
  CHECK(SendBN(connfd, rsa->n));
  CHECK(SendBN(connfd, rsa->e));

  // 2. generate two messages
  BIGNUM ***x = new BIGNUM**[m_len];
  for (uint32_t i = 0; i < m_len; i++) {
    x[i] = new BIGNUM*[2];
    for (uint32_t j = 0; j < 2; j++) {
      x[i][j] = BN_new();
      BN_CHECK(BN_rand(x[i][j], RSA_BITS, -1, 0));
      CHECK(SendBN(connfd, x[i][j]));
    }
  }

  // 4. receive v from Bob (receiver) and send m0' and m1' to Bob
  BIGNUM *v = BN_new();
  BIGNUM *temp = BN_new();
  BIGNUM *k0 = BN_new();
  BIGNUM *k1 = BN_new();
  BIGNUM *m0 = BN_new();
  BIGNUM *m1 = BN_new();
  for (uint32_t i = 0; i < m_len; i++) {
    CHECK(RecvBN(connfd, v));

    BN_CHECK(BN_sub(temp, v, x[i][0]));  // temp = v - x0
    BN_CHECK(BN_mod_exp(k0, temp, rsa->d, rsa->n, ctx));  // k0 = (v - x0)^d mod N

    BN_CHECK(BN_sub(temp, v, x[i][1]));  // temp = v - x1
    BN_CHECK(BN_mod_exp(k1, temp, rsa->d, rsa->n, ctx));  // k1 = (v - x0)^d mod N

    BN_CHECK(BN_add(k0, k0, m[i][0]));
    CHECK(SendBN(connfd, k0));  // send m0' = m0 + k0
    BN_CHECK(BN_add(k1, k1, m[i][1]));
    CHECK(SendBN(connfd, k1));  // send m1' = m1 + k1
  }

  RSA_free(rsa);
  BN_free(v);
  BN_free(temp);
  BN_free(k0);
  BN_free(k1);
  BN_free(m0);
  BN_free(m1);

  for (uint32_t i = 0; i < m_len; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      BN_free(x[i][j]);
    }
  }
  BN_CTX_free(ctx);
  return SUCCESS;
}
int OTRecvBN(const BIGNUM *sel, uint32_t m_len, int connfd, BIGNUM** m) {
  // 0. check the vector size
  uint32_t m_len_from_sender;
  CHECK(RecvData(connfd, &m_len_from_sender, sizeof(uint32_t)));
  if (m_len_from_sender != m_len) {
    LOG(ERROR) << "receiver: The size of vectors are not equal" << endl
               << "sender's length = " << m_len_from_sender << " "
               << "reciver's length = " << m_len << endl;
    return FAILURE;
  }

  BN_CTX *ctx;
  ctx = BN_CTX_new();
  if (ctx == nullptr) {
    LOG(ERROR) << "allocating ctx failed." << endl;
  }
  BIGNUM* rsa_n = BN_new();
  BIGNUM* rsa_e = BN_new();

  // 1. receive public portion of a rsa key from Alice (receiver)
  CHECK(RecvBN(connfd, rsa_n));
  CHECK(RecvBN(connfd, rsa_e));

  // 2. receive two random messages
  BIGNUM ***x = new BIGNUM**[m_len];
  for (uint32_t i = 0; i < m_len; i++) {
    x[i] = new BIGNUM*[2];
    for (uint32_t j = 0; j < 2; j++) {
      x[i][j] = BN_new();
      CHECK(RecvBN(connfd, x[i][j]));
    }
  }

  // 3. generate random k
  BIGNUM **k = new BIGNUM*[m_len];
  BIGNUM *_k = BN_new();
  for (uint32_t i = 0; i < m_len; i++) {
    BN_CHECK(BN_rand(_k, RSA_BITS, -1, 0));
    k[i] = BN_new();
    BN_CHECK(BN_nnmod(k[i], _k, rsa_n, ctx));  // k = _k mod N
  }
  BN_free(_k);

  // 4. compute v = (x_b + k^e) mod N and send it to Alice (sender)
  BIGNUM *v = BN_new();
  BIGNUM *temp = BN_new();
  for (uint32_t i = 0; i < m_len; i++) {
    int sel_bit = BN_is_bit_set(sel, i);
    BN_CHECK(BN_mod_exp(temp, k[i], rsa_e, rsa_n, ctx));  // K^e mod N
    BN_CHECK(BN_add(temp, x[i][sel_bit], temp));  // x_b + (K^e mod N)
    BN_CHECK(BN_nnmod(v, temp, rsa_n, ctx));  // v = (x_b + K^e) mod N
    CHECK(SendBN(connfd, v));
  }
  BN_free(rsa_e);
  BN_free(rsa_n);
  BN_free(v);
  BN_free(temp);

  // 4. receive m0' and m1' from Alice
  BIGNUM *m0p = BN_new();
  BIGNUM *m1p = BN_new();
  BIGNUM *mb = BN_new();
  for (uint32_t i = 0; i < m_len; i++) {
    CHECK(RecvBN(connfd, m0p));
    CHECK(RecvBN(connfd, m1p));
    int sel_bit = BN_is_bit_set(sel, i);
    if (sel_bit == 0) {
      BN_CHECK(BN_sub(mb, m0p, k[i]));  //mb = m0p - k[i]
    } else {
      BN_CHECK(BN_sub(mb, m1p, k[i]));  //mb = m1p - k[i]
    }
    BN_copy(m[i], mb);
  }

  BN_free(m0p);
  BN_free(m1p);

  for (uint32_t i = 0; i < m_len; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      BN_free(x[i][j]);
    }
    BN_free(k[i]);
  }
  BN_CTX_free(ctx);

  return SUCCESS;
}

int OTSend(const block* const * m, uint32_t m_len, int connfd) {
  BIGNUM*** bn_m = new BIGNUM**[m_len];
  for (uint32_t i = 0; i < m_len; i++) {
    bn_m[i] = new BIGNUM*[2];
    for (uint32_t j = 0; j < 2; j++) {
      bn_m[i][j] = BN_new();
      BlockToBN(bn_m[i][j], m[i][j]);
    }
  }
  int ret = OTSendBN(bn_m, m_len, connfd);
  for (uint32_t i = 0; i < m_len; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      BN_free(bn_m[i][j]);
    }
    delete[] bn_m[i];
  }
  delete[] bn_m;
  return ret;
}

int OTRecv(const bool *sel, uint32_t m_len, int connfd, block* m) {
  int ret;
  BIGNUM *bn_sel = BN_new();
  BIGNUM** bn_m = new BIGNUM*[m_len];
  for (uint32_t i = 0; i < m_len; ++i) {
    bn_m[i] = BN_new();
    if (sel[i]) {
      BN_CHECK(BN_set_bit(bn_sel, i));
    }
  }
  if ((ret = OTRecvBN(bn_sel, m_len, connfd, bn_m)) == FAILURE) {
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

