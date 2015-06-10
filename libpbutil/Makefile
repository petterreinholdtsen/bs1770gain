include config.mak
include $(SRCDIR)/version.mak
vpath % $(SRCDIR)
.PHONY: all install clean

CC=gcc
CPPFLAGS+=-I$(SRCDIR)
CPPFLAGS+=-I$(INCLUDEDIR)
CFLAGS+=-O2
CFLAGS+=-Wall
CFLAGS+=-Wextra
CFLAGS+=-Werror

LIBPBUTIL_H+=pbutil.h
LIBPBUTIL_H+=pbutil_priv.h

LIBPBUTIL_A=libpbutil.a

LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_list.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_extend_path.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_mkdir.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_s2w.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_copy_file.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_same_file.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_ext.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_basename.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_wcstok.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_strtok.o)
LIBPBUTIL_O+=$(LIBPBUTIL_A)(pbu_msvcrt.o)
$(LIBPBUTIL_O): $(LIBPBUTIL_H)
$(LIBPBUTIL_A): $(LIBPBUTIL_O)
all: $(LIBPBUTIL_A)
install: $(patsubst %.h,$(INCLUDEDIR)/%.h,$(LIBPBUTIL_H))
install: $(LIBDIR)/libpbutil.a
$(INCLUDEDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)
$(LIBDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)

clean:
	rm -f *.a *.o
