include config.mak
VPATH=$(srcdir)
CPPFLAGS+=-I$(srcdir)
CPPFLAGS+=-I$(includedir)
CPPFLAGS+=-I$(sox-prefix)/include
CPPFLAGS+=-I$(ffmpeg-prefix)/include
CPPFLAGS+=-DHAVE_FFSOX_DYNLOAD
ifeq (,$(msys)) # [
LDLIBS+=-lm
LDLIBS+=-ldl
endif # ]
header:=$(shell find $(srcdir) -name '*.h' -exec basename {} \;)
bs1770gain-header:=$(patsubst %.h,$(srcdir)/%.h,$(header))
libffsox-header:=$(shell find $(includedir) -name 'ffsox*.h')
lib1770-header:=$(shell find $(includedir) -name 'lib1770*.h')
libpbu-header:=$(shell find $(includedir) -name 'pbutil*.h')
source:=$(shell find $(srcdir) -name 'bs1770gain_*.c' -exec basename {} \;)
object:=$(patsubst %.c,%$(exto),$(source))
member:=$(patsubst %$(exto),lib1770gain$(exta)(%$(exto)),$(object))

###############################################################################
.PHONY: all install
all: bs1770gain
all: bs1770gain-tools/$(call so,lib,avutil,56)
all: bs1770gain-tools/$(call so,lib,swresample,3)
all: bs1770gain-tools/$(call so,lib,avcodec,58)
all: bs1770gain-tools/$(call so,lib,avformat,58)
all: bs1770gain-tools/$(call so,,libsox,3)
ifeq (,$(msys)) # [
bs1770gain-tools/$(call so,lib,avutil,56): $(ffmpeg-prefix)/lib/$(call so,lib,avutil,56)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
bs1770gain-tools/$(call so,lib,swresample,3): $(ffmpeg-prefix)/lib/$(call so,lib,swresample,3)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
bs1770gain-tools/$(call so,lib,avcodec,58): $(ffmpeg-prefix)/lib/$(call so,lib,avcodec,58)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
bs1770gain-tools/$(call so,lib,avformat,58): $(ffmpeg-prefix)/lib/$(call so,lib,avformat,58)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
bs1770gain-tools/$(call so,,libsox,3): $(sox-prefix)/lib/$(call so,,libsox,3)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
else # ] [
bs1770gain-tools/%.dll: $(ffmpeg-prefix)/bin/%.dll
	mkdir -p $(@D)
	cp $< $(@D)
bs1770gain-tools/%.dll: $(sox-prefix)/bin/%.dll
	mkdir -p $(@D)
	cp $< $(@D)
endif # ]
bs1770gain: lib1770gain$(exta)
bs1770gain: $(libdir)/libffsox_2$(exta)
bs1770gain: $(libdir)/lib1770_2$(exta)
bs1770gain: $(libdir)/libpbutil$(exta)
install: $(libdir)/lib1770gain$(exta)
install: $(patsubst %.h,$(includedir)/%.h,$(header))
install: $(bindir)/bs1770gain
install: bs1770gain-tools/$(call so,lib,avutil,56)
install: bs1770gain-tools/$(call so,lib,swresample,3)
install: bs1770gain-tools/$(call so,lib,avcodec,58)
install: bs1770gain-tools/$(call so,lib,avformat,58)
install: bs1770gain-tools/$(call so,,libsox,3)
install: $(bindir)/bs1770gain-tools/$(call so,lib,avutil,56)
install: $(bindir)/bs1770gain-tools/$(call so,lib,swresample,3)
install: $(bindir)/bs1770gain-tools/$(call so,lib,avcodec,58)
install: $(bindir)/bs1770gain-tools/$(call so,lib,avformat,58)
install: $(bindir)/bs1770gain-tools/$(call so,,libsox,3)
ifeq (,$(msys)) # [
$(bindir)/bs1770gain-tools/$(call so,lib,avutil,56): $(ffmpeg-prefix)/lib/$(call so,lib,avutil,56)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
$(bindir)/bs1770gain-tools/$(call so,lib,swresample,3): $(ffmpeg-prefix)/lib/$(call so,lib,swresample,3)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
$(bindir)/bs1770gain-tools/$(call so,lib,avcodec,58): $(ffmpeg-prefix)/lib/$(call so,lib,avcodec,58)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
$(bindir)/bs1770gain-tools/$(call so,lib,avformat,58): $(ffmpeg-prefix)/lib/$(call so,lib,avformat,58)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
$(bindir)/bs1770gain-tools/$(call so,,libsox,3): $(sox-prefix)/lib/$(call so,,libsox,3)
	mkdir -p $(@D)
	rm -f $@
	ln -s $< $@
else # ] [
$(bindir)/bs1770gain-tools/%$(extso): bs1770gain-tools/%$(extso)
	mkdir -p $(@D)
	cp $< $(@D)
endif # ]

###############################################################################
$(bindir)/bs1770gain: bs1770gain
	mkdir -p $(@D)
	cp $< $(@D)
$(libdir)/lib1770gain$(exta): lib1770gain$(exta)
	mkdir -p $(@D)
	cp $< $(@D)
$(includedir)/%.h: $(srcdir)/%.h
	mkdir -p $(@D)
	cp $< $(@D)
.PRECIOUS: lib1770gain$(exta)
lib1770gain$(exta): $(member)
bs1770gain$(exto) $(object): $(bs1770gain-header)
bs1770gain$(exto) $(object): $(libffsox-header)
bs1770gain$(exto) $(object): $(lib1770-header)
bs1770gain$(exto) $(object): $(libpbu-header)
.INTERMEDIATE: $(object)
