OBLIVCC=../../bin/oblivcc
RUNTIME=../../_build/libobliv.a
PORT=1234
REMOTE_HOST=localhost
CONNECTION = $(REMOTE_HOST):$(PORT)
CILPATH=../../
CFLAGS=-g -Wall
LOADLIBES=-lm
testName=linreg
TX=tx.dat
TY=ty.dat

./a.out: $(testName).oc $(testName).c ../oblivc/common/util.c $(RUNTIME)
	$(OBLIVCC) $(CFLAGS) -I . $(testName).oc $(testName).c ../oblivc/common/util.c $(LOADLIBES)

clean:
	rm -f a.out $(testName).oc.cil.i $(testName).oc.i $(testName).oc.cil.c

# Run tests quickly
t1:
	$(./a.out)
	./a.out $(CONNECTION) 1 $(TX)

t2:
	$(./a.out) 
	./a.out $(CONNECTION) 2 $(TY)
