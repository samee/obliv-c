/*
 This file is part of TinyGarble.

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

#define G_SIZE 64 // max size of Garbler's input array
#define E_SIZE 64 // max size of Evaluator's input array
#define O_SIZE 64 // max size of output array

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <openssl/bn.h>

#include "util/util.h"
#include "util/tinygarble_config.h"
#include "util/log.h"

namespace po = boost::program_options;
using std::string;
using std::vector;

extern void gc_main(const int *g, const int *e, int *o);

void BNtoByteLittleEndian(BIGNUM* bn, uint8_t *v, uint v_len) {

  int temp_len = BN_num_bytes(bn);
  uint8_t *temp_byte = new uint8_t[temp_len];
  BN_bn2bin(bn, temp_byte);

  // change big-endian to little-endian
  for (int i = 0; i < (int) v_len; i++) {
    if (temp_len - i - 1 >= 0) {
      v[i] = temp_byte[temp_len - i - 1];
    } else {
      v[i] = 0;
    }
  }
  delete[] temp_byte;
}

void ByteLittleEndiantoBN(const uint8_t *v, uint v_len, BIGNUM* bn) {

  uint8_t *temp_byte = new uint8_t[v_len];

  // change big-endian to little-endian
  for (int i = 0; i < (int) v_len; i++) {
    temp_byte[i] = v[v_len - i - 1];
  }

  BN_bin2bn((const uint8_t *) temp_byte, v_len, bn);

  delete[] temp_byte;
}

int main(int argc, char* argv[]) {

  LogInitial(argc, argv);

  string test_name;

  boost::format fmter(
      "A23 C Function Test, TinyGarble version %1%.%2%.%3%.\nAllowed options");
  fmter % TinyGarble_VERSION_MAJOR % TinyGarble_VERSION_MINOR
      % TinyGarble_VERSION_PATCH;
  po::options_description desc(fmter.str());
  desc.add_options()  //
  ("help,h", "produce help message")  //
  ("test_name,n", po::value<string>(&test_name)->default_value(""),
   "A23 test name.");

  po::variables_map vm;
  try {
    po::parsed_options parsed = po::command_line_parser(argc, argv).options(
        desc).allow_unregistered().run();
    po::store(parsed, vm);
    if (vm.count("help")) {
      std::cout << desc << endl;
      return SUCCESS;
    }
    po::notify(vm);
  } catch (po::error& e) {
    LOG(ERROR) << "ERROR: " << e.what() << endl << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  if (test_name == "") {
    LOG(ERROR) << "ERROR: test_name should be set." << endl << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  string g_f_hex_str = string(TINYGARBLE_SOURCE_DIR) + "/a23/" + test_name
      + "/test/g.txt";
  string e_f_hex_str = string(TINYGARBLE_SOURCE_DIR) + "/a23/" + test_name
      + "/test/e.txt";

  string g_str = ReadFileOrPassHex(g_f_hex_str);
  string e_str = ReadFileOrPassHex(e_f_hex_str);

  LOG(INFO) << "g_str = " << g_str << endl;
  LOG(INFO) << "e_str = " << e_str << endl;

  BIGNUM* g_bn = BN_new();
  BN_hex2bn(&g_bn, g_str.c_str());

  BIGNUM* e_bn = BN_new();
  BN_hex2bn(&e_bn, e_str.c_str());

  int *g = new int[G_SIZE];
  int *e = new int[E_SIZE];

  BNtoByteLittleEndian(g_bn, (uint8_t *) g, G_SIZE * sizeof(int));
  BNtoByteLittleEndian(e_bn, (uint8_t *) e, E_SIZE * sizeof(int));

  int *o = new int[O_SIZE];
  memset(o, 0, O_SIZE * sizeof(int));

  gc_main(g, e, o);

  BIGNUM* o_bn = BN_new();
  ByteLittleEndiantoBN((const uint8_t *) o, O_SIZE * sizeof(int), o_bn);

  string o_str = BN_bn2hex(o_bn);


  LOG(INFO) << "o_str = " << o_str << endl;

  std::cout << o_str << endl;

  delete[] g;
  delete[] e;
  delete[] o;
  BN_free(g_bn);
  BN_free(e_bn);
  BN_free(o_bn);
  return SUCCESS;
}
