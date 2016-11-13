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

#ifndef UTIL_COMMON_H_
#define UTIL_COMMON_H_

#include <string>
#include <iostream>
#include "util/tinygarble_config.h"

using std::string;
using std::cerr;
using std::cout;
using std::endl;


#define SUCCESS 0
#define FAILURE (-1)

#define ROW_REDUCTION
#define FREE_XOR
#define DKC2

#define CONST_ZERO ((int64_t)-2)
#define CONST_ONE  ((int64_t)-3)

#define ANDGATE 8
#define ANDNGATE 4
#define NANDGATE 7
#define NANDNGATE 11
#define ORGATE 14
#define ORNGATE 13
#define NORGATE 1
#define NORNGATE 2
#define XORGATE 6
#define XNORGATE 9
#define NOTGATE 12
#define DFFGATE (-1)

// blocks
#define MUXGATE 15
#define HADDERGATE 16
#define FADDERGATE 17
#define INVALGATE 18

#define RDTSC ({unsigned long long res; \
  unsigned hi, lo;   \
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi)); \
  res =  ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 ); \
  res;})
#define fbits(v, p) ((v & (1 << p))>>p)

extern string dump_prefix;

#endif /* UTIL_COMMON_H_ */
