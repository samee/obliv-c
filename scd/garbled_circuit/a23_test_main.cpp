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

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <cstdlib>

#include "crypto/OT_extension.h"
#include "garbled_circuit/garbled_circuit.h"
#include "garbled_circuit/garbled_circuit_testing.h"
#include "scd/scd.h"
#include "scd/scd_evaluator.h"
#include "tcpip/tcpip.h"
#include "util/util.h"
#include "util/tinygarble_config.h"
#include "util/log.h"
#include "tcpip/tcpip_testsuit.h"

namespace po = boost::program_options;
using std::string;
using std::vector;

int main(int argc, char* argv[]) {

  LogInitial(argc, argv);
  HashInit();
  //srand(time(0));
  //SrandSSE(time(0));

  srand(111);
  SrandSSE(1111);

  string scd_file_address;
  uint64_t clock_cycles;
  int64_t terminate_period;
  string test_name;

  boost::format fmter(
      "A23 Test, TinyGarble version %1%.%2%.%3%.\nAllowed options");
  fmter % TinyGarble_VERSION_MAJOR % TinyGarble_VERSION_MINOR
      % TinyGarble_VERSION_PATCH;
  po::options_description desc(fmter.str());
  desc.add_options()  //
  ("help,h", "produce help message")  //
  ("scd_file,i",
   po::value<string>(&scd_file_address)->default_value(
       string(TINYGARBLE_BINARY_DIR)
           + "/scd/netlists/a23_gc_main_64_w_n_cc.scd"),
   "Simple circuit description (.scd) file address.")  //
  ("clock_cycles", po::value<uint64_t>(&clock_cycles)->default_value(1000),
   "Number of clock cycles to evaluate the circuit.")  //
  ("terminate_period,t",
   po::value<int64_t>(&terminate_period)->default_value(1),
   "Terminate signal reveal period: "
   "0: No termination or never reveal, T: Reveal every T clock cycle.")  //
  ("test_name,n", po::value<string>(&test_name)->default_value("sum"),
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

  OutputMode output_mode = OutputMode::last_clock;
  string p_input_str = "";
  string g_input_str = "";
  string e_input_str = "";
  string output_mask = "0";
  string alice_output = "0";
  bool disable_OT = false;
  bool low_mem_foot = true;

  string p_init_f_hex_str = string(TINYGARBLE_SOURCE_DIR) + "/a23/" + test_name
      + "/p.txt";
  string g_init_f_hex_str = string(TINYGARBLE_SOURCE_DIR) + "/a23/"
      + test_name + "/test/g.txt";
  string e_init_f_hex_str = string(TINYGARBLE_SOURCE_DIR) + "/a23/"
      + test_name + "/test/e.txt";

  string p_init_str = ReadFileOrPassHex(p_init_f_hex_str);
  string g_init_str = ReadFileOrPassHex(g_init_f_hex_str);
  string e_init_str = ReadFileOrPassHex(e_init_f_hex_str);

  string output_str;

  LOG(INFO) << "A32 Test " << test_name << " " << clock_cycles
            << "cc with terminate period " << terminate_period << endl;
  int ret = EvalauatePlaintextStr(scd_file_address, p_init_str, g_init_str,
                                  e_init_str, p_input_str, g_input_str,
                                  e_input_str, clock_cycles, terminate_period,
                                  output_mode, &output_str);
  CHECK_EXPR_MSG(ret == SUCCESS, "EvalauatePlaintextStr");

  LOG(INFO) << "SCD Evaluate result: " << output_str << endl;

  GCTestStruct garbler_data = MakeGCTestStruct(scd_file_address, p_init_str,
                                               p_input_str, g_init_str,
                                               g_input_str, alice_output,
                                               output_mask, terminate_period,
                                               output_mode, disable_OT,
                                               low_mem_foot, clock_cycles);
  GCTestStruct eval_data = MakeGCTestStruct(scd_file_address, p_init_str,
                                            p_input_str, e_init_str,
                                            e_input_str, output_str,
                                            output_mask, terminate_period,
                                            output_mode, disable_OT,
                                            low_mem_foot, clock_cycles);

  ret = TcpipTestRun(Alice, (void *) &garbler_data, Bob, (void *) &eval_data);
  CHECK_EXPR_MSG(ret == SUCCESS, "TcpipTestRun");

  return SUCCESS;
}
