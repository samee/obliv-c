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

#include "crypto/BN.h"

#include <iostream>
#include <openssl/bn.h>
#include "util/log.h"
#include "util/util.h"
#include "util/common.h"
#include "util/minunit.h"

using std::endl;

void TestSetup() {
}
void TestTeardown() {
}

MU_TEST(BlockEndian) {

  block w;
  uint8_t *w_ch = (uint8_t *) &w;
  for (uint i = 0; i < sizeof(block); i++) {
    w_ch[i] = i;
  }
  block v = _m128_switch_endian(w);
  uint8_t *v_ch = (uint8_t *) &v;
  for (uint i = 0; i < sizeof(block); i++) {
    mu_check(v_ch[i] == sizeof(block) - i - 1);
  }

}

MU_TEST(BNBlock) {

  BIGNUM* a = BN_new();
  BIGNUM* b = BN_new();
  BIGNUM* c = BN_new();
  block w;
  block v;
  block r;
  block t;
  uint8_t *w_ch = (uint8_t *) &w;
  uint8_t *v_ch = (uint8_t *) &v;

  //////
  for (uint i = 0; i < sizeof(block); i++) {
    w_ch[i] = i;
  }
  BlockToBN(a, w);
  BNToBlock(a, &v);
  for (uint i = 0; i < sizeof(block); i++) {
    mu_check(v_ch[i] == w_ch[i]);
  }
  /////
  BN_one(a);
  BN_one(b);
  BN_add(c, a, b);
  BNToBlock(a, &v);
  BNToBlock(b, &w);
  BNToBlock(c, &t);
  r = _mm_add_epi64(w, v);
  mu_check(CmpBlock(r,t));

  ////
  mu_check(Str2Block("00000000 00000000 00000000 00000010", &r) == SUCCESS);
  t = MakeBlock(0L, 0x10L);
  mu_check(CmpBlock(r,t));

  ////
  mu_check(Str2Block("0092380b 8939f64a 42dcef0d 137ef64a", &w) == SUCCESS);
  BlockToBN(a, w);
  BNToBlock(a, &v);
  for (uint i = 0; i < sizeof(block); i++) {
    mu_check(v_ch[i] == w_ch[i]);
  }

  ////
  mu_check(Str2Block("15060708 490a0b0c 5d0e0f10 11121300", &w) == SUCCESS);
  BlockToBN(a, w);
  BNToBlock(a, &v);
  for (uint i = 0; i < sizeof(block); i++) {
    mu_check(v_ch[i] == w_ch[i]);
  }

  BN_free(a);
  BN_free(b);
  BN_free(c);
}

MU_TEST(BNBitwise) {

  BIGNUM *v = BN_new();
  BIGNUM *w = BN_new();
  BIGNUM *r = BN_new();
  BN_zero(v);
  BN_zero(w);

  BN_xor(r, 10, v, w);
  mu_check(BN_is_zero(r));

  BN_full_one(v, 10);
  BN_xor(r, 10, v, w);
  for (int i = 0; i < 10; i++) {
    mu_check(BN_is_bit_set(r, i));
  }

  BN_full_one(w, 10);
  BN_xor(r, 10, v, w);
  mu_check(BN_is_zero(r));

  BN_invert(v, 10);
  mu_check(BN_is_zero(v));

  BN_invert(v, 10);
  for (int i = 0; i < 10; i++) {
    mu_check(BN_is_bit_set(v, i));
  }

  BN_free(v);
  BN_free(w);
  BN_free(r);
}

MU_TEST_SUITE(TestSuite) {
  MU_SUITE_CONFIGURE(&TestSetup, &TestTeardown);

  MU_RUN_TEST(BlockEndian);
  MU_RUN_TEST(BNBlock);
  MU_RUN_TEST(BNBitwise);
}

int main(int argc, char *argv[]) {
  LogInitial(argc, argv);
  MU_RUN_SUITE(TestSuite);
  MU_REPORT()
  ;
  LogFinish();
  return 0;
}

