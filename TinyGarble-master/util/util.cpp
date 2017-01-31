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

#include "util/util.h"

#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <iomanip>
#include <sstream>
#include <fstream>
#include "crypto/aes.h"
#include "util/common.h"
#include "util/log.h"

using std::ifstream;

static block cur_seed;

void SrandSSE(unsigned int seed) {
  cur_seed = _mm_set_epi32(seed, seed + 1, seed, seed + 1);
}

block RandomBlock() {

  block cur_seed_split;
  block multiplier;
  block adder;
  block mod_mask;
  //block sra_mask;
  //block sseresult;

  static const unsigned int mult[4] = { 214013, 17405, 214013, 69069 };
  static const unsigned int gadd[4] = { 2531011, 10395331, 13737667, 1 };
  static const unsigned int mask[4] = { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
  //static const unsigned int masklo[4] = { 0x00007FFF, 0x00007FFF, 0x00007FFF,
  //  0x00007FFF };

  adder = _mm_load_si128((block *) gadd);
  multiplier = _mm_load_si128((block *) mult);
  mod_mask = _mm_load_si128((block *) mask);
  //sra_mask = _mm_load_si128((block *) masklo);
  cur_seed_split = _mm_shuffle_epi32(cur_seed, _MM_SHUFFLE(2, 3, 0, 1));
  cur_seed = _mm_mul_epu32(cur_seed, multiplier);
  multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
  cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);
  cur_seed = _mm_and_si128(cur_seed, mod_mask);
  cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
  cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));
  cur_seed = _mm_or_si128(cur_seed, cur_seed_split);
  cur_seed = _mm_add_epi32(cur_seed, adder);

  return cur_seed;

}
/**
 * \brief constant v value based on gate Type
 *
 * Two Halves Make a Whole Reducing Data Transfer in Garbled Circuits using Half Gates
 *
 *
 *  a op b = ((a ^ v0) & (b ^ v1)) ^ v2
 *  v2v1v0
 *  000b -> AND
 *  111b -> OR
 */
unsigned short Type2V(int gateType) {
  if (gateType == ANDGATE) {
    return 0b000;
  } else if (gateType == ANDNGATE) {
    return 0b010;
  } else if (gateType == NANDGATE) {
    return 0b100;
  } else if (gateType == NANDNGATE) {
    return 0b110;
  } else if (gateType == ORGATE) {
    return 0b111;
  } else if (gateType == ORNGATE) {
    return 0b101;
  } else if (gateType == NORGATE) {
    return 0b011;
  } else if (gateType == NORNGATE) {
    return 0b001;
  }
  return 0;
}

bool GateOperator(int gateType, bool input0, bool input1 /* = false */) {
  if (gateType == ANDGATE) {
    return (input0 && input1);
  } else if (gateType == ANDNGATE) {
    return (input0 && !input1);
  } else if (gateType == NANDGATE) {
    return !(input0 && input1);
  } else if (gateType == NANDNGATE) {
    return !(input0 && !input1);
  } else if (gateType == ORGATE) {
    return (input0 || input1);
  } else if (gateType == ORNGATE) {
    return (input0 || !input1);
  } else if (gateType == NORGATE) {
    return !(input0 || input1);
  } else if (gateType == NORNGATE) {
    return !(input0 || !input1);
  } else if (gateType == XORGATE) {
    return (input0 != input1);
  } else if (gateType == XNORGATE) {
    return !(input0 != input1);
  } else if (gateType == NOTGATE) {
    return !input0;
  }
  LOG(ERROR) << "Unknown gate type " << gateType << endl;
  return false;
}

