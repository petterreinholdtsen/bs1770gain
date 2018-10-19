AM_CFLAGS=
AM_CPPFLAGS=
AM_LDFLAGS=

AM_CFLAGS+=-Werror
AM_CFLAGS+=-Wall
AM_CFLAGS+=-Wformat
AM_CFLAGS+=-Wpointer-arith
AM_CFLAGS+=-Wcast-align
AM_CFLAGS+=-Wcast-qual
AM_CFLAGS+=-Wreturn-type
AM_CFLAGS+=-Wextra
AM_CFLAGS+=-Wstrict-prototypes
AM_CFLAGS+=-Wno-unused-parameter
AM_CFLAGS+=-Wno-deprecated-declarations
#AM_CFLAGS+=-Wmissing-declarations
#AM_CFLAGS+=-Wmissing-prototypes
#AM_CFLAGS+=-Wno-unused-variable

AM_CPPFLAGS+=-I$(top_srcdir)/libpbutil
AM_CPPFLAGS+=-I$(top_srcdir)/lib1770-2
AM_CPPFLAGS+=-I$(top_srcdir)/libffsox-2

#AM_CFLAGS+=-g
#AM_CFLAGS+=-ggdb
#AM_LDFLAGS+=-static-libgcc
#AM_LDFLAGS+=-static-libstdc++
if WIN32
AM_LDFLAGS+=-municode
endif
AM_LDFLAGS+=@BS1770GAIN_LDFLAGS@

bin_PROGRAMS=
bin_PROGRAMS+=bs1770gain
bs1770gain_SOURCES=
bs1770gain_SOURCES+=bs1770gain.c
bs1770gain_LDADD=
bs1770gain_LDADD+=libbs1770gain.a
bs1770gain_LDADD+=$(top_builddir)/libffsox-2/libffsox_2.a
bs1770gain_LDADD+=$(top_builddir)/libffsox-2/libffsox_2.a
bs1770gain_LDADD+=$(top_builddir)/lib1770-2/lib1770_2.a
bs1770gain_LDADD+=$(top_builddir)/libpbutil/libpbutil.a
if WIN32
AM_LDFLAGS+=-lsox.dll
AM_LDFLAGS+=-lavformat.dll
AM_LDFLAGS+=-lavcodec.dll
AM_LDFLAGS+=-lswresample.dll
AM_LDFLAGS+=-lavutil.dll
else
AM_LDFLAGS+=-lsox
AM_LDFLAGS+=-lavformat
AM_LDFLAGS+=-lavcodec
AM_LDFLAGS+=-lswresample
AM_LDFLAGS+=-lavutil
if FFSOX_DYNLOAD
bs1770gain_LDADD+=-ldl
endif
endif
bs1770gain_LDADD+=-lm

noinst_LIBRARIES=libbs1770gain.a
libbs1770gain_a_SOURCES=
libbs1770gain_a_SOURCES+=bs1770gain.h
libbs1770gain_a_SOURCES+=bs1770gain_priv.h
libbs1770gain_a_SOURCES+=bs1770gain_print_xml.c
libbs1770gain_a_SOURCES+=bs1770gain_print_classic.c
libbs1770gain_a_SOURCES+=bs1770gain_aggregate.c
libbs1770gain_a_SOURCES+=bs1770gain_album.c
libbs1770gain_a_SOURCES+=bs1770gain_opath.c
libbs1770gain_a_SOURCES+=bs1770gain_parse_time.c
libbs1770gain_a_SOURCES+=bs1770gain_transcode.c
libbs1770gain_a_SOURCES+=bs1770gain_tree.c

OBJECTS=
OBJECTS+=$(bst1770gain_OBJECTS)
OBJECTS+=$(libbs1770gain_a_OBJECTS)
$(OBJECTS): $(CONFIG_HEADER)
$(OBJECTS): $(top_srcdir)/libpbutil/pbutil.h
$(OBJECTS): $(top_srcdir)/libpbutil/pbutil_priv.h
$(OBJECTS): $(top_srcdir)/lib1770-2/lib1770.h
$(OBJECTS): $(top_srcdir)/libffsox-2/ffsox.h
$(OBJECTS): $(top_srcdir)/libffsox-2/ffsox_priv.h
$(OBJECTS): $(top_srcdir)/libffsox-2/ffsox_dynload.h

if FFSOX_DYNLOAD
install-exec-hook: $(DESTDIR)$(bindir)/bs1770gain-tools/@LIBSOX_SO@
install-exec-hook: $(DESTDIR)$(bindir)/bs1770gain-tools/@AVFORMAT_SO@
install-exec-hook: $(DESTDIR)$(bindir)/bs1770gain-tools/@AVCODEC_SO@
install-exec-hook: $(DESTDIR)$(bindir)/bs1770gain-tools/@SWRESAMPLE_SO@
install-exec-hook: $(DESTDIR)$(bindir)/bs1770gain-tools/@AVUTIL_SO@
$(DESTDIR)$(bindir)/bs1770gain-tools/$(LIBSOX_SO): @SOX_PREFIX@/@SODIR@/@LIBSOX_SO@
	mkdir -p $(@D)
	cp $< $(@D)
$(DESTDIR)$(bindir)/bs1770gain-tools/$(AVFORMAT_SO): @FFMPEG_PREFIX@/@SODIR@/@AVFORMAT_SO@
	mkdir -p $(@D)
	cp $< $(@D)
$(DESTDIR)$(bindir)/bs1770gain-tools/$(AVCODEC_SO): @FFMPEG_PREFIX@/@SODIR@/@AVCODEC_SO@
	mkdir -p $(@D)
	cp $< $(@D)
$(DESTDIR)$(bindir)/bs1770gain-tools/$(SWRESAMPLE_SO): @FFMPEG_PREFIX@/@SODIR@/@SWRESAMPLE_SO@
	mkdir -p $(@D)
	cp $< $(@D)
$(DESTDIR)$(bindir)/bs1770gain-tools/$(AVUTIL_SO): @FFMPEG_PREFIX@/@SODIR@/@AVUTIL_SO@
	mkdir -p $(@D)
	cp $< $(@D)
endif
