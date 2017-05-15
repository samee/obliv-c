Obliv-C Secure Computation Compiler (`oblivcc`)
===============================================

Obliv-C is a simple GCC wrapper that makes it easy to embed secure computation protocols inside regular C programs. The idea is simple: if you are performing a multi-party distributed computation with sensitive data, just write it in our Obliv-C langauge and compile/link it with your project. The result will be a secure cryptographic protocol that performs this operation without revealing any of the inputs or intermediate values of the computation to any of the parties. Only the outputs are finally shared.

# Installation

Step 1) Installation of dependencies (on Ubuntu systems) `sudo apt-get install ocaml libgcrypt20-dev ocaml-findlib`. 

Step 2) Git-clone this repository, and do a `./configure && make RELEASE=1`. 

Step 3) Start using it! The compiler is a GCC wrapper script found in `bin/oblivcc`. Example codes are in `test/oblivc`. A language tutorial is found [here](http://goo.gl/TXzxD0).

Most of this code was forked from the project CIL (C Intermediate Language). You can diff with the master branch to see which part was added on later.

# Benchmarks

This repository includes several example programs using Obliv-C in the `test/oblivc` directory.  See [obliv-c/test/oblivc/README.txt](https://github.com/uvasrg/obliv-c/blob/obliv-c/test/oblivc/README.txt) for details. 


# Extending the Language

In order to extend the language to, for example, add support for double precision floating point numbers, three kinds of changes need to be made.

1) The preprocessor needs to be modified to recognize any new keywords, "obliv double" for example. These files need to be modified:
    - [src/ext/processObliv/processObliv.ml](src/ext/processObliv/processObliv.ml)
    - [src/ext/processObliv/oblivUtils.ml](src/ext/processObliv/oblivUtils.ml) (may not need changes)
    - [src/cil.ml](src/cil.ml)

2) The Obliv-C extension needs to be modified to correctly process the new additions. These files need to be modified:
    - [src/ext/oblivc/obliv_bits.c](src/ext/oblivc/obliv_bits.c)
    - [src/ext/oblivc/obliv_bits.h](src/ext/oblivc/obliv_bits.h)
    - [src/ext/oblivc/obliv_common.h](src/ext/oblivc/obliv_common.h)
    - [src/ext/oblivc/obliv_bits.c](src/ext/oblivc/obliv_bits.c)
    - [src/ext/oblivc/obliv_types_internal.h](src/ext/oblivc/obliv_types_internal.h) (may not need changes)
    - [src/ext/oblivc/obliv_types.h](src/ext/oblivc/obliv_types.h) (may not need changes)
    - [src/ext/oblivc/obliv.h](src/ext/oblivc/obliv.h) (may not need changes)

3) If a new data type is added, functions acting on that data type will need to be added (for example, plus for a double data type). For each of the functions, a circuit needs to generated. The process for generating the circuit is as follows:
    - Obtain a high-level implementation of the desired function in C or C++
    - Use the [TinyGarble](https://github.com/esonghori/TinyGarble) tool-chain to obtain a Simple Circuit Description file (SCD)
    - Use the include [SCDtoObliv/SCDtoObliv.py](SCDtoObliv/SCDtoObliv.py) program to convert the SCD file to an Obliv-C circuit. Add a header and "footer" to the resulting file (use [src/ext/oblivc/obliv_float_add.c](src/ext/oblivc/obliv_float_add.c) as an example), and add that file to the [src/ext/oblivc](src/ext/oblivc) folder.
    - Add linkings to the circuit as appropriate in the files listed in step two (again, use the floating point circuit functions as an example).

4) Add tests to the test folder to ensure that the added functionality works as intended.


# Help

Please let us know if you are using Obliv-C. If you have any questions, either open an issue here on GitHub or just send me an email at samee@virginia.edu.
