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

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>
#include "scd/parse_netlist.h"
#include "scd/scd.h"
#include "scd/scheduling.h"
#include "util/log.h"

namespace po = boost::program_options;
using std::ofstream;
using std::endl;

int main(int argc, char** argv) {
  LogInitial(argc, argv);

  string input_netlist_file, brist_input_netlist_file;
  string output_scd_file;

  boost::format fmter(
      "Verilog to SCD, TinyGarble version %1%.%2%.%3%.\nAllowed options");
  fmter % TinyGarble_VERSION_MAJOR % TinyGarble_VERSION_MINOR
      % TinyGarble_VERSION_PATCH;
  po::options_description desc(fmter.str());
  desc.add_options()  //
  ("help,h", "produce help message.")  //
  ("netlist,i", po::value<string>(&input_netlist_file),
   "Input netlist (verilog .v) file address.")  //
  ("brist_netlist,b", po::value<string>(&brist_input_netlist_file),
   "Input netlist (.txt) file address (in the format given by "
   "www.cs.bris.ac.uk/Research/CryptographySecurity/MPC/).")  //
  ("scd,o", po::value<string>(&output_scd_file),
   "Output simple circuit description (scd) file address.");

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

  if (output_scd_file.empty()) {
    std::cerr << "ERROR: output scd (-o) flag must be indicated." << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  string out_mapping_filename = output_scd_file + ".map";

  if (!input_netlist_file.empty()) {
    LOG(INFO) << "V2SCD " << input_netlist_file << " to " << output_scd_file
              << endl;
    if (Verilog2SCD(input_netlist_file, out_mapping_filename,
                    output_scd_file) == FAILURE) {
      LOG(ERROR) << "Verilog to SCD failed." << endl;
      return FAILURE;
    }
  } else if (!brist_input_netlist_file.empty()) {
    LOG(INFO) << "Brist2SCD " << brist_input_netlist_file << " to "
              << output_scd_file << endl;
    if (Bris2SCD(brist_input_netlist_file, out_mapping_filename,
                 output_scd_file) == FAILURE) {
      LOG(ERROR) << "Brist netlist to SCD failed." << endl;
      return FAILURE;
    }
  } else {
    std::cerr << "ERROR: input netlist flags must be indicated." << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  LogFinish();
  return SUCCESS;
}
