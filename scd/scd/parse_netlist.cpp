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

#include "scd/parse_netlist.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <fstream>
#include <map>
#include "scd/scheduling.h"
#include "util/common.h"
#include "util/log.h"

using std::ifstream;
using boost::char_separator;
using boost::tokenizer;
using std::map;
using std::pair;

string Type2StrGate(short itype) {
  string type;
  if (itype == ANDGATE) {
    type = "AND";
  } else if (itype == ANDNGATE) {
    type = "ANDN";
  } else if (itype == NANDGATE) {
    type = "NAND";
  } else if (itype == NANDNGATE) {
    type = "NANDN";
  } else if (itype == ORGATE) {
    type = "OR";
  } else if (itype == ORNGATE) {
    type = "ORN";
  } else if (itype == NORGATE) {
    type = "NOR";
  } else if (itype == NORNGATE) {
    type = "NORN";
  } else if (itype == XORGATE) {
    type = "XOR";
  } else if (itype == XNORGATE) {
    type = "XNOR";
  } else if (itype == NOTGATE) {
    type = "IV";
  } else if (itype == DFFGATE) {
    type = "DFF";
  } else {
    type = "NOTVALID";
  }
  return type;
}

int ParseNetlist(const string &filename,
                 ReadCircuitString* read_circuit_string) {

  ifstream fin(filename.c_str(), std::ios::in);
  if (!fin.good()) {
    LOG(ERROR) << "file not found:" << filename << endl;
    return -1;
  }

  short gate_type = INVALGATE;
  uint64_t no_of_bits = 0;
  bool is_right_assignmnet = false;
  bool is_left_assignmnet = false;
  bool is_inport = false;
  bool is_outport = false;
  bool endoffile = false;

  map<string, string> port;
  string port_key;

  while (!endoffile) {
    CHECK_EXPR_MSG(fin.good(), "File is broken, no endmodule found.");
    string line = "";
    getline(fin, line);
    char_separator<char> sep(" ,()\t\r", ";");
    tokenizer<char_separator<char> > tok(line, sep);
    BOOST_FOREACH(string str, tok){
    if(!str.compare(";")) {
      if(gate_type == ANDGATE || gate_type == ANDNGATE ||
          gate_type == NANDGATE|| gate_type == NANDNGATE ||
          gate_type == ORGATE || gate_type == ORNGATE ||
          gate_type == NORGATE || gate_type == NORNGATE ||
          gate_type == XORGATE || gate_type == XNORGATE) {

        CHECK_EXPR_MSG(port.count("A") > 0 && port["A"]!="",
            "A is missing: " + line);
        CHECK_EXPR_MSG(port.count("B") > 0 && port["B"]!="",
            "B is missing: " + line);
        CHECK_EXPR_MSG(port.count("Z") > 0 && port["Z"]!="",
            "Z is missing: " + line);

        ReadGateString g;
        g.type = gate_type;
        g.input[0] = port["A"];
        g.input[1] = port["B"];
        g.output = port["Z"];
        read_circuit_string->gate_list_string.push_back(g);
        port.clear();
        gate_type = INVALGATE;
      } else if(gate_type == NOTGATE) {
        CHECK_EXPR_MSG(port.count("A") > 0 && port["A"]!="",
            "A is missing: " + line);
        CHECK_EXPR_MSG(port.count("Z") > 0 && port["Z"]!="",
            "Z is missing: " + line);;

        ReadGateString g;
        g.type = gate_type;
        g.input[0] = port["A"];
        g.input[1] = "";
        g.output = port["Z"];
        read_circuit_string->gate_list_string.push_back(g);
        port.clear();
        gate_type = INVALGATE;
      } else if(gate_type == DFFGATE) {
        CHECK_EXPR_MSG(port.count("D") > 0 && port["D"]!="",
            "D is missing: " + line);
        CHECK_EXPR_MSG(port.count("I") > 0 && port["I"]!="",
            "I is missing: " + line);
        CHECK_EXPR_MSG(port.count("Q") > 0 && port["Q"]!="",
            "Q is missing: " + line);

        ReadGateString g;
        g.type = gate_type;
        g.input[0] = port["D"];
        g.input[1] = port["I"];
        g.output = port["Q"];
        read_circuit_string->dff_list_string.push_back(g);
        port.clear();
        gate_type = INVALGATE;
      } else if (gate_type == MUXGATE) {

        CHECK_EXPR_MSG(port.count("IN0") > 0 && port["IN0"]!="",
            "IN0 is missing: " + line);
        CHECK_EXPR_MSG(port.count("IN1") > 0 && port["IN1"]!="",
            "IN1 is missing: " + line);
        CHECK_EXPR_MSG(port.count("SEL") > 0 && port["SEL"]!="",
            "SEL is missing: " + line);
        CHECK_EXPR_MSG(port.count("F") > 0 && port["F"]!="",
            "F is missing: " + line);

        ReadGateString g1, g2, g3;

        uint64_t gate_id = read_circuit_string->gate_list_string.size();

        g1.type = XORGATE;
        g1.input[0] = port["IN0"];
        g1.input[1] = port["IN1"];
        g1.output = "MUX_MID_1_" + std::to_string(gate_id);

        g2.type = ANDGATE;
        g2.input[0] = port["SEL"];
        g2.input[1] = g1.output;
        g2.output = "MUX_MID_2_" + std::to_string(gate_id);

        g3.type = XORGATE;
        g3.input[0] = port["IN0"];
        g3.input[1] = g2.output;
        g3.output = port["F"];

        read_circuit_string->gate_list_string.push_back(g1);
        read_circuit_string->gate_list_string.push_back(g2);
        read_circuit_string->gate_list_string.push_back(g3);

        port.clear();
        gate_type = INVALGATE;

      } else if (gate_type == HADDERGATE) {

        CHECK_EXPR_MSG(port.count("IN0") > 0 && port["IN0"]!="",
            "IN0 is missing: " + line);
        CHECK_EXPR_MSG(port.count("IN1") > 0 && port["IN1"]!="",
            "IN1 is missing: " + line);
        CHECK_EXPR_MSG((port.count("SUM") > 0 && port["SUM"]!="") ||
            (port.count("COUT") > 0 && port["COUT"]!=""),
            "SUM or COUT is missing: " + line);

        if(port.count("SUM") > 0 ) {
          ReadGateString g;
          g.type = XORGATE;
          g.input[0] = port["IN0"];
          g.input[1] = port["IN1"];
          g.output = port["SUM"];
          read_circuit_string->gate_list_string.push_back(g);
        }
        if(port.count("COUT") > 0) {
          ReadGateString g;
          g.type = ANDGATE;
          g.input[0] = port["IN0"];
          g.input[1] = port["IN1"];
          g.output = port["COUT"];
          read_circuit_string->gate_list_string.push_back(g);
        }

        port.clear();
        gate_type = INVALGATE;
      } else if (gate_type == FADDERGATE) {

        CHECK_EXPR_MSG(port.count("IN0") > 0 && port["IN0"]!="",
            "IN0 is missing: " + line);
        CHECK_EXPR_MSG(port.count("IN1") > 0 && port["IN1"]!="",
            "IN1 is missing: " + line);
        CHECK_EXPR_MSG(port.count("CIN") > 0 && port["CIN"]!="",
            "CIN is missing: " + line);
        CHECK_EXPR_MSG((port.count("SUM") > 0 && port["SUM"]!="") ||
            (port.count("COUT") > 0 && port["COUT"]!=""),
            "SUM or COUT is missing: " + line);

        uint64_t gate_id = read_circuit_string->gate_list_string.size();

        ReadGateString g1;
        g1.type = XORGATE;
        g1.input[0] = port["IN0"];
        g1.input[1] = port["CIN"];
        g1.output = "FADDER_MID_1_" + std::to_string(gate_id);
        read_circuit_string->gate_list_string.push_back(g1);

        if(port.count("SUM") > 0) {
          ReadGateString g2;

          g2.type = XORGATE;
          g2.input[0] = port["IN1"];
          g2.input[1] = g1.output;
          g2.output = port["SUM"];

          read_circuit_string->gate_list_string.push_back(g2);
        }
        if(port.count("COUT") > 0) {
          ReadGateString g2,g3,g4;

          g2.type = XORGATE;
          g2.input[0] = port["IN1"];
          g2.input[1] = port["CIN"];
          g2.output = "FADDER_MID_2_" + std::to_string(gate_id);

          g3.type = ANDGATE;
          g3.input[0] = g1.output;
          g3.input[1] = g2.output;
          g3.output = "FADDER_MID_3_" + std::to_string(gate_id);

          g4.type = XORGATE;
          g4.input[0] = port["CIN"];
          g4.input[1] = g3.output;
          g4.output = port["COUT"];
          read_circuit_string->gate_list_string.push_back(g2);
          read_circuit_string->gate_list_string.push_back(g3);
          read_circuit_string->gate_list_string.push_back(g4);
        }

        port.clear();
        gate_type = INVALGATE;
      } else {
        is_inport = 0;
        is_outport = 0;
        no_of_bits = 0;
        is_left_assignmnet = false;
        is_right_assignmnet = false;
      }
      port_key = "";
    } else if(!str.compare("endmodule")) {
      endoffile = true;
      break;
    } else if(!str.compare("input")) {
      is_inport = 1;
    } else if(is_inport) {
      if (str.at(0) =='[') {
        tokenizer<> bits(str);
        tokenizer<>::iterator beg = bits.begin();
        string bits_str(*beg);
        no_of_bits = atoi(bits_str.c_str())+1;
      } else if(str.compare("clk") && str.compare("rst")) {
        if(!str.compare("p_init")) {
          read_circuit_string->p_init_size = (no_of_bits>0)?no_of_bits:1;
        } else if(!str.compare("g_init")) {
          read_circuit_string->g_init_size = (no_of_bits>0)?no_of_bits:1;
        } else if(!str.compare("e_init")) {
          read_circuit_string->e_init_size = (no_of_bits>0)?no_of_bits:1;
        } else if(!str.compare("p_input")) {
          read_circuit_string->p_input_size = (no_of_bits>0)?no_of_bits:1;
        } else if(!str.compare("g_input")) {
          read_circuit_string->g_input_size = (no_of_bits>0)?no_of_bits:1;
        } else if(!str.compare("e_input")) {
          read_circuit_string->e_input_size = (no_of_bits>0)?no_of_bits:1;
        } else {
          LOG(ERROR) << "The input name is not valid " << str << endl <<
          "valid choice: { p_init, g_init, e_init, "
          "p_input, g_input, e_input}: " << line << endl;
          return FAILURE;
        }
      }
    } else if(!str.compare("output")) {
      is_outport = 1;
    } else if(is_outport) {
      if (str.at(0) =='[') {
        tokenizer<> bits(str);
        tokenizer<>::iterator beg = bits.begin();
        string bits_str(*beg);
        no_of_bits = atoi(bits_str.c_str())+1;
      } else if(!str.compare("terminate")) {
        if(no_of_bits != 0) {
          LOG(ERROR) << "Terminate signal should be 1-bit output." << endl;
          return FAILURE;
        }
        read_circuit_string->has_terminate = true;
      } else if(!str.compare("o")) {
        read_circuit_string->output_size = (no_of_bits>0)?no_of_bits:1;
      } else {
        LOG(ERROR) << "The output name is not valid " << str << endl <<
        "valid choice: { o, terminate}" << endl;
        return FAILURE;
      }
    } else if(!str.compare("assign")) {
      is_left_assignmnet = true;
    } else if(is_left_assignmnet) {
      read_circuit_string->assignment_list_string.push_back(
          std::make_pair(str, ""));
      is_left_assignmnet = false;
    } else if(!str.compare("=")) {
      is_right_assignmnet = true;
    } else if(is_right_assignmnet) {
      const auto assign_pair = read_circuit_string->assignment_list_string.back();
      read_circuit_string->assignment_list_string.pop_back();
      read_circuit_string->assignment_list_string.push_back(
          std::make_pair(assign_pair.first, str));
      is_right_assignmnet = false;
    } else if(!str.compare("AND")) {
      gate_type = ANDGATE;
    } else if(!str.compare("ANDN")) {
      gate_type = ANDNGATE;
    } else if(!str.compare("NAND")) {
      gate_type = NANDGATE;
    } else if(!str.compare("NANDN")) {
      gate_type = NANDNGATE;
    } else if(!str.compare("OR")) {
      gate_type = ORGATE;
    } else if(!str.compare("ORN")) {
      gate_type = ORNGATE;
    } else if(!str.compare("NOR")) {
      gate_type = NORGATE;
    } else if(!str.compare("NORN")) {
      gate_type = NORNGATE;
    } else if(!str.compare("XOR")) {
      gate_type= XORGATE;
    } else if(!str.compare("XNOR")) {
      gate_type = XNORGATE;
    } else if(!str.compare("IV")) {
      gate_type = NOTGATE;
    } else if(!str.compare("DFF")) {
      gate_type = DFFGATE;
    } else if(!str.compare("MUX")) {
      gate_type = MUXGATE;
    } else if(!str.compare("HADDER")) {
      gate_type = HADDERGATE;
    } else if(!str.compare("FADDER")) {
      gate_type = FADDERGATE;
    } else if (!str.compare(".A")) {
      port_key = "A";
    } else if (!str.compare(".B")) {
      port_key = "B";
    } else if(!str.compare(".D")) {
      port_key = "D";
    } else if(!str.compare(".I")) {
      port_key = "I";
    } else if (!str.compare(".Z")) {
      port_key = "Z";
    } else if (!str.compare(".Q")) {
      port_key = "Q";
    } else if (!str.compare(".IN0")) {
      port_key = "IN0";
    } else if (!str.compare(".IN1")) {
      port_key = "IN1";
    } else if (!str.compare(".CIN")) {
      port_key = "CIN";
    } else if (!str.compare(".SEL") ) {
      port_key = "SEL";
    } else if (!str.compare(".COUT")) {
      port_key = "COUT";
    } else if ( !str.compare(".SUM")) {
      port_key = "SUM";
    } else if (!str.compare(".F")) {
      port_key = "F";
    } else if (!str.compare(".CLK")) {
      port_key = "";
    } else if (!str.compare(".RST")) {
      port_key = "";
    } else if(port_key != "") {
      // fix Synopsys 2015 bug:
      // space after '['
      if(str.at(0) =='[') {
        port[port_key] += str;
        port_key = "";
      } else {
        port[port_key] = str;
      }
    }
  }
}
  return SUCCESS;
}

