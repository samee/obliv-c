Obliv-C Compiler (`oblivcc`)
============================

This readme is still under construction. But there is already a language tutorial at http://goo.gl/TXzxD0

This requires libgcrypt to run. this can be downloaded on Ubuntu with `sudo apt-get install libgcrypt` then it also requires libgcrypt11-dev, which can be downlaoded with `sudo apt-get install libgcrypt11-dev`.

Once in the source folder, a simple `./configure && make` should build it all. The compiler is a GCC wrapper script found in `bin/oblivcc`. Example codes are in `test/oblivc`

Most of this code was forked from the project CIL (C Intermediate Language). You can diff with the master branch to see which part was added on later.

Finally, if you have any questions, either open an issue here on GitHub or just send me an email at samee@virginia.edu.
