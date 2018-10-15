include config.mak
VPATH=$(srcdir)
CPPFLAGS+=-I$(srcdir)
header:=$(shell find $(srcdir) -name '*.h' -exec basename {} \;)
source:=$(shell find $(srcdir) -name '*.c' -exec basename {} \;)
object:=$(patsubst %.c,%$(exto),$(source))

.PHONY: all install
###############################################################################
all: libpbutil$(exta)
libpbutil$(exta): $(patsubst %$(exto),libpbutil$(exta)(%$(exto)),$(object))
$(object): $(patsubst %.h,$(srcdir)/%.h,$(header))
.INTERMEDIATE: $(object)

###############################################################################
install: $(libdir)/libpbutil$(exta)
install: $(patsubst %.h,$(includedir)/%.h,$(header))
$(libdir)/%$(exta): %$(exta)
	mkdir -p $(@D)
	cp $< $(@D)
$(includedir)/%.h: $(srcdir)/%.h
	mkdir -p $(@D)
	cp $< $(@D)