void AddWireArray(map<string, int64_t>& wire_name_table, const string& name,
                  uint64_t size, int64_t *wire_index) {
  if (size == 1) {
    wire_name_table.insert(pair<string, int64_t>(name, *wire_index));
    wire_name_table.insert(
        pair<string, int64_t>(name + "[0]", (*wire_index)++));  // some cases, it is w[0]
  } else {
    for (uint i = 0; i < size; ++i) {
      wire_name_table.insert(
          pair<string, int64_t>(name + "[" + std::to_string(i) + "]",
                                (*wire_index)++));
    }
  }
}

string GetBristWire(uint64_t w, uint64_t wire_size,
                    const ReadCircuitString& read_circuit_string) {
  if (w < read_circuit_string.g_input_size) {
    return "g_input[" + std::to_string(w) + "]";
  } else if (w
      < read_circuit_string.g_input_size + read_circuit_string.e_input_size) {
    uint64_t ind = w - read_circuit_string.g_input_size;
    return "e_input[" + std::to_string(ind) + "]";
  } else if (w >= wire_size - read_circuit_string.output_size) {
    uint64_t ind = w - wire_size + read_circuit_string.output_size;
    return "o[" + std::to_string(ind) + "]";
  }
  return "w" + std::to_string(w);
}

int ParseBrisNetlist(const string &filename,
                     ReadCircuitString* read_circuit_string) {

  ifstream fin(filename.c_str(), std::ios::in);
  if (!fin.good()) {
    LOG(ERROR) << "file not found:" << filename << endl;
    return -1;
  }

  uint64_t gate_size, wire_size;
  fin >> gate_size >> wire_size;

  CHECK_EXPR_MSG(gate_size != 0 && wire_size != 0,
                 "Number of gate or wire are zero.");

  fin >> read_circuit_string->g_input_size >> read_circuit_string->e_input_size
      >> read_circuit_string->output_size;

  for (uint64_t gid = 0; gid < gate_size; gid++) {
    string g_type;
    uint64_t inpu_num, output_num, i0, i1, o;
    //2 1 0 32 406 XOR
    fin >> inpu_num >> output_num >> i0;
    if (inpu_num == 1) {
      i1 = 0;
      fin >> o >> g_type;
    } else if (inpu_num == 2) {
      fin >> i1 >> o >> g_type;
    }

    ReadGateString g;
    g.input[0] = GetBristWire(i0, wire_size, *read_circuit_string);
    if (inpu_num > 1) {
      g.input[1] = GetBristWire(i1, wire_size, *read_circuit_string);
    } else {
      g.input[1] = "";
    }
    g.output = GetBristWire(o, wire_size, *read_circuit_string);

    if (g_type == "INV") {
      g.type = NOTGATE;
    } else if (g_type == "XOR") {
      g.type = XORGATE;
    } else if (g_type == "AND") {
      g.type = ANDGATE;
    } else {
      LOG(ERROR) << "Unknown gate type: " << g_type << endl;
      return FAILURE;
    }

    read_circuit_string->gate_list_string.push_back(g);
  }

  return SUCCESS;
}

