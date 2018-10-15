include config.mak
VPATH=$(srcdir)

###############################################################################
.PHONY: all
all: $(libdir)/lib1770gain$(exta)

###############################################################################
ffmpeg-includedir=$(ffmpeg-prefix)/include
ffmpeg-libdir=$(ffmpeg-prefix)/lib
ifeq (,$(msys)) # [
ffmpeg-sodir=$(ffmpeg-prefix)/lib
else # ] [
ffmpeg-sodir=$(ffmpeg-prefix)/bin
endif # ]
ffmpeg-builddir=$(ffmpeg-prefix)/build/ffmpeg

###############################################################################
sox=sox-14.4.2
sox-includedir=$(sox-prefix)/include
sox-libdir=$(sox-prefix)/lib
ifeq (,$(msys)) # [
sox-sodir=$(sox-prefix)/lib
else # ] [
sox-sodir=$(sox-prefix)/bin
endif # ]
sox-builddir=$(sox-prefix)/build/$(sox)

###############################################################################
ifneq (,$(ffmpeg-prefix)) # [
  conf-opts+='--ffmpeg-prefix=$(ffmpeg-prefix)'
endif # ]
ifneq (,$(sox-prefix)) # [
  conf-opts+='--sox-prefix=$(sox-prefix)'
endif # ]
ifneq (,$(prefix)) # [
  conf-opts+='--prefix=$(prefix)'
endif # ]
ifneq (,$(includedir)) # [
  conf-opts+='--includedir=$(includedir)'
endif # ]
ifneq (,$(bindir)) # [
  conf-opts+='--bindir=$(bindir)'
endif # ]
ifneq (,$(libdir)) # [
  conf-opts+='--libdir=$(libdir)'
endif # ]
ifneq (,$(msvc)) # [
  conf-opts+='--msvc=$(msvc)'
endif # ]
ifneq (,$(mssdk)) # [
  conf-opts+='--mssdk=$(mssdk)'
endif # ]
ifneq (,$(tools)) # [
  conf-opts+='--tools=$(tools)'
endif # ]
ifneq (,$(reldir)) # [
  conf-opts+='--reldir=$(reldir)'
endif # ]
ifneq (,$(nsis)) # [
  conf-opts+='--nsis=$(nsis)'
endif # ]
ifneq (,$(gtk)) # [
  conf-opts+='--gtk=$(gtk)'
endif # ]

###############################################################################
bs1770gain-source:=$(shell find $(srcdir)/bs1770gain -name '*.h' -or -name '*.c')
$(libdir)/lib1770gain$(exta): $(bs1770gain-source)
$(libdir)/lib1770gain$(exta): $(PWD)/bs1770gain/Makefile
	cd $(<D) && $(MAKE) install
	touch $@
################
$(PWD)/bs1770gain/Makefile: $(libdir)/libffsox_2$(exta)
$(PWD)/bs1770gain/Makefile: $(libdir)/lib1770_2$(exta)
$(PWD)/bs1770gain/Makefile: $(libdir)/libpbutil$(exta)
$(PWD)/bs1770gain/Makefile: $(srcdir)/bs1770gain/homegrown.makefile
$(PWD)/bs1770gain/Makefile: $(srcdir)/bs1770gain/homegrown.configure
	mkdir -p $(@D)
	cd $(@D) && $< $(conf-opts)
	touch $@

###############################################################################
libffsox-source:=$(shell find $(srcdir)/libffsox-2 -name '*.h' -or -name '*.c')
$(libdir)/libffsox_2$(exta): $(libffsox-source)
$(libdir)/libffsox_2$(exta): $(PWD)/libffsox-2/Makefile
	cd $(<D) && $(MAKE) install
	touch $@
################
$(PWD)/libffsox-2/Makefile: $(sox-sodir)/$(call so,,libsox,3)
$(PWD)/libffsox-2/Makefile: $(ffmpeg-sodir)/$(call so,lib,avutil,56)
$(PWD)/libffsox-2/Makefile: $(libdir)/libpbutil$(exta)
$(PWD)/libffsox-2/Makefile: $(libdir)/lib1770_2$(exta)
$(PWD)/libffsox-2/Makefile: $(srcdir)/libffsox-2/homegrown.makefile
$(PWD)/libffsox-2/Makefile: $(srcdir)/libffsox-2/homegrown.configure
	mkdir -p $(@D)
	cd $(@D) && $< $(conf-opts)
	touch $@

