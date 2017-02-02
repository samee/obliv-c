OBLIVCC=../bin/oblivcc
RUNTIME=../_build/libobliv.a
PORT=1234
REMOTE_HOST=localhost
CONNECTION = $(REMOTE_HOST):$(PORT)
CILPATH=../
CFLAGS=-g -Wall
LOADLIBES=-lm
testName=testFloatAdd

./a.out: $(testName).oc $(testName).c ../test/oblivc/common/util.c $(RUNTIME)
	$(OBLIVCC) $(CFLAGS) -I . $(testName).oc $(testName).c ../test/oblivc/common/util.c $(LOADLIBES)

clean:
	rm -f a.out $(testName).oc.cil.i $(testName).oc.i $(testName).oc.cil.c

# Run tests quickly
t1:
	$(./a.out)
	./a.out $(CONNECTION) 1

t2:
	$(./a.out) 
	./a.out $(CONNECTION) 2
