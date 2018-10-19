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
#AM_CFLAGS+=-Wmissing-declarations
#AM_CFLAGS+=-Wmissing-prototypes

#AM_CFLAGS+=-g
#AM_CFLAGS+=-ggdb
#AM_LDFLAGS+=-static-libgcc
#AM_LDFLAGS+=-static-libstdc++

$(libpbutil_a_OBJECTS): $(CONFIG_HEADER)

noinst_LIBRARIES=libpbutil.a
libpbutil_a_SOURCES=
libpbutil_a_SOURCES+=pbutil.h
libpbutil_a_SOURCES+=pbutil_priv.h
#libpbutil_a_SOURCES+=dirent.h
libpbutil_a_SOURCES+=pbu_basename.c
libpbutil_a_SOURCES+=pbu_copy_file.c
#libpbutil_a_SOURCES+=pbu_dirent.c
libpbutil_a_SOURCES+=pbu_ext.c
libpbutil_a_SOURCES+=pbu_extend_path.c
libpbutil_a_SOURCES+=pbu_list.c
libpbutil_a_SOURCES+=pbu_malloc.c
libpbutil_a_SOURCES+=pbu_mkdir.c
libpbutil_a_SOURCES+=pbu_msvcrt.c
libpbutil_a_SOURCES+=pbu_s2w.c
libpbutil_a_SOURCES+=pbu_w2s.c
libpbutil_a_SOURCES+=pbu_same_file.c
libpbutil_a_SOURCES+=pbu_strtok.c
libpbutil_a_SOURCES+=pbu_trace.c
libpbutil_a_SOURCES+=pbu_wcstok.c
libpbutil_a_SOURCES+=pbu_vwritelna.c
libpbutil_a_SOURCES+=pbu_vwritelnw.c
