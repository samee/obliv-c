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

#ifndef GARBLED_CIRCUIT_GARBLED_CIRCUIT_H_
#define GARBLED_CIRCUIT_GARBLED_CIRCUIT_H_

#include <cstdint>
#include <string>
#include <openssl/bn.h>
#include "crypto/block.h"
#include "garbled_circuit/garbled_circuit_util.h"

using std::string;

int GarbleStr(const string& scd_file_address, const string& p_init_str,
              const string& p_input_str, const string& init_str,
              const string& input_str, uint64_t clock_cycles,
              const string& output_mask, int64_t terminate_period,
              OutputMode output_mode, bool disable_OT, bool low_mem_foot,
              string* output_str, int connfd);
int EvaluateStr(const string& scd_file_address, const string& p_init_str,
                const string& p_input_str, const string& init_str,
                const string& input_str, uint64_t clock_cycles,
                const string& output_mask, int64_t terminate_period,
                OutputMode output_mode, bool disable_OT, bool low_mem_foot,
                string* output_str, int connfd);

#endif /* GARBLED_CIRCUIT_GARBLED_CIRCUIT_H_ */
