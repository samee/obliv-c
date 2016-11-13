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

#include "util/log.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <map>
#include <vector>
#include "util/common.h"
#include "util/util.h"

namespace po = boost::program_options;
using std::string;
using std::ofstream;
using std::map;
using std::endl;
using std::vector;

string dump_prefix = "";
bool dump_enabled = false;
map<string, ofstream*> dump_map;
ostream* log_map[2];  // ERROR, INFO
DummyLog dummy_log;
bool __dummy_expr__;

DummyLog& DummyLogStream() {
  return dummy_log;
}

void LogInitial(int argc, char *argv[]) {

  string dump_directory;
  po::options_description desc("");
  desc.add_options()  //
  ("alice,a", "")  //
  ("bob,b", "")  //
  ("dump_directory", po::value<string>(&dump_directory), "")  //
  ("error2std", "")  //
  ("log2std", "");

  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc)
      .allow_unregistered().run();

  po::store(parsed, vm);

  bool log_2_std = false;
  if (vm.count("log2std")) {
    log_2_std = true;
  }

  bool error_2_std = false;
  if (vm.count("error2std")) {
    error_2_std = true;
  }

  if (vm.count("dump_directory")) {
    if (vm.count("alice"))
      dump_directory += "alice-";
    else if (vm.count("bob"))
      dump_directory += "bob-";

    dump_prefix = dump_directory;
    vector<string> dumps = { "dff", "input", "output", "table", "r_key" };
    for (const string& f : dumps) {
      dump_map[f] = new ofstream();
    }
  }

  string timestamp = std::to_string(time(nullptr));
  string file_addresss_prefix = string(argv[0]) + "-" + timestamp;

  if (log_2_std) {
    log_map[INFO] = &std::cout;
  } else {
    string file_addresss_info = file_addresss_prefix + "-info.log";
    log_map[INFO] = new std::ofstream(file_addresss_info.c_str(),
                                      std::ios::out);
  }

  if (log_2_std || error_2_std) {
    log_map[ERROR] = &std::cerr;
  } else {
    string file_addresss_error = file_addresss_prefix + "-error.log";
    log_map[ERROR] = new std::ofstream(file_addresss_error.c_str(),
                                       std::ios::out);
  }
}

void LogFinish() {
  if (std::ofstream* log_of = dynamic_cast<std::ofstream*>(log_map[ERROR])) {
    log_of->close();
    delete log_map[ERROR];
  }
  if (std::ofstream* log_of = dynamic_cast<std::ofstream*>(log_map[INFO])) {
    log_of->close();
    delete log_map[INFO];
  }
  for (const auto& dump : dump_map) {
    if (std::ofstream* dump_of = dynamic_cast<std::ofstream*>(dump.second)) {
      if (dump_of->is_open())
        dump_of->close();
      delete dump.second;
    }
  }
}

// dump_file : { "dff", "input", "output", "table", "r_key" }
ostream& Dump(const string& dump_file) {
  if (dump_prefix == "") {
    return dummy_log;
  }
  if (dump_map.count(dump_file)) {
    if (!dump_map[dump_file]->is_open()) {
      string file_addresss = dump_prefix + dump_file + ".hex";
      dump_map[dump_file]->open(file_addresss.c_str(), std::ios::out);
    }
    return *dump_map[dump_file];
  }
  LOG(ERROR) << "No such a dump file " << dump_file << endl;
  return std::cerr;
}

ostream& LogStream(int log_code) {
  if (log_code != ERROR && log_code != INFO) {
    std::cerr << "No such a LOG stream. Use LOG(INFO) or LOG(ERROR)" << log_code
              << endl;
    return std::cerr;
  }
  return *log_map[log_code];
}

std::ostream & operator <<(std::ostream & o, const block& v) {
  uint32_t *v_u32 = (uint32_t*) &v;
  std::ios::fmtflags f(o.flags());
  o << std::hex << std::setfill('0') << std::setw(8) << v_u32[3] << " "
    << std::setfill('0') << std::setw(8) << v_u32[2] << " " << std::setfill('0')
    << std::setw(8) << v_u32[1] << " " << std::setfill('0') << std::setw(8)
    << v_u32[0];
  o.flags(f);
  return o;
}
