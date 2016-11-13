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

#include "scd/scheduling.h"

#include <string>
#include <cstring>
#include "scd/parse_netlist.h"
#include "util/common.h"
#include "util/log.h"

int QuickSort(int64_t *arr, int64_t *index, int64_t left, int64_t right) {
  int64_t i = left, j = right;
  int64_t tmp;
  int64_t pivot = arr[(left + right) / 2];

  /* partition */
  while (i <= j) {
    while (arr[i] > pivot)
      i++;
    while (arr[j] < pivot)
      j--;
    if (i <= j) {
      tmp = arr[i];
      arr[i] = arr[j];
      arr[j] = tmp;

      tmp = index[i];
      index[i] = index[j];
      index[j] = tmp;

      i++;
      j--;
    }
  }
  /* recursion */
  if (left < j)
    QuickSort(arr, index, left, j);
  if (i < right)
    QuickSort(arr, index, i, right);

  return SUCCESS;
}

int TopSort(const vector<ReadGate>& G, int64_t no_task, int64_t *index) {

  // calculate static b-level
  int64_t *sl = new int64_t[no_task];

  for (int64_t i = 0; i < no_task; i++)
    index[i] = i;

  for (int64_t i = no_task - 1; i >= 0; i--) {
    int64_t max = 0;
    for (int64_t j = i + 1; j < no_task; j++) {
      if (G[j].input[0] == G[i].output)
        if (sl[j] > max)
          max = sl[j];
      if (G[j].input[1] == G[i].output)
        if (sl[j] > max)
          max = sl[j];
    }
    sl[i] = 1 + max;
  }

  // sort in descending order of static b-level
  QuickSort(sl, index, 0, no_task - 1);

  delete[] sl;
  return SUCCESS;
}

int64_t GetMinIndex(int64_t *arr, int64_t size) {
  int64_t minimum = arr[0], min_index = 0, i = 1;
  while (i < size) {
    if (arr[i] < minimum) {
      minimum = arr[i];
      min_index = i;
    }
    i++;
  }
  return min_index;
}

int64_t GetMax(int64_t *arr, int64_t size) {
  int64_t maximum = arr[0], i = 1;
  while (i < size) {
    if (arr[i] > maximum)
      maximum = arr[i];
    i++;
  }
  return maximum;
}

int DFS(const ReadCircuit &read_circuit, vector<int> *status, int wire_index,
        vector<int> *loop) {

  const vector<ReadGate>& Gates = read_circuit.gate_list;
  int64_t no_of_input_dff = read_circuit.get_init_input_size()
      + read_circuit.dff_size;

  if (wire_index < 0 || (*status)[wire_index] == 2) {  // clear
    return SUCCESS;
  } else if ((*status)[wire_index] == 1) {  // loop
    loop->push_back(wire_index);
    return FAILURE;
  } else {
    (*status)[wire_index] = 1;
    if (DFS(read_circuit, status, Gates[wire_index - no_of_input_dff].input[0],
            loop) == FAILURE) {
      loop->push_back(wire_index);
      return FAILURE;
    }
    if (DFS(read_circuit, status, Gates[wire_index - no_of_input_dff].input[1],
            loop) == FAILURE) {
      loop->push_back(wire_index);
      return FAILURE;
    }
    (*status)[wire_index] = 2;
  }
  return SUCCESS;
}

int FindPath(const ReadCircuit &read_circuit, vector<int> *loop) {

  vector<int> path;
  int64_t no_of_input_dff = read_circuit.get_init_input_size()
      + read_circuit.dff_size;
  int64_t no_of_gate = read_circuit.gate_size;

  vector<int> status(no_of_input_dff + no_of_gate);
  for (int64_t i = 0; i < no_of_input_dff + no_of_gate; i++) {
    if (i < no_of_input_dff) {  // inputs or DFF inputs
      status[i] = 2;  //clear
    } else {
      status[i] = 0;  //un visited
    }
  }

  for (uint64_t i = 0; i < read_circuit.dff_size; i++) {
    if (DFS(read_circuit, &status, read_circuit.dff_list[i].input[0],
            loop) == FAILURE)
      return FAILURE;
  }

  for (uint64_t i = 0; i < read_circuit.output_size; i++) {
    if (DFS(read_circuit, &status, read_circuit.output_list[i], loop) == FAILURE)
      return FAILURE;
  }

  for (int64_t i = 0; i < no_of_input_dff + no_of_gate; i++) {
    if (status[i] != 2)
      return FAILURE;
  }

  return SUCCESS;
}

