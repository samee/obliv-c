CILPATH=../../../
CFLAGS=-O3
a.out: psi.oc psi.c waksman.c waksman.h ../common/util.c $(CILPATH)/obj/x86_LINUX/libobliv.a
	$(CILPATH)/bin/oblivcc $(CFLAGS) -I . psi.oc psi.c waksman.c ../common/util.c

clean:
	rm -f a.out psi.oc.cil.c psi.oc.cil.i psi.oc.i
