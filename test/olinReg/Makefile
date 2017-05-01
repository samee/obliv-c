testName=linReg
LOADLIBES = -lm
PORT=1234
REMOTE_HOST=localhost
CONNECTION = $(REMOTE_HOST):$(PORT)
TX = tx.dat
TY = ty.dat

$(if $(testName),,$(error 'testName' must be defined before Makefile.simple is used))
CILPATH=../../
CFLAGS=-O3 -g -Wall
./a.out: $(testName).oc $(testName).c ../../test/oblivc/common/util.c $(CILPATH)/obj/x86_LINUX/libobliv.a
	$(CILPATH)/bin/oblivcc $(CFLAGS) -I . $(testName).oc $(testName).c ../../test/oblivc/common/util.c $(LOADLIBES)

clean:
	rm -f a.out $(testName).oc.cil.i $(testName).oc.i $(testName).oc.cil.c runtime.dat

# Run tests quickly
t1:
	$(./a.out)
	./a.out $(CONNECTION) 1 $(TX)

t2:
	$(./a.out) 
	./a.out $(CONNECTION) 2 $(TY)
