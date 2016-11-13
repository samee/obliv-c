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

#ifndef SCD_SCHEDULING_H_
#define SCD_SCHEDULING_H_

#include "scd/v_2_scd.h"

int SortNetlist(ReadCircuit* read_circuit,
                const ReadCircuitString& read_circuit_string);
int WriteMapping(const ReadCircuitString& read_circuit_string,
                 const ReadCircuit &read_circuit,
                 const string& out_mapping_filename);

#endif /* SCD_SCHEDULING_H_ */
