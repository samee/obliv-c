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

#include "scd/scd.h"

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <new>
#include "util/log.h"

int ReadSCD(const string& file_name, GarbledCircuit* garbled_circuit) {
  std::ifstream f(file_name, std::ios::out);
  if (!f.is_open()) {
    LOG(ERROR) << "can't open " << file_name << endl;
    return FAILURE;
  }

  f >> garbled_circuit->p_init_size >> garbled_circuit->g_init_size
      >> garbled_circuit->e_init_size >> garbled_circuit->p_input_size
      >> garbled_circuit->g_input_size >> garbled_circuit->e_input_size
      >> garbled_circuit->dff_size >> garbled_circuit->output_size
      >> garbled_circuit->terminate_id >> garbled_circuit->gate_size;

  if (posix_memalign((void **) (&garbled_circuit->garbledGates), 128,
                     sizeof(GarbledGate) * garbled_circuit->gate_size)) {
    LOG(ERROR) << "Linux is a cheap miser that refuses to give us memory"
               << endl;
    LOG(ERROR) << strerror(errno) << endl;
    return FAILURE;
  }
  if (garbled_circuit->output_size > 0) {
    if (posix_memalign((void **) (&garbled_circuit->outputs), 128,
                       sizeof(int64_t) * garbled_circuit->output_size)) {
      LOG(ERROR) << "Linux is a cheap miser that refuses to give us memory"
                 << endl;
      LOG(ERROR) << strerror(errno) << endl;
      return FAILURE;
    }
  } else {
    garbled_circuit->outputs = nullptr;
  }
  if (garbled_circuit->dff_size > 0) {
    try {
      garbled_circuit->D = new int64_t[garbled_circuit->dff_size];
      garbled_circuit->I = new int64_t[garbled_circuit->dff_size];
    } catch (std::bad_alloc& e) {
      LOG(ERROR) << "Linux is a cheap miser that refuses to give us memory"
                 << endl << e.what() << endl;
      return FAILURE;

    }
  } else {
    garbled_circuit->D = nullptr;
    garbled_circuit->I = nullptr;
  }

  for (uint64_t i = 0; i < garbled_circuit->gate_size; i++) {
    garbled_circuit->garbledGates[i].output = garbled_circuit->get_gate_lo_index()
        + i;
  }

  for (uint64_t i = 0; i < garbled_circuit->gate_size; i++) {
    f >> garbled_circuit->garbledGates[i].input0;
  }

  for (uint64_t i = 0; i < garbled_circuit->gate_size; i++) {
    f >> garbled_circuit->garbledGates[i].input1;
  }
  for (uint64_t i = 0; i < garbled_circuit->gate_size; i++) {
    f >> garbled_circuit->garbledGates[i].type;
  }
  for (uint64_t i = 0; i < garbled_circuit->output_size; i++) {
    f >> garbled_circuit->outputs[i];
  }
  for (uint64_t i = 0; i < garbled_circuit->dff_size; i++) {
    f >> garbled_circuit->D[i];
  }
  for (uint64_t i = 0; i < garbled_circuit->dff_size; i++) {
    f >> garbled_circuit->I[i];
  }

  f.close();

  return SUCCESS;
}

int WriteSCD(const ReadCircuit& read_circuit, const string &file_name) {
  std::ofstream f(file_name.c_str(), std::ios::out);
  if (!f.is_open()) {
    LOG(ERROR) << "can't open " << file_name << endl;
    return -1;
  }

  f << read_circuit.p_init_size << " " << read_circuit.g_init_size << " "
    << read_circuit.e_init_size << " " << read_circuit.p_input_size << " "
    << read_circuit.g_input_size << " " << read_circuit.e_input_size << " "
    << read_circuit.dff_size << " " << read_circuit.output_size << " "
    << read_circuit.terminate_id << " " << read_circuit.gate_size << endl;

  /*
   * 1st input of each gate
   */
  for (uint64_t i = 0; i < read_circuit.gate_size; i++) {
    int gindex = read_circuit.task_schedule[i];
    f << read_circuit.gate_list[gindex].input[0] << " ";
  }
  f << endl;
  /*
   * 2nd input of each gate
   */
  for (uint64_t i = 0; i < read_circuit.gate_size; i++) {
    int gindex = read_circuit.task_schedule[i];
    f << read_circuit.gate_list[gindex].input[1] << " ";
  }
  f << endl;
  /*
   *type of each gate
   */
  for (uint64_t i = 0; i < read_circuit.gate_size; i++) {
    int gindex = read_circuit.task_schedule[i];
    f << (int) read_circuit.gate_list[gindex].type << " ";
  }
  f << endl;
  /*
   * outputs : output wire index
   */
  for (uint64_t i = 0; i < read_circuit.output_size; i++) {
    f << read_circuit.output_list[i] << " ";
  }
  f << endl;
  /*
   * D : latch index
   * it stores a wire index for DFF:
   */
  for (uint64_t i = 0; i < read_circuit.dff_size; i++) {
    f << read_circuit.dff_list[i].input[0] << " ";  //D
  }
  f << endl;
  /*
   * I : initial value index
   * it store the input index or constant value for each DFF.
   * I[i] == CONST_ZERO (== -2) : it means the initial value of the DFF is zero.
   * At the first cycle, Garbler should send token[0] to Evaluator.
   * I[i] == CONST_ONE (== -3) : it means the initial value of the DFF is one.
   * At the first cycle, Garbler should send token[1] to Evaluator.
   * I[i] > 0 : it means the initial value of the DFF is value of actual
   * circuit init port.
   */
  for (uint64_t i = 0; i < read_circuit.dff_size; i++) {
    f << (int64_t) read_circuit.dff_list[i].input[1] << " ";  //I
  }
  f << endl;

  f.close();

  return 0;
}
