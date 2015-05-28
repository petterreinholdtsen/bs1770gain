include config.mak
include $(SRCDIR)/version.mak
vpath % $(SRCDIR)
.PHONY: all install clean

CC=gcc
CPPFLAGS+=-I$(SRCDIR)
CPPFLAGS+=-I$(INCLUDEDIR)
CFLAGS+=-O2
CFLAGS+=-Wall
CFLAGS+=-Werror

LIB1770_A=lib1770-2.a

LIB1770_H+=$(INCLUDEDIR)/pbutil.h
LIB1770_H+=$(INCLUDEDIR)/pbutil_priv.h
LIB1770_H+=lib1770.h

LIB1770_O+=$(LIB1770_A)(lib1770_stats.o)
LIB1770_O+=$(LIB1770_A)(lib1770_block.o)
LIB1770_O+=$(LIB1770_A)(lib1770_pre.o)
LIB1770_O+=$(LIB1770_A)(lib1770_biquad.o)
$(LIB1770_O): $(LIB1770_H)
$(LIB1770_A): $(LIB1770_O)
all: $(LIB1770_A)
install: $(INCLUDEDIR)/lib1770.h
install: $(LIBDIR)/lib1770-2.a
$(INCLUDEDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)
$(LIBDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)

clean:
	rm -f *.a *.o
