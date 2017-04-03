Folders:
  common/     - Utility files used by test cases
  editdist/   - Computes Levenstein distance between strings
  hamming/    - Computes hamming distance between strings
  million/    - Compares two int to see which is greater
  aes/        - Computes AES of a single block (with key expansion)
  ottest/     - Various microbenchmarks for OT protocols
  psi/        - Private set intersection between sets of random ints
  protosplit/ - Tests for multithreading and protocol splitting

To run these tests (assuming the project has been already built), you should
just go to the folder and run the respective Makefile. This should give you
a self-contained 'a.out' executable. Run it without parameters to see usage 
details. E.g. to run editdist:

  $ cd editdist
  $ make
  $ ./a.out
  Port number missing
  Usage: ./a.out <port> <--|remote_host> <string>
  $ ./a.out 1234 -- leaf &
  $ ./a.out 1234 localhost left
  <...>
  Result: 2

The make command simply builds 'a.out'. When you run it, it complains about
missing parameters, and then prints out the usage detail. The next two commands
actually run this. "./a.out 1234 -- leaf" says the following:
  ./a.out  - Run the edit distance function, in Yao's semi-honest protocol
  1234     - This is the TCP port number to use
  --       - Start it up in the server mode, and start listening on port 1234
             for incoming connections. In this case, it has also been
             configured to act as the generator in Yao's protocol.
  leaf     - This is the generator's private input string

At this point, the ampersand (&) at the end makes this wait in the background
for a connection. The second command starts up the client on the same machine
and connects to the server on "localhost", port "1234". It uses the string
"left" as its input. As a client, it is configured to act as the evaluator in
the protocol.  They both output "Result: 2" with a bunch of other stats about
the runtime. 

Currently, hamming test also asks for a <proto> parameter. For now, "yao" is the
only valid value for this parameter. We sometimes use it to test out other
protocols. In general, try to run the program without any parameters or grep the
.c file for "Usage" string to figure out how to use a given test program.
