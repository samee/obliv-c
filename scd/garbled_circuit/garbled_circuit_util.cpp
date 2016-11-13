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

#include "garbled_circuit/garbled_circuit_util.h"

#include "tcpip/tcpip.h"
#include "util/common.h"
#include "util/log.h"
#include "util/util.h"

using std::endl;

uint64_t NumOfNonXor(const GarbledCircuit& garbled_circui) {
  uint64_t num_of_non_xor = 0;
  for (uint64_t i = 0; i < garbled_circui.gate_size; i++) {
    if (garbled_circui.garbledGates[i].type != XORGATE
        && garbled_circui.garbledGates[i].type != XNORGATE
        && garbled_circui.garbledGates[i].type != NOTGATE) {
      num_of_non_xor++;
    }
  }
  return num_of_non_xor;
}

void RemoveGarbledCircuit(GarbledCircuit *garbled_circuit) {
  delete[] garbled_circuit->garbledGates;
  if (garbled_circuit->outputs != nullptr) {
    delete[] garbled_circuit->outputs;
  }
  if (garbled_circuit->I != nullptr) {
    delete[] garbled_circuit->I;
  }
  if (garbled_circuit->D != nullptr) {
    delete[] garbled_circuit->D;
  }
}

int ParseInitInputStr(const string& init_str, const string&input_str,
                      uint64_t init_size, uint64_t input_size,
                      uint64_t clock_cycles, BIGNUM** init, BIGNUM** input) {
  BN_hex2bn(init, init_str.c_str());
  BN_hex2bn(input, input_str.c_str());

  return SUCCESS;
}

inline void HalfGarbleEvalGateKnownValue(int type, int knwon_wire_ind,
                                         short input_value,
                                         short* output_value) {
  bool x0, x1;
  if (knwon_wire_ind == 0) {
    x0 = GateOperator(type, input_value, 0);
    x1 = GateOperator(type, input_value, 1);
  } else {
    x0 = GateOperator(type, 0, input_value);
    x1 = GateOperator(type, 1, input_value);
  }

  if (x0 == x1) {
    if (x0 == 0) {
      *output_value = 0;
    } else {
      *output_value = 1;
    }
  } else {
    *output_value = SECRET;
  }
}

inline void HalfGarbleGate(int type, int knwon_wire_ind, short input_value,
                           BlockPair input_sec_labels, BlockPair* output_labels,
                           short* output_value) {
  bool x0, x1;
  if (knwon_wire_ind == 0) {
    x0 = GateOperator(type, input_value, 0);
    x1 = GateOperator(type, input_value, 1);
  } else {
    x0 = GateOperator(type, 0, input_value);
    x1 = GateOperator(type, 1, input_value);
  }

  if (x0 == x1) {
    if (x0 == 0) {
      *output_value = 0;
    } else {
      *output_value = 1;
    }
  } else if (x0 == 0 && x1 == 1) {
    output_labels->label0 = input_sec_labels.label0;
    output_labels->label1 = input_sec_labels.label1;
  } else /*if(x0 == 1 && x1 == 0)*/{
    output_labels->label0 = input_sec_labels.label1;
    output_labels->label1 = input_sec_labels.label0;
  }
}

void GarbleEvalGateKnownValue(short input0_value, short input1_value, int type,
                              short* output_value) {
  if (!IsSecret(input0_value) && !IsSecret(input1_value)) {
    *output_value = GateOperator(type, input0_value, input1_value);
  } else if (!IsSecret(input0_value)) {
    HalfGarbleEvalGateKnownValue(type, 0, input0_value, output_value);
  } else if (!IsSecret(input1_value)) {
    HalfGarbleEvalGateKnownValue(type, 1, input1_value, output_value);
  } else {
    *output_value = SECRET;
  }
}