int Str2Block(const string &s, block* v) {
  if (!v) {
    LOG(ERROR) << "null pointer in strToBlock." << endl;
    return FAILURE;
  }

  string v_str = s;
  boost::erase_all(v_str, " ");
  boost::erase_all(v_str, "\t");
  boost::erase_all(v_str, "_");

  if (v_str.length() > sizeof(block) * 2) {
    LOG(ERROR) << "Can not parse hex string to 128-bit block: " << v_str << endl
               << "string (len = " << v_str.length() << ") is longer than "
               << sizeof(block) * 2 << endl;
    return FAILURE;
  }

  while (v_str.length() < sizeof(block) * 2) {
    v_str.insert(0, "0");
  }
  string lo_str = v_str.substr(sizeof(block), sizeof(block));
  string hi_str = v_str.substr(0, sizeof(block));
  uint64_t lo, hi;
  try {
    lo = std::stoull(lo_str, nullptr, 16);
    hi = std::stoull(hi_str, nullptr, 16);
  } catch (std::exception& e) {

    //LOG(ERROR) << "error in string to ull: " << endl;
    return FAILURE;
  }

  *v = MakeBlock(hi, lo);

  return SUCCESS;
}

string to_string_hex(uint64_t v, int pad /* = 0 */) {
  std::stringstream stream;
  stream << std::hex << std::setw(pad) << std::setfill('0') << v;
  string ret = stream.str();
  return ret;
}

int OutputBN2StrHighMem(const GarbledCircuit& garbled_circuit, BIGNUM* outputs,
                        uint64_t clock_cycles, OutputMode output_mode,
                        string *output_str) {
  (*output_str) = "";
  if (output_mode == OutputMode::consecutive) {  // normal
    const char* output_c = BN_bn2hex(outputs);
    (*output_str) = output_c;
  } else if (output_mode == OutputMode::separated_clock) {  // Separated by clock
    BIGNUM* temp = BN_new();
    for (uint64_t i = 0; i < clock_cycles; i++) {
      BN_rshift(temp, outputs, i * garbled_circuit.output_size);
      BN_mask_bits(temp, garbled_circuit.output_size);
      (*output_str) += BN_bn2hex(temp);
      if (i < clock_cycles - 1) {
        (*output_str) += "\n";
      }
    }
    BN_free(temp);
  } else if (output_mode == OutputMode::last_clock) {  // only last clock
    BIGNUM* temp = BN_new();
    BN_rshift(temp, outputs, (clock_cycles - 1) * garbled_circuit.output_size);
    BN_mask_bits(temp, garbled_circuit.output_size);
    (*output_str) += BN_bn2hex(temp);
    BN_free(temp);
  }
  return SUCCESS;
}

int OutputBN2StrLowMem(const GarbledCircuit& garbled_circuit, BIGNUM* outputs,
                       uint64_t clock_cycles, OutputMode output_mode,
                       string* output_str) {
  (*output_str) = "";
  if (output_mode == OutputMode::consecutive) {  // normal
    const char* output_c = BN_bn2hex(outputs);
    (*output_str) = output_c;
  } else if (output_mode == OutputMode::separated_clock) {  // Separated by clock
    BIGNUM* temp = BN_new();
    for (uint64_t i = 0; i < clock_cycles; i++) {
      BN_rshift(temp, outputs, i * garbled_circuit.output_size);
      BN_mask_bits(temp, garbled_circuit.output_size);
      (*output_str) += BN_bn2hex(temp);
      if (i < clock_cycles - 1) {
        (*output_str) += "\n";
      }
    }
    BN_free(temp);
  } else if (output_mode == OutputMode::last_clock) {  // only last clock
    (*output_str) += BN_bn2hex(outputs);
  }

  return SUCCESS;
}

string ReadFileOrPassHex(string file_hex_str) {  // file address of or a hex string

  ifstream fin;
  fin.open(file_hex_str);
  if (fin.is_open()) {
    string hex_str = "";
    string line;
    while (std::getline(fin, line)) {
      hex_str = line + hex_str;
    }
    return hex_str;
  } else {
    return file_hex_str;
  }
}


bool icompare_pred(unsigned char a, unsigned char b) {
  return std::tolower(a) == std::tolower(b);
}

bool icompare(std::string const& a, std::string const& b) {
  if (a.length() == b.length()) {
    return std::equal(b.begin(), b.end(), a.begin(), icompare_pred);
  } else {
    return false;
  }
}

