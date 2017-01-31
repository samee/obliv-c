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

#include "util/common.h"
#include "util/log.h"
#include "util/util.h"
#include "scd/scd.h"

int EvalauatePlaintext(const GarbledCircuit& garbled_circuit,
                       const BIGNUM* p_init, const BIGNUM* g_init,
                       const BIGNUM* e_init, const BIGNUM* p_input,
                       const BIGNUM* g_input, const BIGNUM* e_input,
                       uint64_t *clock_cycles, int64_t terminate_period,
                       BIGNUM** outputs) {

  bool* wires = nullptr;
  CHECK_ALLOC(wires = new bool[garbled_circuit.get_wire_size()]);

  bool* dff_latch = nullptr;
  CHECK_ALLOC(dff_latch = new bool[garbled_circuit.dff_size]);

  if (*outputs == nullptr) {
    *outputs = BN_new();
  }
  bn_expand(*outputs, (*clock_cycles) * garbled_circuit.output_size);

  for (uint64_t cid = 0; cid < (*clock_cycles); cid++) {  //for each clock cycle
    //dff initial value
    uint64_t dff_bias = garbled_circuit.get_dff_lo_index();
    if (cid == 0) {
      for (uint64_t i = 0; i < garbled_circuit.dff_size; i++) {
        int64_t wire_index = garbled_circuit.I[i];
        bool b;
        if (wire_index == CONST_ZERO) {
          b = false;
        } else if (wire_index == CONST_ONE) {
          b = true;
        } else if (wire_index >= 0
            && wire_index < (int64_t) garbled_circuit.get_init_size()) {
          if (wire_index < (int64_t) garbled_circuit.get_p_init_hi_index()) {
            b = BN_is_bit_set(
                p_init,
                wire_index - (int64_t) garbled_circuit.get_p_init_lo_index());
          } else if (wire_index
              < (int64_t) garbled_circuit.get_g_init_hi_index()) {
            b = BN_is_bit_set(
                g_init,
                wire_index - (int64_t) garbled_circuit.get_g_init_lo_index());
          } else {  // e_init
            b = BN_is_bit_set(
                e_init,
                wire_index - (int64_t) garbled_circuit.get_e_init_lo_index());
          }
        } else {
          LOG(ERROR) << "Invalid I index: " << wire_index << endl;
          b = false;
        }
        wires[dff_bias + i] = b;
      }
    } else {  //copy latched labels
      for (uint64_t i = 0; i < garbled_circuit.dff_size; i++) {
        int64_t wire_index = garbled_circuit.D[i];
        bool b;
        if (wire_index == CONST_ZERO) {
          b = false;
        } else if (wire_index == CONST_ONE) {
          b = true;
        } else if (wire_index >= 0
            && wire_index < (int64_t) garbled_circuit.get_wire_size()) {
          b = wires[wire_index];
        } else {
          LOG(ERROR) << "Invalid D index: " << wire_index << endl;
          b = false;
        }
        dff_latch[i] = b;
      }
      for (uint64_t i = 0; i < garbled_circuit.dff_size; i++) {
        wires[dff_bias + i] = dff_latch[i];
      }
    }

    // inputs
    uint64_t input_bias = garbled_circuit.get_input_lo_index();
    for (int64_t i = 0; i < (int64_t) garbled_circuit.get_input_size(); i++) {
      uint64_t bit_bias;
      if ((int64_t) (i + input_bias)
          < (int64_t) garbled_circuit.get_p_input_hi_index()) {  // input belongs to p
        bit_bias = cid * garbled_circuit.p_input_size + i + input_bias
            - (int64_t) garbled_circuit.get_p_input_lo_index();
        wires[input_bias + i] = BN_is_bit_set(p_input, bit_bias);

      } else if ((int64_t) (i + input_bias)
          < (int64_t) garbled_circuit.get_g_input_hi_index()) {  // input belongs to g
        bit_bias = cid * garbled_circuit.g_input_size + i + input_bias
            - (int64_t) garbled_circuit.get_g_input_lo_index();
        wires[input_bias + i] = BN_is_bit_set(g_input, bit_bias);
      } else {  // e
        bit_bias = cid * garbled_circuit.e_input_size + i + input_bias
            - (int64_t) garbled_circuit.get_e_input_lo_index();
        wires[input_bias + i] = BN_is_bit_set(e_input, bit_bias);
      }
    }

    for (uint64_t i = 0; i < garbled_circuit.gate_size; i++) {  // for each gates
      GarbledGate& garbled_gate = garbled_circuit.garbledGates[i];
      int64_t input0 = garbled_gate.input0;
      int64_t input1 = garbled_gate.input1;
      int64_t output = garbled_gate.output;
      int type = garbled_gate.type;

      // constant values
      bool inp0;
      if (input0 == CONST_ZERO) {
        inp0 = false;
      } else if (input0 == CONST_ONE) {
        inp0 = true;
      } else if (input0 >= 0
          && input0 < (int64_t) garbled_circuit.get_wire_size()) {
        inp0 = wires[input0];
      } else {
        LOG(ERROR) << "Invalid input0 index: " << input0 << endl;
        inp0 = false;
      }

      bool inp1;
      if (input1 == CONST_ZERO) {
        inp1 = false;
      } else if (input1 == CONST_ONE) {
        inp1 = true;
      } else if (input1 >= 0
          && input1 < (int64_t) garbled_circuit.get_wire_size()) {
        inp1 = wires[input1];
      } else if (type != NOTGATE) {
        LOG(ERROR) << "Invalid input1 index: " << input1 << endl;
        inp1 = false;
      }

      if (type != NOTGATE)
        wires[output] = GateOperator(type, inp0, inp1);
      else
        wires[output] = GateOperator(type, inp0);
    }

    for (uint64_t i = 0; i < garbled_circuit.output_size; i++) {
      if (wires[garbled_circuit.outputs[i]])
        BN_set_bit(*outputs, cid * garbled_circuit.output_size + i);
      else
        BN_clear_bit(*outputs, cid * garbled_circuit.output_size + i);
    }

    //if has terminate signal
    if (terminate_period != 0 && garbled_circuit.terminate_id > 0) {
      if (cid % terminate_period == 0) {
        bool is_terminate = wires[garbled_circuit.terminate_id];
        if (is_terminate) {
          LOG(INFO) << "Terminated in " << cid + 1 << "cc out of "
              << *clock_cycles << "cc." << endl;
          *clock_cycles = cid + 1;
          break;
        }
      }
      //last clock cycle, not terminated
      if (cid == (*clock_cycles) - 1) {
        LOG(ERROR) << "Not enough clock cycles. Circuit is not terminated in "
            << *clock_cycles << "cc." << endl;
      }
    }
  }

  delete[] wires;
  delete[] dff_latch;

  return SUCCESS;
}

