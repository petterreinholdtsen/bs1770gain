include config.mak
include $(SRCDIR)/version.mak
vpath % $(PWD) $(SRCDIR)
.PHONY: all install clean

CC=gcc
CPPFLAGS+=-I$(PWD)
CPPFLAGS+=-I$(SRCDIR)
CPPFLAGS+=-I$(INCLUDEDIR)
CFLAGS+=-O2
CFLAGS+=-Wall
#CFLAGS+=-Werror
LDFLAGS+=-L$(PWD)
LDFLAGS+=-L$(LIBDIR)
LDLIBS+=-lbs1770gain
LDLIBS+=-l1770-2
ifeq (linux,$(OS)) # // {
LDLIBS+=-ldl
LDLIBS+=-lm
endif # // }

all: bs1770gain
ifeq (mingw,$(OS)) # // {
all: bs1770gain-tools/avutil-54.dll
all: bs1770gain-tools/swresample-1.dll
all: bs1770gain-tools/avcodec-56.dll
all: bs1770gain-tools/avformat-56.dll
all: bs1770gain-tools/libsox-2.dll
install: $(BINDIR)/bs1770gain-tools/avutil-54.dll
install: $(BINDIR)/bs1770gain-tools/swresample-1.dll
install: $(BINDIR)/bs1770gain-tools/avcodec-56.dll
install: $(BINDIR)/bs1770gain-tools/avformat-56.dll
install: $(BINDIR)/bs1770gain-tools/libsox-2.dll
else # } {
all: bs1770gain-tools/libavutil.so.54
all: bs1770gain-tools/libswresample.so.1
all: bs1770gain-tools/libavcodec.so.56
all: bs1770gain-tools/libavformat.so.56
all: bs1770gain-tools/libsox.so.2
install: $(BINDIR)/bs1770gain-tools/libavutil.so.54
install: $(BINDIR)/bs1770gain-tools/libswresample.so.1
install: $(BINDIR)/bs1770gain-tools/libavcodec.so.56
install: $(BINDIR)/bs1770gain-tools/libavformat.so.56
install: $(BINDIR)/bs1770gain-tools/libsox.so.2
endif # }

BS1770GAIN_H+=$(INCLUDEDIR)/lib1770.h
BS1770GAIN_H+=bs1770gain.h
BS1770GAIN_H+=bs1770gain_dynload.h

LIBBS1770GAIN=libbs1770gain.a
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_csv2avdict.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_convert.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_seek.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_sox.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_read.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_sink.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_album.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_transcode.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_stats.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_audiostream.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_basename.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_dynload.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_tree.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_oor.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_find_decoder.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_parse_time.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_same_file.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_copy_file.o)
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_path3.o)
ifeq (mingw,$(OS))
LIBBS1770GAIN_O+=$(LIBBS1770GAIN)(bs1770gain_s2w.o)
endif
$(LIBBS1770GAIN_O): $(BS1770GAIN_H)
$(LIBBS1770GAIN): $(LIBBS1770GAIN_O)

bs1770gain: $(LIBBS1770GAIN)
bs1770gain: $(LIBDIR)/lib1770-2.a
bs1770gain: $(BS1770GAIN_H)
ifeq (mingw,$(OS))
install: $(BINDIR)/bs1770gain.exe
else
install: $(BINDIR)/bs1770gain
endif

ifeq (mingw,$(OS)) # // {
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
