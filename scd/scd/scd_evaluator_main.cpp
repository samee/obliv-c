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
#include "scd/scd_evaluator.h"

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "scd/scd.h"
#include "util/common.h"
#include "util/log.h"
#include "util/util.h"

namespace po = boost::program_options;
using std::string;

int main(int argc, char*argv[]) {
  LogInitial(argc, argv);

  string scd_file_address;
  string p_init_f_hex_str;
  string g_init_f_hex_str;
  string e_init_f_hex_str;
  string p_input_f_hex_str;
  string g_input_f_hex_str;
  string e_input_f_hex_str;
  uint64_t clock_cycles;
  int64_t terminate_period;
  string output_mode_str;
  OutputMode output_mode = OutputMode::consecutive;
  boost::format fmter(
      "Evaluate Netlist in Plain-Text, TinyGarble version %1%.%2%.%3%.\nAllowed options");
  fmter % TinyGarble_VERSION_MAJOR % TinyGarble_VERSION_MINOR
      % TinyGarble_VERSION_PATCH;
  po::options_description desc(fmter.str());
  desc.add_options()("help,h", "produce help message")  //
  ("scd_file,i", po::value<string>(&scd_file_address), "scd address")  //
  ("clock_cycles,c", po::value<uint64_t>(&clock_cycles)->default_value(1),
   "Number of clock cycles to evaluate the circuit.")  //
  ("p_init", po::value<string>(&p_init_f_hex_str)->default_value("0"),
   "p_init file or in hexadecimal. In case of file, each "
   "line should contain multiple of 4 bits (e.g., 4bit, 8bit, 32bit).")  //
  ("g_init", po::value<string>(&g_init_f_hex_str)->default_value("0"),
   "g_init file or in hexadecimal.")  //
  ("e_init", po::value<string>(&e_init_f_hex_str)->default_value("0"),
   "e_init file or in hexadecimal.")  //
  ("p_input", po::value<string>(&p_input_f_hex_str)->default_value("0"),
   "p_input file or in hexadecimal.")  //
  ("g_input", po::value<string>(&g_input_f_hex_str)->default_value("0"),
   "g_input file or in hexadecimal.")  //
  ("e_input", po::value<string>(&e_input_f_hex_str)->default_value("0"),
   "e_input file or in hexadecimal.")  //
  ("terminate_period,t",
   po::value<int64_t>(&terminate_period)->default_value(0),
   "Terminate signal reveal period: "
   "0: No termination or never reveal, T: Reveal every T clock cycle.")  //
  ("output_mode", po::value<string>(&output_mode_str),
   "output print mode: {0:consecutive, 1:separated_clock, "
   "2:last_clock}, e.g., consecutive, 0, 1");

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
    LOG(ERROR) << e.what() << endl << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  if (scd_file_address.empty()) {
    std::cerr << "ERROR: scd_file(i) must be indicated." << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  if (vm.count("output_mode")) {
    if (vm["output_mode"].as<string>() == "0"
        || vm["output_mode"].as<string>() == "consecutive") {
      output_mode = OutputMode::consecutive;
    } else if (vm["output_mode"].as<string>() == "1"
        || vm["output_mode"].as<string>() == "separated_clock") {
      output_mode = OutputMode::separated_clock;
    } else if (vm["output_mode"].as<string>() == "2"
        || vm["output_mode"].as<string>() == "last_clock") {
      output_mode = OutputMode::last_clock;
    } else {
      LOG(ERROR) << "ERROR: output_mode should be in "
                 "{0:consecutive, 1:separated_clock, "
                 "2:last_clock}, e.g., consecutive, 0, 1"
                 << endl;
      std::cout << desc << endl;
      return FAILURE;
    }
  }

  string p_init_str = ReadFileOrPassHex(p_init_f_hex_str);
  string g_init_str = ReadFileOrPassHex(g_init_f_hex_str);
  string e_init_str = ReadFileOrPassHex(e_init_f_hex_str);
  string p_input_str = ReadFileOrPassHex(p_input_f_hex_str);
  string g_input_str = ReadFileOrPassHex(g_input_f_hex_str);
  string e_input_str = ReadFileOrPassHex(e_input_f_hex_str);

  string output_str;
  EvalauatePlaintextStr(scd_file_address, p_init_str, g_init_str, e_init_str,
                        p_input_str, g_input_str, e_input_str, clock_cycles,
                        terminate_period, output_mode, &output_str);

  std::cout << output_str << endl;

  LogFinish();
  return SUCCESS;
}