void GarbleGate(BlockPair input0_labels, short input0_value,
                BlockPair input1_labels, short input1_value, int type,
                uint64_t cid, uint64_t gid, GarbledTable* garbled_tables,
                uint64_t* garbled_table_ind, block R, AES_KEY AES_Key,
                BlockPair* output_labels, short* output_value) {

  *output_value = SECRET;
  output_labels->label0 = ZeroBlock();
  output_labels->label1 = ZeroBlock();

  if (!IsSecret(input0_value) && !IsSecret(input1_value)) {
    *output_value = GateOperator(type, input0_value, input1_value);

  } else if (!IsSecret(input0_value)) {
    HalfGarbleGate(type, 0, input0_value, input1_labels, output_labels,
                   output_value);
  } else if (!IsSecret(input1_value)) {
    HalfGarbleGate(type, 1, input1_value, input0_labels, output_labels,
                   output_value);
  } else {

    if (type != NOTGATE && CmpBlock(input0_labels.label0, input1_labels.label0)) {
      // equal secret values as inputs

      if (type == XORGATE || type == ANDNGATE || type == NORNGATE) {
        *output_value = 0;
      } else if (type == XNORGATE || type == ORNGATE || type == NANDNGATE) {
        *output_value = 1;
      } else if (type == ANDGATE || type == ORGATE) {
        output_labels->label0 = input0_labels.label0;
        output_labels->label1 = input0_labels.label1;
      } else if (type == NANDGATE || type == NORGATE) {
        output_labels->label0 = input0_labels.label1;
        output_labels->label1 = input0_labels.label0;
      }

    } else if (type != NOTGATE
        && CmpBlock(input0_labels.label0, input1_labels.label1)) {
      // inverted secret values as input
      if (type == XNORGATE || type == ANDGATE || type == NORGATE) {
        *output_value = 0;
      } else if (type == XORGATE || type == ORGATE || type == NANDGATE) {
        *output_value = 1;
      } else if (type == ANDNGATE || type == ORNGATE) {
        output_labels->label0 = input0_labels.label0;
        output_labels->label1 = input0_labels.label1;
      } else if (type == NANDNGATE || type == NORNGATE) {
        output_labels->label0 = input0_labels.label1;
        output_labels->label1 = input0_labels.label0;
      }

    } else {

      if (type == XORGATE) {
        output_labels->label0 = XorBlock(input0_labels.label0,
                                         input1_labels.label0);
        output_labels->label1 = XorBlock(input0_labels.label1,
                                         input1_labels.label0);
      } else if (type == XNORGATE) {
        output_labels->label0 = XorBlock(input0_labels.label1,
                                         input1_labels.label0);
        output_labels->label1 = XorBlock(input0_labels.label0,
                                         input1_labels.label0);
      } else if (type == NOTGATE) {
        output_labels->label0 = input0_labels.label1;
        output_labels->label1 = input0_labels.label0;
      } else {
        block A1;
        block A0;
        unsigned short v = Type2V(type);
        if (v & 1) {
          A1 = (input0_labels.label0);
          A0 = (input0_labels.label1);
        } else {
          A0 = (input0_labels.label0);
          A1 = (input0_labels.label1);
        }

        block B1;
        block B0;
        if (v & 2) {
          B1 = (input1_labels.label0);
          B0 = (input1_labels.label1);
        } else {
          B0 = (input1_labels.label0);
          B1 = (input1_labels.label1);
        }

        unsigned short pa = get_LSB(A0);
        unsigned short pb = get_LSB(B0);

        block tweak0 = MakeBlock(cid, 2 * gid + 0);
        block tweak1 = MakeBlock(cid, 2 * gid + 1);

        block keys[4];
        keys[0] = XorBlock(A0, tweak0);
        keys[1] = XorBlock(A1, tweak0);
        keys[2] = XorBlock(B0, tweak1);
        keys[3] = XorBlock(B1, tweak1);

        block mask[4];
        mask[0] = keys[0];
        mask[1] = keys[1];
        mask[2] = keys[2];
        mask[3] = keys[3];
        AESEcbEncryptBlks(keys, 4, &(AES_Key));

        mask[0] = XorBlock(mask[0], keys[0]);
        mask[1] = XorBlock(mask[1], keys[1]);
        mask[2] = XorBlock(mask[2], keys[2]);
        mask[3] = XorBlock(mask[3], keys[3]);

        block table[2];
        table[0] = XorBlock(mask[0], mask[1]);
        if (pb) {
          table[0] = XorBlock(table[0], R);
        }
        block G = mask[0];
        if (pa) {
          G = XorBlock(G, table[0]);
        }
        table[1] = XorBlock(mask[2], mask[3]);
        table[1] = XorBlock(table[1], A0);

        block E = mask[2];
        if (pb) {
          E = XorBlock(E, table[1]);
          E = XorBlock(E, A0);
        }

        block C0;
        block C1;
        if (v & 4) {
          C1 = XorBlock(G, E);
          C0 = XorBlock(R, C1);
        } else {
          C0 = XorBlock(G, E);
          C1 = XorBlock(R, C0);
        }

        output_labels->label0 = C0;
        output_labels->label1 = C1;

        garbled_tables[(*garbled_table_ind)].row[0] = table[0];
        garbled_tables[(*garbled_table_ind)].row[1] = table[1];
        garbled_tables[(*garbled_table_ind)].gid = gid;
        (*garbled_table_ind)++;
      }
    }
  }
}

