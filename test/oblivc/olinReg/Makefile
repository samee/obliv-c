testName=linReg
LOADLIBES = -lm
$(if $(testName),,$(error 'testName' must be defined before Makefile.simple is used))
CILPATH=../../../
REMOTE_HOST=localhost
CFLAGS=-O3 -g -DREMOTE_HOST=$(REMOTE_HOST) -Wall
./a.out: $(testName).oc $(testName).c ../common/util.c $(CILPATH)/obj/x86_LINUX/libobliv.a
	$(CILPATH)/bin/oblivcc $(CFLAGS) -I . $(testName).oc $(testName).c ../common/util.c $(LOADLIBES)

clean:
	rm -f a.out $(testName).oc.cil.i $(testName).oc.i $(testName).oc.cil.c runtime.dat
