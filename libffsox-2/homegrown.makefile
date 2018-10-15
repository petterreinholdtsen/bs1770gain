include config.mak
VPATH=$(srcdir)
CPPFLAGS+=-I$(srcdir)
CPPFLAGS+=-I$(includedir)
CPPFLAGS+=-I$(sox-prefix)/include
CPPFLAGS+=-I$(ffmpeg-prefix)/include
CPPFLAGS+=-DHAVE_FFSOX_DYNLOAD
header:=$(shell find $(srcdir) -name '*.h' -exec basename {} \;)
libffsox-header:=$(patsubst %.h,$(srcdir)/%.h,$(header))
lib1770-header:=$(shell find $(includedir) -name 'lib1770*.h')
libpbu-header:=$(shell find $(includedir) -name 'pbutil*.h')
source:=$(shell find $(srcdir) -name '*.c' -exec basename {} \;)
object:=$(patsubst %.c,%$(exto),$(source))
member:=$(patsubst %$(exto),libffsox_2$(exta)(%$(exto)),$(object))

###############################################################################
.PHONY: all install
all: libffsox_2$(exta)
install: libffsox_2$(exta)
install: $(libdir)/libffsox_2$(exta)
install: $(patsubst %.h,$(includedir)/%.h,$(header))

###############################################################################
$(libdir)/libffsox_2$(exta): libffsox_2$(exta)
	mkdir -p $(@D)
	cp $< $(@D)
$(includedir)/%.h: $(srcdir)/%.h
	mkdir -p $(@D)
	cp $< $(@D)
.PRECIOUS: libffsox_2$(exta)
libffsox_2$(exta): $(member)
$(object): $(libffsox-header)
$(object): $(lib1770-header)
$(object): $(libpbu-header)
.INTERMEDIATE: $(object)