inline short InvertSecretValue(short value) {
  if (value == SECRET)
    return SECRET_INV;
  else
    return SECRET;
}

inline short XorSecret(short a, short b) {
  if (a == b) {
    return SECRET;
  } else {
    return SECRET_INV;
  }
}

inline void HalfEvalGate(int type, int knwon_wire_ind, short input_value,
                         short input_sec_value, block input_sec_labels,
                         block* output_labels, short* output_value) {

  bool x0, x1;
  if (knwon_wire_ind == 0) {
    x0 = GateOperator(type, input_value, 0);
    x1 = GateOperator(type, input_value, 1);
  } else {
    x0 = GateOperator(type, 0, input_value);
    x1 = GateOperator(type, 1, input_value);
  }

  if (x0 == x1) {
    if (x0 == 0) {
      *output_value = 0;
    } else {
      *output_value = 1;
    }
  } else if (x0 == 0 && x1 == 1) {
    *output_value = input_sec_value;
    *output_labels = input_sec_labels;
  } else /* if (x0 == 1 && x1 == 0) */{
    *output_value = InvertSecretValue(input_sec_value);
    *output_labels = input_sec_labels;
  }
}

void EvalGate(block input0_labels, short input0_value, block input1_labels,
              short input1_value, int type, uint64_t cid, uint64_t gid,
              GarbledTable* garbled_tables, uint64_t* garbled_table_ind,
              AES_KEY AES_Key, block* output_labels, short* output_value) {

  *output_value = SECRET;
  *output_labels = ZeroBlock();

  if (!IsSecret(input0_value) && !IsSecret(input1_value)) {
    *output_value = GateOperator(type, input0_value, input1_value);
  } else if (!IsSecret(input0_value)) {
    HalfEvalGate(type, 0, input0_value, input1_value, input1_labels,
                 output_labels, output_value);
  } else if (!IsSecret(input1_value)) {
    HalfEvalGate(type, 1, input1_value, input0_value, input0_labels,
                 output_labels, output_value);
  } else {

    if (type != NOTGATE && CmpBlock(input0_labels, input1_labels)
        && input0_value == input1_value) {
      // equal secret values as inputs

      if (type == XORGATE || type == ANDNGATE || type == NORNGATE) {
        *output_value = 0;
      } else if (type == XNORGATE || type == ORNGATE || type == NANDNGATE) {
        *output_value = 1;
      } else if (type == ANDGATE || type == ORGATE) {
        *output_value = input0_value;
        *output_labels = input0_labels;
      } else if (type == NANDGATE || type == NORGATE) {
        *output_value = InvertSecretValue(input0_value);
        *output_labels = input0_labels;
      }

    } else if (type != NOTGATE && CmpBlock(input0_labels, input1_labels)
        && input0_value != input1_value) {
      // inverted secret values as input
      if (type == XNORGATE || type == ANDGATE || type == NORGATE) {
        *output_value = 0;
      } else if (type == XORGATE || type == ORGATE || type == NANDGATE) {
        *output_value = 1;
      } else if (type == ANDNGATE || type == ORNGATE) {
        *output_value = input0_value;
        *output_labels = input0_labels;
      } else if (type == NANDNGATE || type == NORNGATE) {
        *output_value = InvertSecretValue(input0_value);
        *output_labels = input0_labels;
      }

    } else {

      if (type == XORGATE) {
        *output_value = XorSecret(input0_value, input1_value);
        *output_labels = XorBlock(input0_labels, input1_labels);
      } else if (type == XNORGATE) {
        *output_value = InvertSecretValue(
            XorSecret(input0_value, input1_value));
        *output_labels = XorBlock(input0_labels, input1_labels);
      } else if (type == NOTGATE) {
        *output_value = InvertSecretValue(input0_value);
        *output_labels = input0_labels;
      } else {
        block A = (input0_labels);
        block B = (input1_labels);

        unsigned short sa = get_LSB(A);
        unsigned short sb = get_LSB(B);

        block tweak0 = MakeBlock(cid, 2 * gid + 0);
        block tweak1 = MakeBlock(cid, 2 * gid + 1);

        block table[2];
        if (garbled_tables[(*garbled_table_ind)].gid == gid) {  // correct table
          table[0] = garbled_tables[(*garbled_table_ind)].row[0];
          table[1] = garbled_tables[(*garbled_table_ind)].row[1];
          (*garbled_table_ind)++;
          DUMP("table") << table[0] << endl;
          DUMP("table") << table[1] << endl;
        } else {  // the gate will not be used in output.
          table[0] = RandomBlock();
          table[1] = RandomBlock();
        }

        block keys[2];
        keys[0] = XorBlock(A, tweak0);
        keys[1] = XorBlock(B, tweak1);

        block mask[2];
        mask[0] = keys[0];
        mask[1] = keys[1];
        AESEcbEncryptBlks(keys, 2, &(AES_Key));

        mask[0] = XorBlock(mask[0], keys[0]);
        mask[1] = XorBlock(mask[1], keys[1]);

        block G = mask[0];
        if (sa) {
          G = XorBlock(G, table[0]);
        }

        block E = mask[1];
        if (sb) {
          E = XorBlock(E, table[1]);
          E = XorBlock(E, A);
        }

        block C = XorBlock(E, G);

        *output_labels = C;
      }
    }
  }
}

