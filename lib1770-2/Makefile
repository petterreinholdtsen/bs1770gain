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

LIB1770=lib1770-2.a
LIB1770_O+=$(LIB1770)(lib1770_stats.o)
LIB1770_O+=$(LIB1770)(lib1770_block.o)
LIB1770_O+=$(LIB1770)(lib1770_pre.o)
LIB1770_O+=$(LIB1770)(lib1770_biquad.o)
$(LIB1770): $(LIB1770_O)
$(LIB1770_O): lib1770.h
all: $(LIB1770)
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
