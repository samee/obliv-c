Obliv-C Compiler (`oblivcc`)
============================

Step 1) Installation of dependencies (on Ubuntu systems) `sudo apt-get install ocaml libgcrypt20-dev ocaml-findlib`. 

Step 2) Git-clone this repository, and do a `./configure && make RELEASE=1`. 

Step 3) Start using it! The compiler is a GCC wrapper script found in `bin/oblivcc`. Example codes are in `test/oblivc`. A language tutorial is found [here](http://goo.gl/TXzxD0).

Most of this code was forked from the project CIL (C Intermediate Language). You can diff with the master branch to see which part was added on later.

Finally, if you have any questions, either open an issue here on GitHub or just send me an email at samee@virginia.edu.
