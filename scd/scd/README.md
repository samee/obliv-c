Simple Circuit Description (SCD)
=======
The initial idea of a Simple Circuit Description (SCD) was proposed in JustGarble 
paper (S&P'13) to represent a acyclic Boolean circuit. TinyGarble paper (S&P'15)
proposed a modified version of SCD which supports a sequential circuit with 
flip-flops.

## Circuit Format
TinyGarble's V2SCD accepts Verilog netlist circuits with a special format. 
The circuit's ports should be in {`clk`, `rst`, `g_init`, `e_init`, 
`g_input`,`e_input`, `o`} where `clk` is clock cycle, `rst` is active high 
reset, `g_init` is garbler's (Alice) initial values, `e_init` is evaluator's 
(Bob) initial values, `g_input` is garbler's inputs, `e_input` is evaluator's 
input, and `o` is the output port.

`g_init` and `e_init` ports will be read only at the first clock cycle 
and must be connected to Flip-Flops’ `I` (initial) ports.
`g_input` and `e_input` posts will be read at every clock cycle, thus their 
bit-width should be multiplied by number of clock cycles.
It is also true for `o` port which will be provided at every clock cycle.

## Wire Indexing
Wires are indexed according to this order:
1- g_init
2- e_init
3- g_input
4- e_input
5- gates' output (A gate's output index is same as the gate's index plus the 
	gate output offset which is equal to size of init and input wires.)  

## SCD Format
Unlike JustGarble's SCD, TinyGarble's SCD is in ASCII format and human-readable.
The format consists of seven lines:
1- `g_init_size`, `e_init_size`, `g_input_size`, `e_input_size`, 
	`dff_size`, `output_size`, and `gate_size`.
2- gate's `input0` index
3- gate's `input1` index
4- gate's `type` (defined in [util/common.h](util/common.h))
5- `outputs` index
6- Flip-Flop's `D` (data wire index) 
7- Flip-Flop's `I` (initial value index chosen from `g_init` and `e_init`).

## References
- Mihir Bellare, Viet Tung Hoang, Sriram Keelveedhi, and Phillip Rogaway.
Efficient garbling from a fixed-key blockcipher. In <i>S&P</i>, pages 478–492.
IEEE, 2013. 
- Ebrahim M. Songhori, Siam U. Hussain, Ahmad-Reza Sadeghi, Thomas Schneider
and Farinaz Koushanfar, ["TinyGarble: Highly Compressed and Scalable Sequential
Garbled Circuits."](http://esonghori.github.io/file/TinyGarble.pdf) <i>Security
and Privacy, 2015 IEEE Symposium on</i> May, 2015.

