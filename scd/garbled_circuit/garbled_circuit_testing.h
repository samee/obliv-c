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
#ifndef GARBLED_CIRCUIT_GARBLED_CIRCUIT_TESTING_H_
#define GARBLED_CIRCUIT_GARBLED_CIRCUIT_TESTING_H_

#include <cstdint>
#include <string>
#include "garbled_circuit/garbled_circuit_util.h"

using std::string;

struct GCTestStruct {
  string scd_file_address;
  string p_init;
  string p_input;
  string init;
  string input;
  string output;
  string output_mask;
  int64_t terminate_period;
  OutputMode output_mode;
  bool disable_OT;
  bool low_mem_foot;
  uint64_t clock_cycles;
};

GCTestStruct MakeGCTestStruct(const string& scd_file_address,
                              const string& p_init, const string& p_input,
                              const string& init, const string& input,
                              const string& output, const string& output_mask,
                              int64_t terminate_period, OutputMode output_mode,
                              bool disable_OT, bool low_mem_foot,
                              uint64_t clock_cycles);

int Alice(const void* data, int connfd);
int Bob(const void *data, int connfd);

#endif /* GARBLED_CIRCUIT_GARBLED_CIRCUIT_TESTING_H_ */
