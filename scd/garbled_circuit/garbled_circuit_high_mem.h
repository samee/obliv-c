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

#ifndef GARBLED_CIRCUIT_GARBLED_CIRCUIT_HIGH_MEM_H_
#define GARBLED_CIRCUIT_GARBLED_CIRCUIT_HIGH_MEM_H_

#include <openssl/bn.h>
#include "garbled_circuit/garbled_circuit_util.h"
#include "crypto/aes.h"

int GarbleBNHighMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                    BIGNUM* p_input, BIGNUM* g_init, BIGNUM* g_input,
                    uint64_t* clock_cycles, const string& output_mask,
                    int64_t terminate_period, OutputMode output_mode,
                    BIGNUM* output_bn, block R, block global_key,
                    bool disable_OT, int connfd);
int EvaluateBNHighMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                      BIGNUM* p_input, BIGNUM* e_init, BIGNUM* e_input,
                      uint64_t* clock_cycles, const string& output_mask,
                      int64_t terminate_period, OutputMode output_mode,
                      BIGNUM* output_bn, block global_key, bool disable_OT,
                      int connfd);
int GarbleHighMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                  BIGNUM* p_input, block* init_labels, block* input_labels,
                  block global_key, block R, uint64_t* clock_cycles,
                  int64_t terminate_period, int connfd, block* output_labels,
                  short* output_vals);
int EvaluateHighMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                    BIGNUM* p_input, block* init_labels, block* input_labels,
                    block global_key, uint64_t* clock_cycles,
                    int64_t terminate_period, int connfd, block* output_labels,
                    short* output_vals);
int GarbleOT(const GarbledCircuit& garbled_circuit, block* init_labels,
             block* input_labels, uint64_t clock_cycles, int connfd);
int EvalauteOT(const GarbledCircuit& garbled_circuit, BIGNUM* e_init,
               block* init_labels, BIGNUM* e_input, block* input_labels,
               uint64_t clock_cycles, int connfd);
int GarbleTransferLabels(const GarbledCircuit& garbled_circuit, BIGNUM* g_init,
                         block* init_labels, BIGNUM* g_input,
                         block* input_labels, uint64_t clock_cycles,
                         bool disable_OT, int connfd);
int EvaluateTransferLabels(const GarbledCircuit& garbled_circuit,
                           BIGNUM* e_init, block* init_labels, BIGNUM* e_input,
                           block* input_labels, uint64_t clock_cycles,
                           bool disable_OT, int connfd);
int GarbleMakeLabels(const GarbledCircuit& garbled_circuit, block** init_labels,
                     block** input_labels, block** output_labels,
                     short** output_vals, block R, uint64_t clock_cycles);
int EvaluateMakeLabels(const GarbledCircuit& garbled_circuit,
                       block** init_labels, block** input_labels,
                       block** output_labels, short** output_vals,
                       uint64_t clock_cycles);
int GarbleTransferOutput(const GarbledCircuit& garbled_circuit,
                         block* output_labels, short * output_vals,
                         uint64_t clock_cycles, const string& output_mask,
                         OutputMode output_mode, BIGNUM* output_bn, int connfd);
int EvaluateTransferOutput(const GarbledCircuit& garbled_circuit,
                           block* output_labels, short* output_vals,
                           uint64_t clock_cycles, const string& output_mask,
                           OutputMode output_mode, BIGNUM* output_bn,
                           int connfd);

#endif /* GARBLED_CIRCUIT_GARBLED_CIRCUIT_HIGH_MEM_H_ */