int Schedule(const ReadCircuit &read_circuit, int64_t no_core, int64_t **core) {

  const vector<ReadGate>& G = read_circuit.gate_list;
  int64_t no_task;

  int64_t input_size = read_circuit.get_init_input_size();

  int64_t no_of_input_dff = input_size + read_circuit.dff_size;
  no_task = read_circuit.gate_size;

  vector<int> loop;
  if (FindPath(read_circuit, &loop) != SUCCESS) {
    LOG(ERROR) << "There is a loop in the circuit." << endl;

    string loop_srt = "";
    for (uint64_t i = 0; i < loop.size(); i++) {
      loop_srt += std::to_string(loop[i] - no_of_input_dff) + " -> ";
    }
    LOG(ERROR) << loop_srt << endl;

    //return FAILURE;
  }

  int64_t *index;
  index = new int64_t[no_task];
  TopSort(G, no_task, index);

  // start of scheduling
  int64_t *p0, *p1, *core_busy, *core_index;

  p0 = new int64_t[no_task];
  memset(p0, -1, no_task * sizeof(int64_t));
  p1 = new int64_t[no_task];
  memset(p1, -1, no_task * sizeof(int64_t));

  core_index = new int64_t[no_core];
  memset(core_index, 0, no_core * sizeof(int64_t));
  core_busy = new int64_t[no_core];
  memset(core_busy, 0, no_core * sizeof(int64_t));

  int64_t scheduled = 0;
  while (scheduled < no_task) {
    for (int64_t i = 0; i < no_task; i++) {
      if (p0[index[i]] == -1) {  // not assigned yet
        if (((G[index[i]].input[0] - no_of_input_dff < 0)
            || (p0[G[index[i]].input[0] - no_of_input_dff] != -1))) {
          if ((G[index[i]].input[1] - no_of_input_dff < 0)
              || (p0[G[index[i]].input[1] - no_of_input_dff] != -1)) {  //ready
            p1[index[i]] = GetMinIndex(core_busy, no_core);
            core[p1[index[i]]][core_index[p1[index[i]]]] = index[i];
            core_index[p1[index[i]]]++;
            core_busy[p1[index[i]]] = core_busy[p1[index[i]]] + 1;
            scheduled++;
          }
        }
      }
    }

    for (int64_t i = 0; i < no_task; i++) {
      p0[i] = p1[i];
    }
  }

  delete[] index;
  delete[] p0;
  delete[] p1;
  delete[] core_index;
  delete[] core_busy;

  return SUCCESS;
}

int TopologicalSortMultiCore(ReadCircuit &read_circuit) {

  int64_t **core;
  core = new int64_t*[1];  // no of rows = no_core
  core[0] = new int64_t[read_circuit.gate_size + 1];  // no of columns = no_of_gates+1
  memset(core[0], -1, (read_circuit.gate_size + 1) * sizeof(uint64_t));

  Schedule(read_circuit, 1, core);

  read_circuit.task_schedule.resize(read_circuit.gate_size);

  vector<int64_t> ts(read_circuit.gate_size);

  int64_t input_size = read_circuit.get_init_input_size();
  for (int64_t i = 0; i < (int64_t) read_circuit.gate_size; i++) {
    read_circuit.task_schedule[i] = core[0][i];
    ts[i] = core[0][i] + input_size + read_circuit.dff_size;
  }

  vector<int64_t> ts_1(
      input_size + read_circuit.dff_size + read_circuit.gate_size);

  for (int64_t i = 0; i < input_size + (int64_t) read_circuit.dff_size; i++) {
    ts_1[i] = i;
  }

  for (int64_t i = 0; i < (int64_t) read_circuit.gate_size; i++) {
    ts_1[ts[i]] = i + input_size + read_circuit.dff_size;
  }
  for (int64_t i = 0; i < (int64_t) read_circuit.dff_size; i++) {
    if (read_circuit.dff_list[i].input[0] > 0) {  // Constant values are negative
      read_circuit.dff_list[i].input[0] =
          ts_1[read_circuit.dff_list[i].input[0]];
    }
    if (read_circuit.dff_list[i].input[1] > 0) {  // Constant values are negative
      read_circuit.dff_list[i].input[1] =
          ts_1[read_circuit.dff_list[i].input[1]];
    }
    read_circuit.dff_list[i].output = ts_1[read_circuit.dff_list[i].output];
  }
  for (int64_t i = 0; i < (int64_t) read_circuit.gate_size; i++) {
    if (read_circuit.gate_list[i].input[0] > 0) {  // Constant values are negative
      read_circuit.gate_list[i].input[0] =
          ts_1[read_circuit.gate_list[i].input[0]];
    }
    if (read_circuit.gate_list[i].input[1] > 0) {  // Constant values are negative
      read_circuit.gate_list[i].input[1] =
          ts_1[read_circuit.gate_list[i].input[1]];
    }
    read_circuit.gate_list[i].output = ts_1[i + input_size
        + read_circuit.dff_size];
  }
  for (int64_t i = 0; i < (int64_t) read_circuit.output_size; i++) {
    read_circuit.output_list[i] = ts_1[read_circuit.output_list[i]];
  }

  LOG(INFO) << endl << "Topological Sort" << endl;
  LOG(INFO) << "dffs:\tD\tI\tQ" << endl;
  for (int64_t i = 0; i < (int64_t) read_circuit.dff_size; i++) {
    LOG(INFO) << i << "\t" << Type2StrGate(read_circuit.dff_list[i].type)
        << "\t" << read_circuit.dff_list[i].input[0] << "\t"
        << read_circuit.dff_list[i].input[1] << "\t"
        << read_circuit.dff_list[i].output << endl;
  }
  LOG(INFO) << endl;

  LOG(INFO) << "gates:\tI0\tI1\tO" << endl;
  for (int64_t i = 0; i < (int64_t) read_circuit.gate_size; i++) {
    int64_t gid = read_circuit.task_schedule[i];
    LOG(INFO) << i << "\t" << Type2StrGate(read_circuit.gate_list[gid].type)
        << "\t" << read_circuit.gate_list[gid].input[0] << "\t"
        << read_circuit.gate_list[gid].input[1] << "\t"
        << read_circuit.gate_list[gid].output << endl;
  }
  LOG(INFO) << endl;

  LOG(INFO) << "outputs:" << endl;
  for (int64_t i = 0; i < (int64_t) read_circuit.output_size; i++) {
    LOG(INFO) << read_circuit.output_list[i] << endl;
  }
  LOG(INFO) << endl;
  delete[] core[0];
  delete[] core;
  return 0;
}
///////////////////////////////////

