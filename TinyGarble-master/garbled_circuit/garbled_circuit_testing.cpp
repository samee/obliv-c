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
#include "garbled_circuit/garbled_circuit_testing.h"

#include <iostream>
#include "garbled_circuit/garbled_circuit.h"
#include "crypto/OT_extension.h"
#include "tcpip/tcpip.h"
#include "tcpip/tcpip_testsuit.h"
#include "scd/scd_evaluator.h"
#include "util/common.h"
#include "util/util.h"
#include "util/log.h"
#include "util/tinygarble_config.h"

GCTestStruct MakeGCTestStruct(const string& scd_file_address,
                              const string& p_init, const string& p_input,
                              const string& init, const string& input,
                              const string& output, const string& output_mask,
                              int64_t terminate_period, OutputMode output_mode,
                              bool disable_OT, bool low_mem_foot,
                              uint64_t clock_cycles) {
  GCTestStruct ret;

  ret.scd_file_address = scd_file_address;
  ret.p_init = p_init;
  ret.p_input = p_input;
  ret.init = init;
  ret.input = input;
  ret.output = output;
  ret.output_mask = output_mask;
  ret.terminate_period = terminate_period;
  ret.output_mode = output_mode;
  ret.disable_OT = disable_OT;
  ret.low_mem_foot = low_mem_foot;
  ret.clock_cycles = clock_cycles;
  return ret;
}

int Alice(const void* data, int connfd) {
  GCTestStruct* gc_data = (GCTestStruct*) data;
  string output_str;
  int ret = GarbleStr(gc_data->scd_file_address, gc_data->p_init,
                      gc_data->p_input, gc_data->init, gc_data->input,
                      gc_data->clock_cycles, gc_data->output_mask,
                      gc_data->terminate_period, gc_data->output_mode,
                      gc_data->disable_OT, gc_data->low_mem_foot, &output_str,
                      connfd);

  if (ret == FAILURE) {
    LOG(ERROR) << "GarbleStr failed.";
    return FAILURE;
  }

  if (output_str != gc_data->output) {
    LOG(ERROR) << "Alice-side equality test failed "
               "(plain-text's != garble circuit's): "
               << gc_data->output << " != " << output_str << endl;
    return FAILURE;
  }
  LOG(INFO) << "Equality passed: " << output_str << " == " << gc_data->output
            << endl;
  return SUCCESS;
}

int Bob(const void *data, int connfd) {
  GCTestStruct* gc_data = (GCTestStruct*) data;
  string output_str;
  int ret = EvaluateStr(gc_data->scd_file_address, gc_data->p_init,
                        gc_data->p_input, gc_data->init, gc_data->input,
                        gc_data->clock_cycles, gc_data->output_mask,
                        gc_data->terminate_period, gc_data->output_mode,
                        gc_data->disable_OT, gc_data->low_mem_foot, &output_str,
                        connfd);
  if (ret == FAILURE) {
    LOG(ERROR) << "EvaluateStr failed.";
    return FAILURE;
  }

  if (output_str != gc_data->output) {
    LOG(ERROR) << "Bob's side equality test failed "
               "(plain-text's != garble circuit's): "
               << gc_data->output << " != " << output_str << endl;
    return FAILURE;
  }
  LOG(INFO) << "Equality passed: " << output_str << " == " << gc_data->output
            << endl;

  return SUCCESS;
}

