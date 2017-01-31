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

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <ctime>

#include "crypto/OT_extension.h"
#include "garbled_circuit/garbled_circuit.h"
#include "scd/scd.h"
#include "tcpip/tcpip.h"
#include "util/util.h"
#include "util/tinygarble_config.h"
#include "util/log.h"

namespace po = boost::program_options;
using std::string;
using std::vector;

int CheckOptionsAlice(const string& scd_file_address, uint64_t clock_cycles,
                      const string& output_mask, bool disable_OT,
                      bool low_mem_foot, int connfd) {

  int size = scd_file_address.length();
  SendData(connfd, &size, sizeof(int));
  if (size > 0) {
    SendData(connfd, scd_file_address.c_str(), size);
  }

  SendData(connfd, &clock_cycles, sizeof(uint64_t));

  size = output_mask.length();
  SendData(connfd, &size, sizeof(int));
  if (size > 0) {
    SendData(connfd, output_mask.c_str(), size);
  }

  SendData(connfd, &disable_OT, sizeof(bool));
  SendData(connfd, &low_mem_foot, sizeof(bool));

  int status;
  RecvData(connfd, &status, sizeof(int));

  LOG(INFO) << "status is " << status << endl;

  return status;
}

int CheckOptionsBob(const string& scd_file_address, uint64_t clock_cycles,
                    const string& output_mask, bool disable_OT,
                    bool low_mem_foot, int connfd) {

  string scd_file_address_;
  uint64_t clock_cycles_;
  string output_mask_;
  bool disable_OT_;
  bool low_mem_foot_;

  char *buff;
  int size;

  RecvData(connfd, &size, sizeof(int));
  if (size <= 0) {
    scd_file_address_ = "";
  } else {
    buff = new char[size];
    RecvData(connfd, buff, size);
    scd_file_address_ = string(buff);
    delete[] buff;
  }

  RecvData(connfd, &clock_cycles_, sizeof(uint64_t));

  RecvData(connfd, &size, sizeof(int));
  if (size <= 0) {
    output_mask_ = "";
  } else {
    buff = new char[size];
    RecvData(connfd, buff, size);
    output_mask_ = string(buff);
    delete[] buff;
  }

  RecvData(connfd, &disable_OT_, sizeof(bool));
  RecvData(connfd, &low_mem_foot_, sizeof(bool));

  int status;
  if (scd_file_address_ != scd_file_address || clock_cycles_ != clock_cycles
      || output_mask_ != output_mask || disable_OT_ != disable_OT
      || low_mem_foot_ != low_mem_foot) {
    LOG(ERROR) << "Alice's and Bob's options are not same." << endl;
    status = FAILURE;
    SendData(connfd, &status, sizeof(int));
  } else {
    status = SUCCESS;
    SendData(connfd, &status, sizeof(int));
  }

  return status;
}

