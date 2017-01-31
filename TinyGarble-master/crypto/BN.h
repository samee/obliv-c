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

#ifndef CRYPTO_BN_H_
#define CRYPTO_BN_H_

#include <openssl/bn.h>
#include <cstdint>
#include "crypto/block.h"

int BN_invert(BIGNUM* v, uint32_t bits);
int BN_full_one(BIGNUM* v, uint32_t bits);
int BN_xor(BIGNUM* r, uint32_t bits, const BIGNUM* v, const BIGNUM* w);
void BlockToBN(BIGNUM *a, block w);
void BNToBlock(const BIGNUM *a, block *w);
int SendBN(int connf, const BIGNUM *bignum);
int RecvBN(int connf, BIGNUM *bignum);

#endif /* CRYPTO_BN_H_ */