int FillFanout(GarbledCircuit* garbled_circuit) {

  int64_t gate_bias = (int64_t) garbled_circuit->get_gate_lo_index();
  for (int64_t gid = 0; gid < (int64_t) garbled_circuit->gate_size; gid++) {
    GarbledGate& garbledGate = garbled_circuit->garbledGates[gid];

    garbledGate.fanout = 0;

    int64_t input0 = garbledGate.input0;
    int64_t input1 = garbledGate.input1;

    if (input0 >= gate_bias) {
      CHECK_EXPR((input0 - gate_bias < gid));
      garbled_circuit->garbledGates[input0 - gate_bias].fanout++;
    }
    if (input1 >= gate_bias) {
      CHECK_EXPR((input1 - gate_bias < gid));
      garbled_circuit->garbledGates[input1 - gate_bias].fanout++;
    }
  }

  for (uint64_t did = 0; did < garbled_circuit->dff_size; did++) {
    int64_t gid = garbled_circuit->D[did] - gate_bias;
    if (gid >= 0) {
      garbled_circuit->garbledGates[gid].fanout++;
    }
  }

  for (uint64_t oid = 0; oid < garbled_circuit->output_size; oid++) {
    int64_t gid = garbled_circuit->outputs[oid] - gate_bias;
    // if output is not directly connected to DFF's Q or inputs.
    if (gid >= 0) {
      garbled_circuit->garbledGates[gid].fanout++;
    }
  }

  if (garbled_circuit->terminate_id > 0) {
    int64_t gid = garbled_circuit->terminate_id - gate_bias;
    // if terminate signal is not directly connected to DFF's Q or inputs.
    if (gid >= 0) {
      garbled_circuit->garbledGates[gid].fanout++;
    }
  }

  for (int64_t gid = 0; gid < (int64_t) garbled_circuit->gate_size; gid++) {
    CHECK_EXPR_MSG(
        (garbled_circuit->garbledGates[gid].fanout > 0),
        "Non-positive fanout initialization: gid = " + std::to_string(gid));
  }

  return SUCCESS;
}

void ReduceFanout(const GarbledCircuit& garbled_circuit, int *fanout,
                  int64_t wid, int64_t gate_bias) {

  int64_t gid = wid - gate_bias;
  if (gid >= 0 && gid < (int64_t) garbled_circuit.gate_size
      && fanout[gid] > 0) {
    fanout[gid]--;
    if (fanout[gid] == 0) {
      ReduceFanout(garbled_circuit, fanout,
                   garbled_circuit.garbledGates[gid].input0, gate_bias);
      ReduceFanout(garbled_circuit, fanout,
                   garbled_circuit.garbledGates[gid].input1, gate_bias);
    }
  }
}