int EvalauatePlaintextStr(const string& scd_file_address,
                          const string& p_init_str, const string& g_init_str,
                          const string& e_init_str, const string& p_input_str,
                          const string& g_input_str, const string& e_input_str,
                          uint64_t clock_cycles, int64_t terminate_period,
                          OutputMode output_mode, string* outputs_str) {
  BIGNUM* p_init = BN_new();
  BIGNUM* g_init = BN_new();
  BIGNUM* e_init = BN_new();
  BIGNUM* p_input = BN_new();
  BIGNUM* g_input = BN_new();
  BIGNUM* e_input = BN_new();

  BN_hex2bn(&p_init, p_init_str.c_str());
  BN_hex2bn(&g_init, g_init_str.c_str());
  BN_hex2bn(&e_init, e_init_str.c_str());
  BN_hex2bn(&p_input, p_input_str.c_str());
  BN_hex2bn(&g_input, g_input_str.c_str());
  BN_hex2bn(&e_input, e_input_str.c_str());

  LOG(INFO) << "p_init = " << BN_bn2hex(p_init) << "\tg_init = "
            << BN_bn2hex(g_init) << "\te_init = " << BN_bn2hex(e_init)
            << "\tp_input = " << BN_bn2hex(p_input) << "\tg_input = "
            << BN_bn2hex(g_input) << "\te_input = " << BN_bn2hex(e_input)
            << endl;

  GarbledCircuit garbled_circuit;
  if (ReadSCD(scd_file_address, &garbled_circuit) == FAILURE) {
    LOG(ERROR) << "Error while reading scd file: " << scd_file_address << endl;
    return FAILURE;
  }

  if (terminate_period != 0 && garbled_circuit.terminate_id == 0) {
    LOG(ERROR) << "There is no terminate signal in the circuit."
               " The terminate period should be 0."
               << endl;
    return FAILURE;
  }

  BIGNUM* outputs = BN_new();
  EvalauatePlaintext(garbled_circuit, p_init, g_init, e_init, p_input, g_input,
                     e_input, &clock_cycles, terminate_period, &outputs);

  OutputBN2StrHighMem(garbled_circuit, outputs, clock_cycles, output_mode,
                      outputs_str);
  return SUCCESS;
}