int IdAssignment(const ReadCircuitString& read_circuit_string,
                 ReadCircuit* read_circuit) {
  read_circuit->p_init_size = read_circuit_string.p_init_size;
  read_circuit->g_init_size = read_circuit_string.g_init_size;
  read_circuit->e_init_size = read_circuit_string.e_init_size;
  read_circuit->p_input_size = read_circuit_string.p_input_size;
  read_circuit->g_input_size = read_circuit_string.g_input_size;
  read_circuit->e_input_size = read_circuit_string.e_input_size;

  read_circuit->dff_size = read_circuit_string.dff_list_string.size();
  read_circuit->gate_size = read_circuit_string.gate_list_string.size();
  read_circuit->output_size = read_circuit_string.output_size;

  int64_t wire_index = 0;
  map<string, int64_t> wire_name_table;

  AddWireArray(wire_name_table, "p_init", read_circuit->p_init_size,
               &wire_index);
  AddWireArray(wire_name_table, "g_init", read_circuit->g_init_size,
               &wire_index);
  AddWireArray(wire_name_table, "e_init", read_circuit->e_init_size,
               &wire_index);
  AddWireArray(wire_name_table, "p_input", read_circuit->p_input_size,
               &wire_index);
  AddWireArray(wire_name_table, "g_input", read_circuit->g_input_size,
               &wire_index);
  AddWireArray(wire_name_table, "e_input", read_circuit->e_input_size,
               &wire_index);

  for (uint64_t i = 0; i < read_circuit->dff_size; i++) {
    wire_name_table.insert(
        pair<string, int64_t>(read_circuit_string.dff_list_string[i].output,
                              wire_index++));  //DFF Qs
  }

  for (uint i = 0; i < read_circuit->gate_size; i++) {
    wire_name_table.insert(
        pair<string, int64_t>(read_circuit_string.gate_list_string[i].output,
                              wire_index++));  // gates' output
  }
  wire_name_table.insert(pair<string, int64_t>("", ((uint64_t) - 1)));
  wire_name_table.insert(pair<string, int64_t>("1'b0", CONST_ZERO));
  wire_name_table.insert(pair<string, int64_t>("1'b1", CONST_ONE));

  for (uint64_t i = 0; i < read_circuit_string.assignment_list_string.size();
      i++) {
    const auto assign_pair = read_circuit_string.assignment_list_string[i];
    if (wire_name_table.count(assign_pair.first)) {
      wire_name_table[assign_pair.second] = wire_name_table[assign_pair.first];
    } else if (wire_name_table.count(assign_pair.second)) {
      wire_name_table[assign_pair.first] = wire_name_table[assign_pair.second];
    } else {
      LOG(ERROR) << "Can not find wire " << assign_pair.first << " or "
          << assign_pair.second
          << " which were mentioned in an assignment statement" << endl;
    }
  }

  read_circuit->gate_list.resize(read_circuit->gate_size);
  read_circuit->output_list.resize(read_circuit->output_size);
  read_circuit->dff_list.resize(read_circuit->dff_size);

  for (uint64_t i = 0; i < read_circuit->gate_size; i++) {
    read_circuit->gate_list[i].type = read_circuit_string.gate_list_string[i]
        .type;
    CHECK_EXPR_MSG(
        wire_name_table.count(read_circuit_string.gate_list_string[i].input[0])
            != 0,
        read_circuit_string.gate_list_string[i].input[0] + " "
            + read_circuit_string.gate_list_string[i].output);
    CHECK_EXPR_MSG(
        wire_name_table.count(read_circuit_string.gate_list_string[i].input[1])
            != 0,
        read_circuit_string.gate_list_string[i].input[1]);
    CHECK_EXPR_MSG(
        wire_name_table.count(read_circuit_string.gate_list_string[i].output)
            != 0,
        read_circuit_string.gate_list_string[i].output);
    read_circuit->gate_list[i].input[0] = wire_name_table[read_circuit_string
        .gate_list_string[i].input[0]];
    read_circuit->gate_list[i].input[1] = wire_name_table[read_circuit_string
        .gate_list_string[i].input[1]];
    read_circuit->gate_list[i].output = wire_name_table[read_circuit_string
        .gate_list_string[i].output];
  }

  for (uint64_t i = 0; i < read_circuit->dff_size; i++) {
    read_circuit->dff_list[i].type =
        read_circuit_string.dff_list_string[i].type;

    CHECK_EXPR_MSG(
        wire_name_table.count(read_circuit_string.dff_list_string[i].input[0])
            != 0,
        read_circuit_string.dff_list_string[i].input[0]);
    CHECK_EXPR_MSG(
        wire_name_table.count(read_circuit_string.dff_list_string[i].input[1])
            != 0,
        read_circuit_string.dff_list_string[i].input[1]);
    CHECK_EXPR_MSG(
        wire_name_table.count(read_circuit_string.dff_list_string[i].output)
            != 0,
        read_circuit_string.dff_list_string[i].output);

    read_circuit->dff_list[i].input[0] = wire_name_table[read_circuit_string
        .dff_list_string[i].input[0]];
    read_circuit->dff_list[i].input[1] = wire_name_table[read_circuit_string
        .dff_list_string[i].input[1]];
    read_circuit->dff_list[i].output = wire_name_table[read_circuit_string
        .dff_list_string[i].output];
  }

  for (uint64_t i = 0; i < read_circuit->output_size; i++) {
    if (read_circuit->output_size == 1) {
      CHECK_EXPR(
          wire_name_table.count("o[0]") != 0 || wire_name_table.count("o") != 0);

      if (wire_name_table.count("o[0]")) {
        read_circuit->output_list[i] = wire_name_table["o[0]"];
      } else {
        read_circuit->output_list[i] = wire_name_table["o"];
      }
    } else {
      CHECK_EXPR_MSG(wire_name_table.count("o[" + std::to_string(i) + "]") != 0,
                     "o[" + std::to_string(i) + "]");
      read_circuit->output_list[i] = wire_name_table["o[" + std::to_string(i)
          + "]"];
    }
  }

  if (read_circuit_string.has_terminate) {
    CHECK_EXPR(wire_name_table["terminate"] != 0);
    read_circuit->terminate_id = wire_name_table["terminate"];
  }

  return SUCCESS;
}
