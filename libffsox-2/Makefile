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
#LDFLAGS+=-L$(LIBDIR)
#LDLIBS+=-lavformat
#LDLIBS+=-lavcodec
#LDLIBS+=-lavutil
ifeq (linux,$(OS)) # {
LDLIBS+=-lm
LDLIBS+=-ldl
endif # }

LIBFFSOX_H+=ffsox.h
LIBFFSOX_H+=ffsox_priv.h
LIBFFSOX_H+=ffsox_dynload.h
LIBFFSOX_A=libffsox-2.a
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_filter.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_write_encode.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_write_copy.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_write.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_read_decode.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_read_copy.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_read.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_source_link.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_source.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_node.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_sink.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_read_list.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_machine.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_frame.o)
# utilities
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_audiostream.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_list.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_csv2avdict.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_basename.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_dynload.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_path3.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_wcstok.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_strtok.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_msvcrt.o)
$(LIBFFSOX_O): $(LIBFFSOX_H)
$(LIBFFSOX_A): $(LIBFFSOX_O)
all: mux
all: $(LIBFFSOX)
ifeq (mingw,$(OS)) # {
install: $(BINDIR)/mux.exe
else # } {
install: $(BINDIR)/mux
endif # }
mux: $(LIBFFSOX_A)
install: $(patsubst %.h,$(INCLUDEDIR)/%.h,$(LIBFFSOX_H))
install: $(LIBDIR)/libffsox-2.a
ifeq (mingw,$(OS)) # {
$(BINDIR)/%.exe: %
	mkdir -p $(@D)
	cp $< $(@D)
else # } {
$(BINDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)
endif # }
$(INCLUDEDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)
$(LIBDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)

clean:
	rm -f *.a *.o