enum Mark {
  UnMarked = 0,
  TempMarked = 1,  // Temporary mark
  PerMarked = 2   // Permanently mark
};

int TopologicalSortVisit(const ReadCircuit &read_circuit, vector<Mark>* marks,
                         int64_t current_unmark_index,
                         vector<int64_t>* sorted_list, vector<int64_t>* loop) {

  int64_t init_input_size = read_circuit.get_init_input_size();
  int64_t init_input_dff_size = init_input_size + read_circuit.dff_size;

  if (current_unmark_index < 0) {  // CONSTANT or IV 2nd input (-1) are sorted.
    return SUCCESS;
  } else if (marks->at(current_unmark_index) == Mark::TempMarked) {
    LOG(ERROR) << "There is a loop in the circuit." << endl;
    loop->push_back(current_unmark_index);
    return FAILURE;
  } else if (marks->at(current_unmark_index) == Mark::UnMarked) {
    marks->at(current_unmark_index) = Mark::TempMarked;

    if (TopologicalSortVisit(
        read_circuit,
        marks,
        read_circuit.gate_list[current_unmark_index - init_input_dff_size].input[0],
        sorted_list, loop) == FAILURE) {
      loop->push_back(current_unmark_index);
      return FAILURE;
    }
    if (TopologicalSortVisit(
        read_circuit,
        marks,
        read_circuit.gate_list[current_unmark_index - init_input_dff_size].input[1],
        sorted_list, loop) == FAILURE) {
      loop->push_back(current_unmark_index);
      return FAILURE;
    }

    marks->at(current_unmark_index) = Mark::PerMarked;
    sorted_list->push_back(current_unmark_index);
  }

  return SUCCESS;
}

int TopologicalSort(const ReadCircuit &read_circuit,
                    vector<int64_t>* sorted_list,
                    const ReadCircuitString& read_circuit_string) {

  int64_t init_input_size = read_circuit.get_init_input_size();
  int64_t init_input_dff_size = init_input_size + read_circuit.dff_size;

  sorted_list->clear();
  vector<Mark> marks(init_input_dff_size + read_circuit.gate_size,
                     Mark::UnMarked);

  // inputs are already sorted
  for (int64_t i = 0; i < init_input_dff_size; i++) {
    marks[i] = Mark::PerMarked;
    sorted_list->push_back(i);
  }

  while (true) {
    int64_t unmark_index = -1;
    // Everything is sorted.
    if (sorted_list->size() == init_input_dff_size + read_circuit.gate_size) {
      break;
    }
    for (int64_t i = 0; i < (int64_t) read_circuit.gate_size; i++) {
      //TODO: use a list to store unmarked.
      if (marks[init_input_dff_size + i] == Mark::UnMarked) {
        unmark_index = init_input_dff_size + i;
        break;
      }
    }
    if (unmark_index != -1) {  // There is an unmarked gate.
      vector<int64_t> loop;  // for detecting loop
      if (TopologicalSortVisit(read_circuit, &marks, unmark_index, sorted_list,
                               &loop) == FAILURE) {
        string loop_id_str = "";
        string loop_name_str = "";
        for (int64_t i = (int64_t) loop.size() - 1; i > 0; i--) {
          loop_name_str += read_circuit_string.gate_list_string[loop[i]
              - init_input_dff_size].output + "->";
          loop_id_str += std::to_string(loop[i] - init_input_dff_size) + "->";
        }
        loop_name_str += read_circuit_string.gate_list_string[loop[0]
            - init_input_dff_size].output;
        loop_id_str += std::to_string(loop[0] - init_input_dff_size);
        LOG(ERROR) << "Loop name: " << loop_name_str << endl << "Loop ID: "
                   << loop_id_str << endl;
        return FAILURE;
      }
    } else {
      break;  // there is no unmarked gate.
    }
  }

  CHECK_EXPR_MSG(
      sorted_list->size() == init_input_dff_size + read_circuit.gate_size,
      "Some gates are not sorted.");

  return SUCCESS;
}

