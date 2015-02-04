Folders:
  common/   - Utility files used by test cases
  editdist/ - Computes Levenstein distance between strings
  hamming/  - Computes hamming distance between strings
  million/  - Compares two int to see which is greater
  ottest/   - Various microbenchmarks for OT protocols
  psi/      - Private set intersection between sets of random ints

To run these tests (assuming the project has been already built), you should
just go to the folder and run the respective Makefile. This should give you
a self-contained 'a.out' executable. Run it without parameters to see usage 
details. E.g. to run editdist:

  $ cd editdist
  $ make
  $ ./a.out
  Port number missing
  Usage: <port> <1|2> <string>
  $ ./a.out 1234 1 leaf &
  $ ./a.out 1234 2 left
  <...>
  Result: 2

The make command simply builds 'a.out'. When you run it, it complains about
missing parameters, and then prints out the usage detail. The next two commands
actually run this. "./a.out 1234 1 leaf" says the following:
  ./a.out  - Run the edit distance function, in Yao's semi-honest protocol
  1234     - This is the TCP port number to use
  1        - Start it up in the 'party 1' mode, which means it is the generator 
             for Yao's protocol. In this case, it has also been configured to
             act as the TCP server (although this is not necessary), so that it
             listens on port 1234 for incoming connections.
  leaf     - This is the generator's private input string

At this point, the ampersand (&) at the end makes this wait in the background
for a connection. The second command starts up the client on the same machine
and plays the role of evaluator (party 2), with the input string "left". They 
both output "Result: 2" with a bunch of other stats about the runtime. 

By default, the clients are hardcoded to use "localhost" as the TCP server. To 
change that, use "make REMOTE_HOST=server.com" instead of just "make" when 
building the client in the second command. 

Currently, hamming test also asks for a <proto> parameter. For now, "yao" is the
only valid value for this parameter. We sometimes use it to test out other
protocols.
