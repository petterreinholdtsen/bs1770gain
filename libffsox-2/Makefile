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
ifeq (mingw,$(OS)) # {
LDLIBS+=-lole32
LDLIBS+=-lksguid
else
LDLIBS+=-lm
LDLIBS+=-ldl
endif # }

PROGRAM+=mux1
PROGRAM+=mux2
PROGRAM+=mux3
ifeq (mingw,$(OS))
PROGRAM+=wasapi
PROGRAM+=player
endif

LIBFFSOX_H+=$(INCLUDEDIR)/pbutil.h
LIBFFSOX_H+=$(INCLUDEDIR)/pbutil_priv.h
LIBFFSOX_H+=ffsox.h
LIBFFSOX_H+=ffsox_priv.h
LIBFFSOX_H+=ffsox_dynload.h

LIBFFSOX_A=libffsox-2.a
##
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_audio_player.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_analyze.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_collect.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_aggregate.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_convert.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_sox_reader.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_frame_writer.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_frame_consumer.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_frame_reader.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_packet_writer.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_packet_consumer.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_packet_consumer_list.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_stream.o)
##
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_source_link.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_source_progress.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_source.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_node.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_sink.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_machine.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_frame_convert_sox.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_frame_convert.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_frame.o)
# utilities
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_sox_add_effect.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_sox_pull_handler.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_sox_read_handler.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_audiostream.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_find_decoder.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_csv2avdict.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_dynload.o)
LIBFFSOX_O+=$(LIBFFSOX_A)(ffsox_path3.o)
$(LIBFFSOX_O): $(LIBFFSOX_H)
$(LIBFFSOX_A): $(LIBFFSOX_O)
all: $(PROGRAM)
all: $(LIBFFSOX_A)
ifeq (mingw,$(OS)) # {
install: $(patsubst %,$(BINDIR)/%.exe,$(PROGRAM))
else # } {
install: $(patsubst %,$(BINDIR)/%,$(PROGRAM))
endif # }
$(PROGRAM): $(LIBFFSOX_A)
mux3: $(LIBDIR)/lib1770-2.a
$(PROGRAM): $(LIBDIR)/libpbutil.a
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
