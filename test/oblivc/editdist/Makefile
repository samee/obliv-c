CFLAGS="-O3"

a.out: editdist.c editdist.oc editdist.h ../../../obj/x86_LINUX/libobliv.a
	../../../bin/oblivcc -I . editdist.c editdist.oc $(CFLAGS)
clean:
	rm -f editdist.oc.cil.c editdist.oc.cil.i editdist.oc.i a.out
