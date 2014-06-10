all:
	~/obliv-c/bin/oblivcc mutual.c mutual.oc -I ./ -lrt

test:
	./cycle './a.out 1 alice1.txt | ./a.out 2 bob1.txt'
