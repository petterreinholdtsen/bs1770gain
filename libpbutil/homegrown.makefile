include config.mak
VPATH=$(srcdir)
CPPFLAGS+=-I$(srcdir)
CPPFLAGS+=-I$(includedir)
header:=$(shell find $(srcdir) -name '*.h' -exec basename {} \;)
pbutil-header:=$(patsubst %.h,$(srcdir)/%.h,$(header))
source:=$(shell find $(srcdir) -name '*.c' -exec basename {} \;)
object:=$(patsubst %.c,%$(exto),$(source))
member:=$(patsubst %$(exto),libpbutil$(exta)(%$(exto)),$(object))

###############################################################################
.PHONY: all install
all: libpbutil$(exta)
install: libpbutil$(exta)
install: $(libdir)/libpbutil$(exta)
install: $(patsubst %.h,$(includedir)/%.h,$(header))

###############################################################################
$(libdir)/libpbutil$(exta): libpbutil$(exta)
	mkdir -p $(@D)
	cp $< $(@D)
$(includedir)/%.h: $(srcdir)/%.h
	mkdir -p $(@D)
	cp $< $(@D)
.PRECIOUS: libpbutil$(exta)
libpbutil$(exta): $(member)
$(object): $(pbutil-header)
.INTERMEDIATE: $(object)