void PrintPredecessorsGarble(const GarbledCircuit& garbled_circuit,
                             BlockPair *wires, int64_t cid, int64_t gid) {
  if (gid < 0) {
    return;
  }
  GarbledGate& garbledGate = garbled_circuit.garbledGates[gid];
  int64_t input0 = garbledGate.input0;
  int64_t input1 = garbledGate.input1;
  int64_t output = garbledGate.output;

  BlockPair input0_labels = { ZeroBlock(), ZeroBlock() };
  BlockPair input1_labels = { ZeroBlock(), ZeroBlock() };
  if (input0 >= 0 && input0 < (int64_t) garbled_circuit.get_wire_size()) {
    input0_labels = wires[input0];
  }
  if (input1 >= 0 && input1 < (int64_t) garbled_circuit.get_wire_size()) {
    input1_labels = wires[input1];
  }
  BlockPair output_labels = wires[output];

  LOG(INFO) << "g @" << cid << " gid = " << gid << endl << "i0: "
      << input0_labels.label0 << " - " << input0_labels.label1 << endl << "i1: "
      << input1_labels.label0 << " - " << input1_labels.label1 << endl << "o : "
      << output_labels.label0 << " - " << output_labels.label1 << endl;

  if (!CmpBlock(input0_labels.label0, ZeroBlock())) {
    int64_t input0_gid = input0 - garbled_circuit.get_gate_lo_index();
    PrintPredecessorsGarble(garbled_circuit, wires, cid, input0_gid);
  }
  if (!CmpBlock(input1_labels.label0, ZeroBlock())) {
    int64_t input1_gid = input1 - garbled_circuit.get_gate_lo_index();
    PrintPredecessorsGarble(garbled_circuit, wires, cid, input1_gid);
  }

}

void PrintPredecessorsEval(const GarbledCircuit& garbled_circuit, block *wires,
                           int64_t cid, int64_t gid) {
  if (gid < 0) {
    return;
  }
  GarbledGate& garbledGate = garbled_circuit.garbledGates[gid];
  int64_t input0 = garbledGate.input0;
  int64_t input1 = garbledGate.input1;
  int64_t output = garbledGate.output;

  block input0_labels = ZeroBlock();
  block input1_labels = ZeroBlock();
  if (input0 >= 0 && input0 < (int64_t) garbled_circuit.get_wire_size()) {
    input0_labels = wires[input0];
  }
  if (input1 >= 0 && input1 < (int64_t) garbled_circuit.get_wire_size()) {
    input1_labels = wires[input1];
  }
  block output_labels = wires[output];

  LOG(INFO) << "e @" << cid << " gid = " << gid << endl << "i0: "
      << input0_labels << endl << "i1: " << input1_labels << endl << "o : "
      << output_labels << endl;

  if (!CmpBlock(input0_labels, ZeroBlock())) {
    int64_t input0_gid = input0 - garbled_circuit.get_gate_lo_index();
    PrintPredecessorsEval(garbled_circuit, wires, cid, input0_gid);
  }
  if (!CmpBlock(input1_labels, ZeroBlock())) {
    int64_t input1_gid = input1 - garbled_circuit.get_gate_lo_index();
    PrintPredecessorsEval(garbled_circuit, wires, cid, input1_gid);
  }

}

int GarbleTransferTerminate(const GarbledCircuit& garbled_circuit,
                            const BlockPair &terminate_label,
                            short terminate_val, bool* is_terminate,
                            int connfd) {
  if (terminate_val == 0) {
    (*is_terminate) = false;
  } else if (terminate_val == 1) {
    (*is_terminate) = true;
  } else {
    short terminate_type = get_LSB(terminate_label.label0);
    CHECK(SendData(connfd, &terminate_type, sizeof(short)));
    short eval_terminate_type;
    CHECK(RecvData(connfd, &eval_terminate_type, sizeof(short)));
    (*is_terminate) = (terminate_type == eval_terminate_type);
  }

  return SUCCESS;
}

int EvaluateTransferTerminate(const GarbledCircuit& garbled_circuit,
                              const block &terminate_label, short terminate_val,
                              bool* is_terminate, int connfd) {
  if (terminate_val == 0) {
    (*is_terminate) = false;
  } else if (terminate_val == 1) {
    (*is_terminate) = true;
  } else {
    short garble_terminate_type;
    CHECK(RecvData(connfd, &garble_terminate_type, sizeof(short)));
    short terminate_type = get_LSB(terminate_label);
    CHECK(RecvData(connfd, &terminate_type, sizeof(short)));
    (*is_terminate) = (terminate_type == garble_terminate_type);
  }

  return SUCCESS;
}
