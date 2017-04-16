#pragma once
#include <obliv_types_internal.h>

void obliv_float_add_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2);
void obliv_float_sub_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2);
void obliv_float_mult_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2);
void obliv_float_div_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2);
void obliv_float_eq_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2);
void obliv_float_le_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2);
void obliv_float_lt_circuit(OblivBit* dest, const OblivBit* op1, const OblivBit* op2);
