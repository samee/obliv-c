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

#include "scd/v_2_scd.h"

#include "util/log.h"
#include "util/util.h"
#include "util/common.h"
#include "util/tinygarble_config.h"
#include "util/minunit.h"

#define TEST_REPEAT 10

void TestSetup() {
}
void TestTeardown() {
}

MU_TEST(SumNbitNcc) {

  vector<string> verilogfilenames = { "aes_11cc.v", "hamming_32bit_1cc.v",
      "mips_32bit_64mem_ncc.v", "public_test_8bit_ncc.v", "aes_1cc.v",
      "hamming_32bit_8cc.v", "mult_32bit_1cc.v", "compare_nbit_ncc.v",
      "k_nns_31bit_4nei_ncc.v", "mult_32bit_32cc.v", "sha3_24cc.v",
      "encoder_32bit_1cc.v", "mux_8bit_1cc.v", "sum_8bit_1cc.v",
      "non_secret_test_8bit_ncc.v", "sum_nbit_ncc.v" };

  for (uint64_t i = 0; i < verilogfilenames.size(); i++) {

    string in_file_name = string(TINYGARBLE_BINARY_DIR) + "/scd/netlists/"
        + verilogfilenames[i];
    string out_file_name = "/dev/null";
    string out_mapping_filename = "/dev/null";

    LOG(INFO) << "Verilog2SCD " << in_file_name << endl;
    int ret = Verilog2SCD(in_file_name, out_mapping_filename, out_file_name);

    string error_str = "Verilog2SCD " + verilogfilenames[i];
    mu_assert(ret == SUCCESS, error_str.c_str());
  }
}

MU_TEST_SUITE(TestSuite) {
  MU_SUITE_CONFIGURE(&TestSetup, &TestTeardown);

  MU_RUN_TEST(SumNbitNcc);
}

int main(int argc, char *argv[]) {
  LogInitial(argc, argv);
  srand(time(0));
  MU_RUN_SUITE(TestSuite);
  MU_REPORT()
  ;
  LogFinish();
  return 0;
}
