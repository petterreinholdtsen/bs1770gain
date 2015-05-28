include config.mak
include $(SRCDIR)/version.mak
vpath % $(SRCDIR)
.PHONY: all install clean

CC=gcc
CPPFLAGS+=-I$(SRCDIR)
CPPFLAGS+=-I$(INCLUDEDIR)
CFLAGS+=-O2
CFLAGS+=-Wall
ifeq (mingw,$(OS))
LDFLAGS+=-static
endif
LDFLAGS+=-L$(LIBDIR)
####
COMMON_LDLIBS+=-l1770-2
####
FFMPEG_LDLIBS+=-lavformat
FFMPEG_LDLIBS+=-lavcodec
FFMPEG_LDLIBS+=-lavutil
FFMPEG_LDLIBS+=-lswresample
FFMPEG_LDLIBS+=-lz
ifeq (mingw,$(OS))
FFMPEG_LDLIBS+=-liconv
FFMPEG_LDLIBS+=-lws2_32
FFMPEG_LDLIBS+=-lkernel32
endif
####
SNDFILE_LDLIBS+=-lsndfile
SNDFILE_LDLIBS+=-lFLAC
SNDFILE_LDLIBS+=-lvorbisenc
SNDFILE_LDLIBS+=-lvorbis
SNDFILE_LDLIBS+=-logg

TARGET+=sndfile1770
TARGET+=ffmpeg1770

all: $(TARGET)
ifeq (mingw,$(OS))
install: $(patsubst %,$(BINDIR)/%.exe,$(TARGET))
else
install: $(patsubst %,$(BINDIR)/%,$(TARGET))
endif
$(TARGET): $(INCLUDEDIR)/lib1770.h
$(TARGET): $(LIBDIR)/lib1770-2.a
sndfile1770: $(LIBDIR)/libsndfile.a
sndfile1770: LDLIBS=$(COMMON_LDLIBS) $(SNDFILE_LDLIBS) -lm
ffmpeg1770: $(LIBDIR)/libavutil.a
ffmpeg1770: $(LIBDIR)/libavcodec.a
ffmpeg1770: $(LIBDIR)/libavformat.a
ffmpeg1770: $(LIBDIR)/libswresample.a
ffmpeg1770: LDLIBS=$(COMMON_LDLIBS) $(FFMPEG_LDLIBS) -lm

ifeq (mingw,$(OS))
$(BINDIR)/%.exe: %
	mkdir -p $(@D)
	cp $< $(@D)
else
$(BINDIR)/%: %
	mkdir -p $(@D)
	cp $< $(@D)
endif

clean:
	rm -f *.a *.o
