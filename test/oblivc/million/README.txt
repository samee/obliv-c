This test uses a tiny bash script `cycle` that you can find at https://github.com/samee/cmd

Simply compares two integers supplied by two parties on the command line. Doesn't even open a TCP socket, but instead uses stdio to run the protocol. For testing, we use a cyclic pipe:

# compile using our GCC wrapper
/path/to/oblivcc million.c million.oc -I .

# run: party 1 provides input value 15, party 2 provides 10
cycle './a.out 1 15 | ./a.out 2 10'
# Result output should be -1, 0, or 1 depending 
#   on less than, equal to, or greater than

# to perform this over a network, say, over SSH:
cycle './a.out 1 15 | ssh remotemachine.server.com:/remote/path/to/a.out 2 10'

# alternatively, you can run the two parties without the cycle script:
mkfifo temp_fifo
./a.out 1 15 < temp_fifo | ./a.out 2 10 > temp_fifo
rm temp_fifo
