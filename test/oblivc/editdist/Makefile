CILPATH=../../../
CFLAGS=-O3
a.out: editdist.oc editdist.c ../common/util.c $(CILPATH)/obj/x86_LINUX/libobliv.a
	$(CILPATH)/bin/oblivcc $(CFLAGS) -I . editdist.oc editdist.c ../common/util.c

clean:
	rm -f a.out editdist.oc.cil.i editdist.oc.i editdist.oc.cil.c
