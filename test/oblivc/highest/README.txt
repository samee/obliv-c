Prints out the highest value that is entered by two parties on the command line

# compile using our GCC wrapper
/path/to/oblivcc highest.c highest.oc -I .
# run: party 1 enters the numbers 1,2, and 3 and party 2 entered the numbers 2,4, and 6
cycle './a.out 1 1 2 3 | ./a.out 2 2 4 6'
#this result will be 6

The cycle command is a handy bash script that you can find at https://github.com/samee/cmd
