/*
 * garbled_circuit_low_mem.h
 *
 *  Created on: Oct 26, 2015
 *      Author: ebi
 */

#ifndef GARBLED_CIRCUIT_GARBLED_CIRCUIT_LOW_MEM_H_
#define GARBLED_CIRCUIT_GARBLED_CIRCUIT_LOW_MEM_H_

#include <openssl/bn.h>
#include "garbled_circuit/garbled_circuit_util.h"
#include "crypto/aes.h"

int GarbleBNLowMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                   BIGNUM* p_input, BIGNUM* g_init, BIGNUM* g_input,
                   uint64_t* clock_cycles, const string& output_mask,
                   int64_t terminate_period, OutputMode output_mode,
                   BIGNUM* output_bn, block R, block global_key,
                   bool disable_OT, int connfd);
int EvaluateBNLowMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                     BIGNUM* p_input, BIGNUM* e_init, BIGNUM* e_input,
                     uint64_t* clock_cycles, const string& output_mask,
                     int64_t terminate_period, OutputMode output_mode,
                     BIGNUM* output_bn, block global_key, bool disable_OT,
                     int connfd);
uint64_t GarbleLowMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                      BIGNUM* p_input, block* init_labels, block* input_labels,
                      GarbledTable* garbled_tables_temp,
                      GarbledTable* garbled_tables, uint64_t *garbled_table_ind,
                      block R, AES_KEY& AES_Key, uint64_t cid, int connfd,
                      BlockPair* wires, short* wires_val, BlockPair* dff_latch,
                      short* dff_latch_val, int* fanout,
                      BlockPair* terminate_label, short* terminate_val,
                      block* output_labels, short* output_vals);
uint64_t EvaluateLowMem(const GarbledCircuit& garbled_circuit, BIGNUM* p_init,
                        BIGNUM* p_input, block* init_labels,
                        block* input_labels, GarbledTable* garbled_tables,
                        uint64_t *garbled_table_ind, AES_KEY& AES_Key,
                        uint64_t cid, int connfd, block *wires,
                        short* wires_val, block *dff_latch,
                        short* dff_latch_val, int* fanout,
                        block* terminate_label, short* terminate_val,
                        block* output_labels, short* output_vals);
int GarbleAllocLabels(const GarbledCircuit& garbled_circuit,
                      block** init_labels, block** input_labels,
                      block** output_labels, short** output_vals, block R);
int GarbleGneInitLabels(const GarbledCircuit& garbled_circuit,
                        block* init_labels, block R);
int GarbleGenInputLabels(const GarbledCircuit& garbled_circuit,
                         block* input_labels, block R);
int EvaluateAllocLabels(const GarbledCircuit& garbled_circuit,
                        block** init_labels, block** input_labels,
                        block** output_labels, short** output_vals);
int GarbleTransferInitLabels(const GarbledCircuit& garbled_circuit,
                             BIGNUM* g_init, block* init_labels,
                             bool disable_OT, int connfd);
int GarbleTransferInputLabels(const GarbledCircuit& garbled_circuit,
                              BIGNUM* g_input, block* input_labels,
                              uint64_t cid, bool disable_OT, int connfd);
int EvaluateTransferInitLabels(const GarbledCircuit& garbled_circuit,
                               BIGNUM* e_init, block* init_labels,
                               bool disable_OT, int connfd);
int EvaluateTransferInputLabels(const GarbledCircuit& garbled_circuit,
                                BIGNUM* e_input, block* input_labels,
                                uint64_t cid, bool disable_OT, int connfd);
int GarbleTransferOutputLowMem(const GarbledCircuit& garbled_circuit,
                               block* output_labels, short* output_vals,
                               uint64_t cid, OutputMode output_mode,
                               const string& output_mask, BIGNUM* output_bn,
                               int connfd);
int EvaluateTransferOutputLowMem(const GarbledCircuit& garbled_circuit,
                                 block* output_labels, short* output_vals,
                                 uint64_t cid, OutputMode output_mode,
                                 const string& output_mask, BIGNUM* output_bn,
                                 int connfd);

#endif /* GARBLED_CIRCUIT_GARBLED_CIRCUIT_LOW_MEM_H_ */
