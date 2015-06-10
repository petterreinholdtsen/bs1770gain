include config.mak
include $(SRCDIR)/version.mak
vpath % $(PWD) $(SRCDIR)
.PHONY: all install clean

ifeq (msvc,$(OS)) # {
# https://msdn.microsoft.com/en-us/library/abx4dbyh.aspx
DOS2UX=$(shell echo '$(1)'|sed -e 's,\(.\):/,/\1/,g')
export PATH+=:$(call DOS2UX,$(MSVS))/Common7/IDE
MSVC=$(MSVS)/VC
CL='$(MSVC)/bin/cl'

CFLAGS+=-nologo
CFLAGS+=-O2
CFLAGS+=-MD
CFLAGS+=-WX

CPPFLAGS+=-D_MT
CPPFLAGS+=-D_DLL
CPPFLAGS+='-I$(MSVC)/include'
CPPFLAGS+='-I$(MSSDK)/include'
else # } {
CC=gcc
CFLAGS+=-O2
CFLAGS+=-Wall
CFLAGS+=-Wextra
CFLAGS+=-Werror
LDFLAGS+=-L$(PWD)
LDFLAGS+=-L$(LIBDIR)
ifeq (linux,$(OS)) # // {
LDLIBS+=-ldl
LDLIBS+=-lm
endif # // }
endif # }

CPPFLAGS+=-I$(PWD)
CPPFLAGS+=-I$(SRCDIR)
CPPFLAGS+=-I$(INCLUDEDIR)

ALL+=bs1770gain
SOXDLLV=2
all: $(ALL)
ifeq ($(filter $(OS),mingw msvc),$(OS)) # {
all: bs1770gain-tools/avutil-54.dll
all: bs1770gain-tools/swresample-1.dll
all: bs1770gain-tools/avcodec-56.dll
all: bs1770gain-tools/avformat-56.dll
all: bs1770gain-tools/libsox-$(SOXDLLV).dll
install: $(BINDIR)/bs1770gain-tools/avutil-54.dll
install: $(BINDIR)/bs1770gain-tools/swresample-1.dll
install: $(BINDIR)/bs1770gain-tools/avcodec-56.dll
install: $(BINDIR)/bs1770gain-tools/avformat-56.dll
install: $(BINDIR)/bs1770gain-tools/libsox-$(SOXDLLV).dll
else # } {
all: bs1770gain-tools/libavutil.so.54
all: bs1770gain-tools/libswresample.so.1
all: bs1770gain-tools/libavcodec.so.56
all: bs1770gain-tools/libavformat.so.56
all: bs1770gain-tools/libsox.so.$(SOXDLLV)
install: $(BINDIR)/bs1770gain-tools/libavutil.so.54
install: $(BINDIR)/bs1770gain-tools/libswresample.so.1
install: $(BINDIR)/bs1770gain-tools/libavcodec.so.56
install: $(BINDIR)/bs1770gain-tools/libavformat.so.56
install: $(BINDIR)/bs1770gain-tools/libsox.so.$(SOXDLLV)
endif # }

LIBBS1770GAIN_H+=$(INCLUDEDIR)/pbutil.h
LIBBS1770GAIN_H+=$(INCLUDEDIR)/lib1770.h
LIBBS1770GAIN_H+=$(INCLUDEDIR)/ffsox.h
LIBBS1770GAIN_H+=$(INCLUDEDIR)/ffsox_priv.h
LIBBS1770GAIN_H+=$(INCLUDEDIR)/ffsox_dynload.h
LIBBS1770GAIN_H+=bs1770gain.h
LIBBS1770GAIN_H+=bs1770gain_priv.h

LIBBS1770GAIN_A=libbs1770gain.a
LIBBS1770GAIN_O+=$(LIBBS1770GAIN_A)(bs1770gain_aggregate.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN_A)(bs1770gain_album.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN_A)(bs1770gain_transcode.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN_A)(bs1770gain_opath.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN_A)(bs1770gain_tree.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN_A)(bs1770gain_parse_time.o)
$(LIBBS1770GAIN_O): $(LIBBS1770GAIN_H)
$(LIBBS1770GAIN_A): $(LIBBS1770GAIN_O)
bs1770gain: $(LIBBS1770GAIN_A)
$(LIBBS1770GAIN_O) bs1770gain: $(LIBDIR)/libffsox-2.a
$(LIBBS1770GAIN_O) bs1770gain: $(LIBDIR)/lib1770-2.a
$(LIBBS1770GAIN_O) bs1770gain: $(LIBDIR)/libpbutil.a
install: all
ifeq ($(filter $(OS),mingw msvc),$(OS)) # {
install: $(patsubst %,$(BINDIR)/%.exe,$(ALL))
else
install: $(patsubst %,$(BINDIR)/%,$(ALL))
endif

ifeq ($(filter $(OS),mingw msvc),$(OS)) # {
bs1770gain-tools/%: $(BINDIR)/%
	mkdir -p $(@D)
	cp $< $(@D)
$(BINDIR)/bs1770gain-tools/%: bs1770gain-tools/%
	mkdir -p $(@D)
	cp $< $(@D)
$(BINDIR)/%.exe: %
	mkdir -p $(@D)
	cp $< $(@D)
else # // } {
bs1770gain-tools/%: $(LIBDIR)/%
	mkdir -p $(@D)
	cp $< $(@D)
$(LIBDIR)/bs1770gain-tools/%: bs1770gain-tools/%
	mkdir -p $(@D)
	cp $< $(@D)
$(BINDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)
endif # }

clean:
	rm -f bs1770gain bs1770gain.exe *.a *.o

###############################################################################
ifeq (msvc,$(OS)) # {
%.dll: %.obj
	$(CL) -LD $(CFLAGS) $(CPPFLAGS) -Fe$@ $^ -link $(LDFLAGS) $(LDLIBS)
%.exe: %.obj
	$(CL) $(CFLAGS) $(CPPFLAGS) -Fe$@ $^ -link $(LDFLAGS) $(LDLIBS)
%.obj: %.c
	$(CL) $(CFLAGS) $(CPPFLAGS) -c -Fo$@ $<
%.obj: %.cpp
	$(CL) $(CFLAGS) $(CPPFLAGS) -c -Fo$@ $<
endif # }