###############################################################################
lib1770-source:=$(shell find $(srcdir)/lib1770-2 -name '*.h' -or -name '*.c')
$(libdir)/lib1770_2$(exta): $(lib1770-source)
$(libdir)/lib1770_2$(exta): $(PWD)/lib1770-2/Makefile
	cd $(<D) && $(MAKE) install
	touch $@
################
$(PWD)/lib1770-2/Makefile: $(libdir)/libpbutil$(exta)
$(PWD)/lib1770-2/Makefile: $(srcdir)/lib1770-2/homegrown.makefile
$(PWD)/lib1770-2/Makefile: $(srcdir)/lib1770-2/homegrown.configure
	mkdir -p $(@D)
	cd $(@D) && $< $(conf-opts)
	touch $@

###############################################################################
libpbutil-source:=$(shell find $(srcdir)/libpbutil -name '*.h' -or -name '*.c')
$(libdir)/libpbutil$(exta): $(libpbutil-source)
$(libdir)/libpbutil$(exta): $(PWD)/libpbutil/Makefile
	cd $(<D) && $(MAKE) install
	touch $@
################
$(PWD)/libpbutil/Makefile: $(srcdir)/libpbutil/homegrown.makefile
$(PWD)/libpbutil/Makefile: $(srcdir)/libpbutil/homegrown.configure
	mkdir -p $(@D)
	cd $(@D) && $< $(conf-opts)
	touch $@

###############################################################################
sox-opts+=--prefix=$(sox-prefix)
sox-opts+=--disable-openmp
sox-opts+=--with-gnu-ld
sox-opts+=--without-gsm
sox-opts+=--without-libltdl
ifneq (,$(mingw)) # [
sox-opts+=--disable-symlinks
sox-opts+=--disable-stack-protector
sox-opts+=--without-png
sox-opts+='CC=gcc -static-libgcc'
endif # ]

$(sox-sodir)/$(call so,,libsox,3): $(sox-builddir)/Makefile
	cd $(<D) && $(MAKE) install
$(sox-builddir)/Makefile: $(tooldir)/unpack/$(sox)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(sox-opts)
$(tooldir)/unpack/$(sox)/configure: $(tooldir)/download/$(sox).tar.bz2
	mkdir -p $(@D)/..
	cd $(@D)/.. && tar xvfj $<
	touch $@
$(tooldir)/download/$(sox).tar.bz2:
	mkdir -p $(@D)
	cd $(@D) && http://sourceforge.net/projects/sox/files/sox/14.4.2/$(@F)/download

###############################################################################
ffmpeg-opts+=--prefix=$(ffmpeg-prefix)
ffmpeg-opts+=--enable-gpl
ffmpeg-opts+=--enable-version3
#ffmpeg-opts+=--enable-nonfree
ffmpeg-opts+=--enable-shared
#ffmpeg-opts+=--disable-iconv
#ffmpeg-opts+=--disable-bzlib
#ffmpeg-opts+=--disable-zlib
#ffmpeg-opts+=--disable-lzma
ffmpeg-opts+=--disable-doc
ifneq (,$(msvc)) # [
ffmpeg-opts+=--disable-w32threads
ffmpeg-opts+='--extra-ldflags=-static-libgcc -static-libstdc++ -static'
endif # ]

$(ffmpeg-sodir)/$(call so,lib,avutil,56): $(ffmpeg-builddir)/Makefile
	cd $(<D) && $(MAKE) install
$(ffmpeg-builddir)/Makefile: $(tooldir)/unpack/ffmpeg/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(ffmpeg-opts)
$(tooldir)/unpack/ffmpeg/configure: $(tooldir)/download/ffmpeg-snapshot.tar.bz2
	mkdir -p $(@D)/..
	cd $(@D)/.. && tar xvfj $<
	touch $@
$(tooldir)/download/ffmpeg-snapshot.tar.bz2:
	mkdir -p $(@D)
	cd $(@D) && wget http://ffmpeg.org/releases/$(@F)
