Obliv-C Secure Computation Compiler (`oblivcc`)
===============================================

Obliv-C is a simple GCC wrapper that makes it easy to embed secure computation protocols inside regular C programs. The idea is simple: if you are performing a multi-party distributed computation with sensitive data, just write it in our Obliv-C langauge and compile/link it with your project. The result will be a secure cryptographic protocol that performs this operation without revealing any of the inputs or intermediate values of the computation to any of the parties. Only the outputs are finally shared.

# Installation
Unfortunately, the instructions for Fedora and Mac OS might be a little out of date, since we discovered OPAM is now a dependency (#49). Please ping us if you figured out how to install Obliv-C on those systems.

1. Installation of dependencies. 
  * For Ubuntu: `sudo apt-get install ocaml libgcrypt20-dev ocaml-findlib opam m4`.
  * For Mac OS (with Macports): `sudo port install gcc5 ocaml ocaml-findlib opam libgcrypt +devel`.
  * For Fedora: these instructions may be outdated. They worked as of [commit 6553c1f7c](https://github.com/samee/obliv-c/commit/6553c1f7c), but we haven't tested it since [OPAM](https://opam.ocaml.org/) became a hard dependency on other platforms. There doesn't seem to be a well-supported RPM for it either. Please let us know if that has changed, and we will update this text here. Here is the old instruction for reference: `sudo dnf install glibc-devel.i686 ocaml ocaml-ocamldoc ocaml-findlib ocaml-findlib-devel libgcrypt libgcrypt-devel perl-ExtUtils-MakeMaker perl-Data-Dumper`.

2. If you are using OPAM as our package manager, and this is the first time you are using it, set it up:
   ```
   opam init
   opam switch 4.06.0
   eval `opam config env`
   opam install camlp4 ocamlfind ocamlbuild batteries
   ```
   Version 4.06.0 just happened to be the most recent version when we tested. You can check what you have by running `opam switch list`, and try a more recent one.

3. Git-clone this repository, and compile:
  * For Linux: `./configure && make`.
  * For Mac OS (with Macports): `CC=/opt/local/bin/gcc-mp-5 CPP=/opt/local/bin/cpp-mp-5 LIBRARY_PATH=/opt/local/lib ./configure && make`.

4. Start using it! The compiler is a GCC wrapper script found in `bin/oblivcc`. Example codes are in `test/oblivc`. A language tutorial is found [here](http://goo.gl/TXzxD0).

Most of this code was forked from the project CIL (C Intermediate Language). You can diff with the master branch to see which part was added on later.

# Benchmarks

This repository includes several example programs using Obliv-C in the `test/oblivc` directory.  See [obliv-c/test/oblivc/README.txt](https://github.com/uvasrg/obliv-c/blob/obliv-c/test/oblivc/README.txt) for details. 

# Help

Please let us know if you are using Obliv-C.  If you have any questions, either open an issue here on GitHub or just send me an email at sza4uq@virginia.edu.
