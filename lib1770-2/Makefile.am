CFLAGS+=-Werror
CFLAGS+=-Wall
CFLAGS+=-Wformat
CFLAGS+=-Wpointer-arith
CFLAGS+=-Wcast-align
CFLAGS+=-Wcast-qual
CFLAGS+=-Wreturn-type
CFLAGS+=-Wextra
CFLAGS+=-Wstrict-prototypes
#CFLAGS+=-Wmissing-declarations
#CFLAGS+=-Wmissing-prototypes

CPPFLAGS+=-I$(top_srcdir)/libpbutil

#CFLAGS+=-g
#CFLAGS+=-ggdb
#LDFLAGS+=-static-libgcc
#LDFLAGS+=-static-libstdc++

$(libffsox_2_a_OBJECTS): $(CONFIG_HEADER)
$(libffsox_2_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil.h
$(libffsox_2_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil_priv.h

noinst_LIBRARIES=lib1770_2.a
lib1770_2_a_SOURCES=
lib1770_2_a_SOURCES+=lib1770.h
lib1770_2_a_SOURCES+=lib1770_biquad.c
lib1770_2_a_SOURCES+=lib1770_block.c
lib1770_2_a_SOURCES+=lib1770_pre.c
lib1770_2_a_SOURCES+=lib1770_stats.c