int SortNetlist(ReadCircuit *read_circuit,
                const ReadCircuitString& read_circuit_string) {

  int64_t init_input_size = read_circuit->get_init_input_size();
  int64_t init_input_dff_size = init_input_size + read_circuit->dff_size;

  vector<int64_t> sorted_list;
  if (TopologicalSort(*read_circuit, &sorted_list, read_circuit_string) == FAILURE)
    return FAILURE;

  vector<int64_t> sorted_list_1(sorted_list.size());  //reverse sorted list

  for (int64_t i = 0; i < (int64_t) sorted_list.size(); i++) {
    sorted_list_1[sorted_list[i]] = i;
  }

  read_circuit->task_schedule.clear();
  read_circuit->task_schedule.resize(read_circuit->gate_list.size(), 0);
  for (int64_t i = init_input_dff_size; i < (int64_t) sorted_list.size(); i++) {
    read_circuit->task_schedule[i - init_input_dff_size] = sorted_list[i]
        - init_input_dff_size;  // align index
  }

  for (int64_t i = 0; i < (int64_t) read_circuit->dff_size; i++) {
    if (read_circuit->dff_list[i].input[0] > 0) {  // Constant values are negative
      read_circuit->dff_list[i].input[0] = sorted_list_1[read_circuit->dff_list[i]
          .input[0]];
    }
    if (read_circuit->dff_list[i].input[1] > 0) {  // Constant values are negative
      read_circuit->dff_list[i].input[1] = sorted_list_1[read_circuit->dff_list[i]
          .input[1]];
    }
    read_circuit->dff_list[i].output = sorted_list_1[read_circuit->dff_list[i]
        .output];
  }
  for (int64_t i = 0; i < (int64_t) read_circuit->gate_size; i++) {
    read_circuit->gate_list[i].output = sorted_list_1[init_input_dff_size + i];
    if (read_circuit->gate_list[i].input[0] > 0) {  // Constant values are negative
      read_circuit->gate_list[i].input[0] =
          sorted_list_1[read_circuit->gate_list[i].input[0]];
      CHECK_EXPR_MSG(
          read_circuit->gate_list[i].input[0] < read_circuit->gate_list[i].output,
          "input 0 is larger than gate id");
    }
    if (read_circuit->gate_list[i].input[1] > 0) {  // Constant values are negative
      read_circuit->gate_list[i].input[1] =
          sorted_list_1[read_circuit->gate_list[i].input[1]];
      CHECK_EXPR_MSG(
          read_circuit->gate_list[i].input[1] < read_circuit->gate_list[i].output,
          "input 1 is larger than gate id");
    }
  }
  for (int64_t i = 0; i < (int64_t) read_circuit->output_size; i++) {
    read_circuit->output_list[i] = sorted_list_1[read_circuit->output_list[i]];
  }

  if (read_circuit->terminate_id != 0) {
    read_circuit->terminate_id = sorted_list_1[read_circuit->terminate_id];
  }

  return SUCCESS;
}

int WriteMapping(const ReadCircuitString& read_circuit_string,
                 const ReadCircuit &read_circuit,
                 const string& out_mapping_filename) {

  std::ofstream f(out_mapping_filename, std::ios::out);
  if (!f.is_open()) {
    LOG(ERROR) << "can't open " << out_mapping_filename << endl;
    return FAILURE;
  }

  for (int64_t i = 0; i < (int64_t) read_circuit.dff_size; i++) {
    f << read_circuit_string.dff_list_string[i].output << " "
        << read_circuit.dff_list[i].output << endl;
  }

  for (int64_t i = 0; i < (int64_t) read_circuit.gate_size; i++) {
    int64_t gid = read_circuit.task_schedule[i];
    f << read_circuit_string.gate_list_string[gid].output << " "
        << read_circuit.gate_list[gid].output << endl;
  }

  f << "terminate " << read_circuit.terminate_id << endl;

  f.close();

  return SUCCESS;
}
