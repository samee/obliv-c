Obliv-C Secure Computation Compiler (`oblivcc`)
===============================================

Obliv-C is a simple GCC wrapper that makes it easy to embed secure computation protocols inside regular C programs. The idea is simple: if you are performing a multi-party distributed computation with sensitive data, just write it in our Obliv-C langauge and compile/link it with your project. The result will be a secure cryptographic protocol that performs this operation without revealing any of the inputs or intermediate values of the computation to any of the parties. Only the outputs are finally shared.

# Installation

1. Installation of dependencies:
  * For Ubuntu: `sudo apt-get install ocaml libgcrypt20-dev ocaml-findlib ocaml-ocamldoc`.
  * For Fedora: `sudo dnf install glibc-devel.i686 ocaml ocaml-ocamldoc ocaml-findlib ocaml-findlib-devel libgcrypt libgcrypt-devel perl-ExtUtils-MakeMaker perl-Data-Dumper`

2. Git-clone this repository, and do a `./configure && make RELEASE=1`. 

3. Start using it! The compiler is a GCC wrapper script found in `bin/oblivcc`. Example codes are in `test/oblivc`. A language tutorial is found [here](http://goo.gl/TXzxD0).

Most of this code was forked from the project CIL (C Intermediate Language). You can diff with the master branch to see which part was added on later.

# Benchmarks

This repository includes several example programs using Obliv-C in the `test/oblivc` directory.  See [obliv-c/test/oblivc/README.txt](https://github.com/uvasrg/obliv-c/blob/obliv-c/test/oblivc/README.txt) for details. 

# Help

Please let us know if you are using Obliv-C.  If you have any questions, either open an issue here on GitHub or just send me an email at samee@virginia.edu.
