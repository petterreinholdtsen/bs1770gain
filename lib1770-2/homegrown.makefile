include config.mak
VPATH=$(srcdir)
CPPFLAGS+=-I$(srcdir)
CPPFLAGS+=-I$(includedir)
header:=$(shell find $(srcdir) -name '*.h' -exec basename {} \;)
lib1770-header:=$(patsubst %.h,$(srcdir)/%.h,$(header))
pbu-header:=$(shell find $(includedir) -name 'pbutil*.h')
source:=$(shell find $(srcdir) -name '*.c' -exec basename {} \;)
object:=$(patsubst %.c,%$(exto),$(source))
member:=$(patsubst %$(exto),lib1770_2$(exta)(%$(exto)),$(object))

###############################################################################
.PHONY: all install
all: lib1770_2$(exta)
install: lib1770_2$(exta)
install: $(libdir)/lib1770_2$(exta)
install: $(patsubst %.h,$(includedir)/%.h,$(header))

###############################################################################
$(libdir)/lib1770_2$(exta): lib1770_2$(exta)
	mkdir -p $(@D)
	cp $< $(@D)
$(includedir)/%.h: $(srcdir)/%.h
	mkdir -p $(@D)
	cp $< $(@D)
.PRECIOUS: lib1770_2$(exta)
lib1770_2$(exta): $(member)
$(object): $(lib1770-header)
$(object): $(pbu-header)
.INTERMEDIATE: $(object)
