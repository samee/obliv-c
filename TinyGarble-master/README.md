[![Build Status](https://travis-ci.org/esonghori/TinyGarble.svg?branch=develop)](https://travis-ci.org/esonghori/TinyGarble)

TinyGarble
=======
TinyGarble is a full implementation of 
[Yao's Garbled Circuit (GC) protocol](https://en.wikipedia.org/wiki/Garbled_Circuit) for
two-party Secure Function Evaluation (SFE) in which the parties are able to
execute any function on their private inputs and learn the output without
leaking any information about their inputs.
This repository consists of two main parts: (1) circuit synthesis (output examples 
of this is stored in `scd/netlist/v.tar.bz` and will be unzipped and translated in 
`bin/scd/netlist/` after `make`) and (2) secure function evaluation.
Circuit synthesis is partially described in TinyGarble paper in IEEE S&P'15 (see
References). It is based on upon hardware synthesis and sequential circuit
concept and outputs a netlist Verilog (`.v`) file (not included in this repository). 
The other part of TinyGarble, hereafter called "TinyGarble", is a GC framework 
implemented based on [JustGarble](http://cseweb.ucsd.edu/groups/justgarble/)
project. Beside Free-XOR, Row-reduction, OT extension, and
Fixed-key block cipher, TinyGarble includes Half Gates which is the most recent
optimization on GC protocol and reduces the communication by 33%.
TinyGarble also includes communication and Oblivious Transfer (OT) which were
missing in JustGarble. Note that OT is a crucial part for the security of the GC 
protocol.

TinyGarble general flow:
1. Write a Verilog file (`.v`) describing the function.
2. Synthesis the Verilog file using TinyGarble's *circuit synthesis* to generate
a netlist Verilog file (`.v`).
3. Translate the netlist file (`.v`) to a simple circuit description file
(`.scd`) using TinyGarble's `V2SCD_Main` and then provide both parties with the
file.
4. Execute `TinyGarble` using `--alice` flag on one party and `--bob` flag
on the other plus other appropriate arguments.

Any questions or comments: [Ebrahim Songhori](mailto:e.songhori@gmail.com)

## TinyGarble

### Dependencies
Install dependencies: g++, OpenSSL (1.0.1f <), boost(1.55.0 <), and cmake
(3.1.0 <). On Ubuntu:

* g++:
```
	$ sudo apt-get install g++
```
* OpenSSL:
```
	$ sudo apt-get install libssl-dev
```
* boost:
```
  $ sudo apt-get install libboost-all-dev
```
* cmake:
```
  $ sudo apt-get install software-properties-common
  $ sudo add-apt-repository ppa:george-edison55/cmake-3.x
  $ sudo apt-get update
  $ sudo apt-get upgrade
  $ sudo apt-get install cmake
```

### Compile
Configure TinyGarble and then compile it in `bin` directory (for debug mode, use 
`cmake ..` inside `bin` directory before `make`):
```
  $ ./configure
  $ cd bin
  $ make
```

### Run an example
For finding Hamming distance between two 32-bit private inputs (e.g.,
Alice: FF55AA77, Bob: 12345678), on Alice's terminal, run:
```
  $ bin/garbled_circuit/TinyGarble --alice --scd_file bin/scd/netlists/hamming_32bit_1cc.scd --input FF55AA77
```
And on Bob's terminal, run:
```
  $ bin/garbled_circuit/TinyGarble --bob --scd_file bin/scd/netlists/hamming_32bit_1cc.scd --input 12345678
```
Note that, it is supposed that Alice and Bob are in a same mahcine
(server_ip = 127.0.0.1) in this example.
The expected output is 13 in hexadecimal which is the hamming distance between
the two numbers. For showing more detailes, you may use `--log2std` option.

### Test
In `bin` directory call `ctest`:
```
	$ ctest -V
```

### Binaries
#### Main binary
* `V2SCD_Main`: Translating netlist Verilog (`.v`) file to simple circuit
description (`.scd`) file.
```
  -h [ --help ]                         produce help message.
  -i [ --netlist ]
                                        Input netlist (verilog .v) file
                                        address.
  -o [ --scd ]
                                        Output simple circuit description (scd)
                                        file address.

```
* `garbled_circuit/TinyGarble`: TinyGarble main binary:
```
  -h [ --help ]                         produce help message
  -a [ --alice ]                        Run as Alice (server).
  -b [ --bob ]                          Run as Bob (client).
  -i [ --scd_file ]                     Simple circuit description (.scd) file
                                        address.
  -p [ --port ] arg (=1234)             socket port
  -s [ --server_ip ] arg (=127.0.0.1)   Server's (Alice's) IP, required when
                                        running as Bob.
  --init arg (=0)                       Hexadecimal init for initializing DFFs.
  --input arg (=0)                      Hexadecimal input.
  --clock_cycles arg (=1)               Number of clock cycles to evaluate the
                                        circuit.
  --dump_directory arg                  Directory for dumping memory hex files.
  --disable_OT                          Disable Oblivious Transfer (OT) for
                                        transferring labels. WARNING: OT is
                                        crucial for GC security.
  --low_mem_foot                        Enables low memory footprint mode for
                                        circuits with multiple clock cycles. In
                                        this mode, OT is called at each clock
                                        cycle which degrades the performance.
  --output_mask arg (=0)                Hexadecimal mask for output. 0
                                        indicates that output belongs to Bob,
                                        and 1 belongs to Alice.
  --output_mode arg (=0)                0: normal, 1:separated by clock 2:last
                                        clock.
```
#### Other binary
* `scd/SCD_Evaluator_Main`: Evaluating a simple circuit description (`.scd`) file:
```
  -h [ --help ]                         produce help message
  -i [ --scd_file ] 					scd address
  --clock_cycles arg (=1)               Number of clock cycles to evaluate the
                                        circuit.
  --g_init arg (=0)                     g_init in hexadecimal.
  --e_init arg (=0)                     e_init in hexadecimal.
  --g_input arg (=5)                    g_input in hexadecimal.
  --e_input arg (=4)                    e_input in hexadecimal.
  --output_mode arg (=0)                0: normal, 1:separated by clock 2:last
                                        clock.
```
* `crypto/OT_Main`: Oblivious Transfer binary:
```
  -h [ --help ]                         produce help message
  -a [ --alice ]                        Run as Alice (server).
  --message0 arg (=15141312_11100908_07060504_03020100)
                                        Alice's 128-bit message 0 in
                                        hexadecimal w/o '0x'.
  --message1 arg (=00010203_04050607_08091011_12131415)
                                        Alice's 128-bit message 1 in
                                        hexadecimal w/o '0x'.
  --select arg (=0)                     Bob's 1-bit selection (0/1).
  -b [ --bob ]                          Run as Bob (client).
  -p [ --port ] arg (=1234)             socket port
  -s [ --server_ip ] arg (=127.0.0.1)   Server's (Alice's) IP, required when
                                        running as Bob.
```
#### Test binary
1. `Util_Test`
2. `TCPIP_Test`
3. `BN_Test`
4. `OT_Test`
5. `OT_Extension_Test`
6. `SCD_Evaluator_Test`
7. `Garbled_Circuit_Test`

## References
- Ebrahim M. Songhori, Siam U. Hussain, Ahmad-Reza Sadeghi, Thomas Schneider
and Farinaz Koushanfar, ["TinyGarble: Highly Compressed and Scalable Sequential
Garbled Circuits."](http://esonghori.github.io/file/TinyGarble.pdf) <i>Security
and Privacy, 2015 IEEE Symposium on</i> May, 2015.
- Mihir Bellare, Viet Tung Hoang, Sriram Keelveedhi, and Phillip Rogaway.
Efficient garbling from a fixed-key blockcipher. In <i>S&P</i>, pages 478–492.
IEEE, 2013.
- Samee Zahur, Mike Rosulek, and David Evans. ["Two halves make a whole:
Reducing data transfer in garbled circuits using half
gates."](http://eprint.iacr.org/2014/756)
In <i>Eurocrypt, 2015</i>.
- G. Asharov, Y. Lindell, T. Schneider and M. Zohner: More Efficient Oblivious
Transfer and Extensions for Faster Secure Computation In <i>CCS'13</i>.