int main(int argc, char* argv[]) {

  LogInitial(argc, argv);
  HashInit();
  srand(time(0));
  SrandSSE(time(0));

  int port;
  string scd_file_address;
  string server_ip;
  string p_init_f_hex_str;
  string p_input_f_hex_str;
  string init_f_hex_str;
  string input_f_hex_str;
  int64_t terminate_period;
  uint64_t clock_cycles;
  string output_mask;
  string output_mode_str;
  OutputMode output_mode = OutputMode::consecutive;
  bool disable_OT = false;
  bool low_mem_foot = false;
  boost::format fmter(
      "Garble and Evaluate Netlist, TinyGarble version %1%.%2%.%3%.\nAllowed options");
  fmter % TinyGarble_VERSION_MAJOR % TinyGarble_VERSION_MINOR
      % TinyGarble_VERSION_PATCH;
  po::options_description desc(fmter.str());
  desc.add_options()  //
  ("help,h", "produce help message")  //
  ("alice,a", "Run as Alice (server).")  //
  ("bob,b", "Run as Bob (client).")  //
  ("scd_file,i",
   po::value<string>(&scd_file_address)->default_value(
       string(TINYGARBLE_BINARY_DIR) + "/scd/netlists/hamming_32bit_1cc.scd"),
   "Simple circuit description (.scd) file address.")  //
  ("port,p", po::value<int>(&port)->default_value(1234), "socket port")  //
  ("server_ip,s", po::value<string>(&server_ip)->default_value("127.0.0.1"),
   "Server's (Alice's) IP, required when running as Bob.")  //
  ("p_init", po::value<string>(&p_init_f_hex_str)->default_value("0"),
   "File or Hexadecimal public init for initializing DFFs. In case of file,"
   " each line should contain multiple of 4 bits (e.g., 4bit, 8bit, 32bit).")  //
  ("p_input", po::value<string>(&p_input_f_hex_str)->default_value("0"),
   "File or Hexadecimal public input.")  //
  ("init", po::value<string>(&init_f_hex_str)->default_value("0"),
   "Hexadecimal init for initializing DFFs.")  //
  ("input", po::value<string>(&input_f_hex_str)->default_value("0"),
   "File or Hexadecimal input.")  //
  ("clock_cycles", po::value<uint64_t>(&clock_cycles)->default_value(1),
   "Number of clock cycles to evaluate the circuit.")  //
  ("dump_directory", po::value<string>(&dump_prefix)->default_value(""),
   "Directory for dumping memory hex files.")  //
  ("disable_OT", "Disables Oblivious Transfer (OT) for transferring labels. "
   "WARNING: OT is crucial for GC security.")  //
  ("low_mem_foot", "Enables low memory footprint mode for circuits with "
   "multiple clock cycles. In this mode, OT is called at each clock cycle "
   "which degrades the performance.")  //
  ("output_mask", po::value<string>(&output_mask)->default_value("0"),
   "Hexadecimal mask for output. 0 indicates that output belongs to Bob, "
   "and 1 belongs to Alice. It has the same length of the output for a "
   "single clock in case of sequential circuits.")  //
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
    LOG(ERROR) << "ERROR: " << e.what() << endl << endl;
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
                 "{0:consecutive, 1:separated_clock, 2:last_clock}"
                 << endl;
      std::cout << desc << endl;
      return FAILURE;
    }
  }

  if (vm.count("disable_OT")) {
    disable_OT = true;
    LOG(INFO) << "OT is disabled. WARNING:OT is crucial for GC security."
              << endl;
  }

  if (vm.count("low_mem_foot")) {
    low_mem_foot = true;
    LOG(INFO) << "Low memory footprint mode is on." << endl;
  }

  if (vm.count("alice") == 0 && vm.count("bob") == 0) {
    LOG(ERROR) << "One of --alice or --bob mode flag should be used." << endl
               << endl;
    std::cout << desc << endl;
    return FAILURE;
  }

  // Transferring file in to hex
  string p_init_str = ReadFileOrPassHex(p_init_f_hex_str);
  string p_input_str = ReadFileOrPassHex(p_input_f_hex_str);
  string init_str = ReadFileOrPassHex(init_f_hex_str);
  string input_str = ReadFileOrPassHex(input_f_hex_str);

  if (vm.count("alice")) {
    // open the socket
    int connfd;
    if ((connfd = ServerInit(port)) == -1) {
      LOG(ERROR) << "Cannot open the socket in port " << port << endl;
      return FAILURE;
    }
    LOG(INFO) << "Open Alice's server on port: " << port << endl;

    //CHECK(
    //    CheckOptionsAlice("", clock_cycles, output_mask, disable_OT,
    //                      low_mem_foot, connfd));

    string output_str;
    uint64_t delta_time = RDTSC;
    CHECK(
        GarbleStr(scd_file_address, p_init_str, p_input_str, init_str,
                  input_str, clock_cycles, output_mask, terminate_period,
                  output_mode, disable_OT, low_mem_foot, &output_str, connfd));
    delta_time = RDTSC - delta_time;

    LOG(INFO) << "Alice's output = " << output_str << endl;
    LOG(INFO) << "Total Alice time (cc) = " << delta_time << endl;
    std::cout << output_str << endl;

    ServerClose(connfd);
  } else if (vm.count("bob")) {

    if (vm.count("server_ip")) {
      server_ip = vm["server_ip"].as<string>();
    } else {
      LOG(ERROR) << "Server IP should be specified, when running as Bob."
                 << endl << endl;
      std::cout << desc << endl;
      return -1;
    }

    // open socket, connect to server
    int connfd;
    if ((connfd = ClientInit(server_ip.c_str(), port)) == -1) {
      LOG(ERROR) << "Cannot connect to " << server_ip << ":" << port << endl;
      return FAILURE;
    }
    LOG(INFO) << "Connect Bob's client to Alice's server on " << server_ip
              << ":" << port << endl;

    //CHECK(
    //    CheckOptionsBob("", clock_cycles, output_mask, disable_OT, low_mem_foot,
    //                    connfd));

    string output_str;
    uint64_t delta_time = RDTSC;
    CHECK(
        EvaluateStr(scd_file_address, p_init_str, p_input_str, init_str,
                    input_str, clock_cycles, output_mask, terminate_period,
                    output_mode, disable_OT, low_mem_foot, &output_str, connfd));
    delta_time = RDTSC - delta_time;

    LOG(INFO) << "Bob's output = " << output_str << endl;
    LOG(INFO) << "Total Bob time (cc) = " << delta_time << endl;
    std::cout << output_str << endl;

    ClientClose(connfd);
  }

  LogFinish();
  HashFinish();
  return SUCCESS;
}

