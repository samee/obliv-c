testName=floatAdd
LOADLIBES = -lm
PORT=1234
REMOTE_HOST=localhost
CONNECTION = $(REMOTE_HOST):$(PORT)

$(if $(testName),,$(error 'testName' must be defined before Makefile.simple is used))
CILPATH=../
CFLAGS= -g -Wall

out: 
	$(CILPATH)/bin/oblivcc $(CFLAGS) -I . $(testName).oc $(testName).c

clean:
	rm -f a.out $(testName).oc.cil.i $(testName).oc.i $(testName).oc.cil.c

# Run tests quickly
t1:
	make $(out)
	./a.out $(CONNECTION) 1

t2:
	make $(out) 
	./a.out $(CONNECTION) 2