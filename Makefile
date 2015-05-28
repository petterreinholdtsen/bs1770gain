include config.mak
include $(SRCDIR)/bs1770gain/version.mak
include $(SRCDIR)/examples/version.mak
include $(SRCDIR)/libffsox-2/version.mak
include $(SRCDIR)/lib1770-2/version.mak

.PHONY: all install release-bin release-src release-tools

###############################################################################
BS1770GAIN=bs1770gain-$(BS1770GAIN_VERSION)
BS1770GAIN_INSTALL+=$(BINDIR)/bs1770gain
install: $(BS1770GAIN_INSTALL)

########
EXAMPLES=examples-$(EXAMPLES_VERSION)
EXAMPLES_INSTALL+=$(BINDIR)/ffmpeg1770
EXAMPLES_INSTALL+=$(BINDIR)/sndfile1770
ifeq (yes,$(ENABLE_EXAMPLES)) # {
install: $(EXAMPLES_INSTALL)
endif # }

########
LIBPBUTIL=libpbutil-$(LIBPBUTIL_VERSION)
LIBPBUTIL_INSTALL+=$(LIBDIR)/libpbutil.a
install: $(LIBPBUTIL_INSTALL)

########
LIBFFSOX=libffsox-2-$(LIBFFSOX_VERSION)
LIBFFSOX_INSTALL+=$(LIBDIR)/libffsox-2.a
install: $(LIBFFSOX_INSTALL)

########
LIB1770=lib1770-2-$(LIB1770_VERSION)
LIB1770_INSTALL+=$(LIBDIR)/lib1770-2.a
install: $(LIB1770_INSTALL)

########
SOXV=14.4.1
SOX=sox-$(SOXV)
SOX_INSTALL+=$(INCLUDEDIR)/sox.h
ifeq (mingw,$(OS))
SOX_INSTALL+=$(BINDIR)/libsox-2.dll
else
SOX_INSTALL+=$(LIBDIR)/libsox.so.2
endif
#install: $(SOX_INSTALL)

########
LIBSNDFILE=libsndfile-1.0.25
LIBSNDFILE_INSTALL+=$(INCLUDEDIR)/sndfile.h
LIBSNDFILE_INSTALL+=$(LIBDIR)/libsndfile.a

########
LIBFLAC=flac-1.3.1
#LIBFLAC=flac-1.3.0
#LIBFLAC=flac-1.2.1
LIBFLAC_INSTALL+=$(INCLUDEDIR)/FLAC/all.h
LIBFLAC_INSTALL+=$(LIBDIR)/libFLAC.a
#install: $(LIBFLAC_INSTALL)

########
LIBVORBIS=libvorbis-1.3.4
LIBVORBIS_INSTALL+=$(INCLUDEDIR)/vorbis/codec.h
LIBVORBIS_INSTALL+=$(INCLUDEDIR)/vorbis/vorbisenc.h
LIBVORBIS_INSTALL+=$(INCLUDEDIR)/vorbis/vorbisfile.h
LIBVORBIS_INSTALL+=$(LIBDIR)/libvorbis.a
LIBVORBIS_INSTALL+=$(LIBDIR)/libvorbisenc.a
LIBVORBIS_INSTALL+=$(LIBDIR)/libvorbisfile.a
#install: $(LIBVORBIS_INSTALL)

########
LIBOGG=libogg-1.3.2
LIBOGG_INSTALL+=$(INCLUDEDIR)/ogg/ogg.h
LIBOGG_INSTALL+=$(LIBDIR)/libogg.a
#install: $(LIBOGG_INSTALL)

########
PKG_CONFIG=pkg-config-0.28
PKG_CONFIG_INSTALL+=$(BINDIR)/pkg-config
#install: $(PKG_CONFIG_INSTALL)

########
FFMPEG=ffmpeg
FFMPEG_INSTALL+=$(INCLUDEDIR)/libavutil/avutil.h
FFMPEG_INSTALL+=$(INCLUDEDIR)/libavcodec/avcodec.h
FFMPEG_INSTALL+=$(INCLUDEDIR)/libavformat/avformat.h
FFMPEG_INSTALL+=$(LIBDIR)/libavutil.a
FFMPEG_INSTALL+=$(LIBDIR)/libavcodec.a
FFMPEG_INSTALL+=$(LIBDIR)/libavformat.a
FFMPEG_INSTALL+=$(BINDIR)/ffmpeg
#install: $(FFMPEG_INSTALL)

########
YASM=yasm-1.3.0
YASM_INSTALL+=$(BINDIR)/yasm
#install: $(YASM_INSTALL)

###############################################################################
$(BS1770GAIN_INSTALL): build/$(BS1770GAIN)/Makefile FORCE
	cd $(<D) && $(MAKE) install
build/$(BS1770GAIN)/Makefile: $(SOX_INSTALL)
build/$(BS1770GAIN)/Makefile: $(FFMPEG_INSTALL)
build/$(BS1770GAIN)/Makefile: $(LIBFFSOX_INSTALL)
build/$(BS1770GAIN)/Makefile: $(LIB1770_INSTALL)
build/$(BS1770GAIN)/Makefile: $(LIBPBUTIL_INSTALL)
build/$(BS1770GAIN)/Makefile: $(SRCDIR)/bs1770gain/Makefile
build/$(BS1770GAIN)/Makefile: $(SRCDIR)/bs1770gain/configure
	mkdir -p $(@D)
	cd $(@D) && $< --prefix=$(PREFIX) --tooldir=$(TOOLDIR)
	touch $@

###############################################################################
$(EXAMPLES_INSTALL): build/$(EXAMPLES)/Makefile FORCE
	cd $(<D) && $(MAKE) install
build/$(EXAMPLES)/Makefile: $(LIB1770_INSTALL)
build/$(EXAMPLES)/Makefile: $(FFMPEG_INSTALL)
build/$(EXAMPLES)/Makefile: $(LIBSNDFILE_INSTALL)
build/$(EXAMPLES)/Makefile: $(SRCDIR)/examples/Makefile
build/$(EXAMPLES)/Makefile: $(SRCDIR)/examples/configure
	mkdir -p $(@D)
	cd $(@D) && $< --prefix=$(PREFIX) --tooldir=$(TOOLDIR)
	touch $@

###############################################################################
$(LIBFFSOX_INSTALL): build/$(LIBFFSOX)/Makefile FORCE
	cd $(<D) && $(MAKE) install
build/$(LIBFFSOX)/Makefile: $(LIBPBUTIL_INSTALL)
build/$(LIBFFSOX)/Makefile: $(SOX_INSTALL)
build/$(LIBFFSOX)/Makefile: $(FFMPEG_INSTALL)
build/$(LIBFFSOX)/Makefile: $(SRCDIR)/libffsox-2/Makefile
build/$(LIBFFSOX)/Makefile: $(SRCDIR)/libffsox-2/configure
	mkdir -p $(@D)
	cd $(@D) && $< --prefix=$(PREFIX) --tooldir=$(TOOLDIR)
	touch $@

###############################################################################
$(LIB1770_INSTALL): build/$(LIB1770)/Makefile FORCE
	cd $(<D) && $(MAKE) install
build/$(LIB1770)/Makefile: $(LIBPBUTIL_INSTALL)
build/$(LIB1770)/Makefile: $(SRCDIR)/lib1770-2/Makefile
build/$(LIB1770)/Makefile: $(SRCDIR)/lib1770-2/configure
	mkdir -p $(@D)
	cd $(@D) && $< --prefix=$(PREFIX) --tooldir=$(TOOLDIR)
	touch $@

###############################################################################
$(LIBPBUTIL_INSTALL): build/$(LIBPBUTIL)/Makefile FORCE
	cd $(<D) && $(MAKE) install
build/$(LIBPBUTIL)/Makefile: $(SRCDIR)/libpbutil/Makefile
build/$(LIBPBUTIL)/Makefile: $(SRCDIR)/libpbutil/configure
	mkdir -p $(@D)
	cd $(@D) && $< --prefix=$(PREFIX) --tooldir=$(TOOLDIR)
	touch $@

###############################################################################
SOX_OPTS+=--prefix=$(PREFIX)
SOX_OPTS+=--disable-gomp
SOX_OPTS+=--with-gnu-ld
SOX_OPTS+=--without-ladspa
SOX_OPTS+=--without-ffmpeg
SOX_OPTS+=--without-gsm
SOX_OPTS+=--without-libltdl
SOX_OPTS+='CPPFLAGS=-I$(INCLUDEDIR)'
SOX_OPTS+='LDFLAGS=-L$(LIBDIR)'
ifeq (mingw,$(OS)) # {
SOX_OPTS+=--disable-symlinks
SOX_OPTS+=--disable-shared
ifeq (mingw,$(OS)) # {
SOX_LIBS+=-lFLAC
SOX_LIBS+=-lvorbisfile
SOX_LIBS+=-lvorbisenc
SOX_LIBS+=-lvorbis
SOX_LIBS+=-logg
endif # }
SOX_LIBS+=-lwinmm
$(SOX_INSTALL): LDFLAGS=-L$(LIBDIR)
$(SOX_INSTALL): LIBS=$(SOX_LIBS)
$(SOX_INSTALL): $(LIBDIR)/libsox.a
$(LIBDIR)/libsox.a: build/$(SOX)/Makefile
	cd $(<D) && $(MAKE) install
else # } {
$(SOX_INSTALL): build/$(SOX)/Makefile
	cd $(<D) && $(MAKE) install
endif # }
ifeq (mingw,$(OS)) # {
#build/$(SOX)/Makefile: $(LIBSNDFILE_INSTALL)
build/$(SOX)/Makefile: $(LIBFLAC_INSTALL)
build/$(SOX)/Makefile: $(LIBVORBIS_INSTALL)
endif # }
build/$(SOX)/Makefile: $(UNPACKDIR)/$(SOX)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(SOX_OPTS)
	touch $@
$(UNPACKDIR)/$(SOX)/configure: $(TOOLDIR)/$(SOX).tar.bz2
	mkdir -p $(UNPACKDIR)
	tar xfvj $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/$(SOX).tar.bz2:
	mkdir -p $(@D)
	wget -O $@ http://sourceforge.net/projects/sox/files/sox/$(SOXV)/$(@F)/download

###############################################################################
LIBSNDFILE_OPTS+=--prefix=$(PREFIX)
ifeq (mingw,$(OS)) # {
LIBSNDFILE_OPTS+=--disable-shared
endif # }
LIBSNDFILE_OPTS+=--disable-sqlite
LIBSNDFILE_OPTS+=--disable-alsa
LIBSNDFILE_OPTS+=PKG_CONFIG_PATH=$(LIBDIR)/pkgconfig
LIBSNDFILE_OPTS+='PKG_CONFIG=$(BINDIR)/pkg-config'
LIBSNDFILE_OPTS+='FLAC_CFLAGS=-I$(INCLUDEDIR)'
LIBSNDFILE_OPTS+='FLAC_LIBS=-L$(LIBDIR) -lFLAC'
LIBSNDFILE_OPTS+='VORBIS_CFLAGS=-I$(INCLUDEDIR)'
LIBSNDFILE_OPTS+='VORBIS_LIBS=-L$(LIBDIR) -lvorbis'
LIBSNDFILE_OPTS+='VORBISENC_CFLAGS=-I$(INCLUDEDIR)'
LIBSNDFILE_OPTS+='VORBISENC_LIBS=-L$(LIBDIR) -lvorbisenc'
LIBSNDFILE_OPTS+='OGG_CFLAGS=-I$(INCLUDEDIR)'
LIBSNDFILE_OPTS+='OGG_LIBS=-L$(LIBDIR) -logg'
$(LIBSNDFILE_INSTALL): build/$(LIBSNDFILE)/Makefile
	cd $(<D) && $(MAKE) install
build/$(LIBSNDFILE)/Makefile: $(LIBFLAC_INSTALL)
build/$(LIBSNDFILE)/Makefile: $(LIBVORBIS_INSTALL)
build/$(LIBSNDFILE)/Makefile: $(PKG_CONFIG_INSTALL)
build/$(LIBSNDFILE)/Makefile: $(UNPACKDIR)/$(LIBSNDFILE)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(LIBSNDFILE_OPTS)
	touch $@
$(UNPACKDIR)/$(LIBSNDFILE)/configure: $(TOOLDIR)/$(LIBSNDFILE).tar.gz
	mkdir -p $(UNPACKDIR)
	tar xfvz $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/$(LIBSNDFILE).tar.gz:
	mkdir -p $(@D)
	wget -O $@ http://www.mega-nerd.com/libsndfile/files/$(@F)

###############################################################################
LIBFLAC_OPTS+=--prefix=$(PREFIX)
LIBFLAC_OPTS+=--enable-static
ifeq (mingw,$(OS)) # {
LIBFLAC_OPTS+=--disable-shared
endif # }
LIBFLAC_OPTS+=--disable-xmms-plugin
LIBFLAC_OPTS+=--disable-doxygen-docs
LIBFLAC_OPTS+=--disable-oggtest
$(LIBFLAC_INSTALL): build/$(LIBFLAC)/Makefile
	cd $(<D) && $(MAKE) all install
build/$(LIBFLAC)/Makefile: $(LIBOGG_INSTALL)
build/$(LIBFLAC)/Makefile: $(UNPACKDIR)/$(LIBFLAC)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(LIBFLAC_OPTS)
	touch $@
$(UNPACKDIR)/$(LIBFLAC)/configure: $(TOOLDIR)/$(LIBFLAC).tar.xz
	mkdir -p $(UNPACKDIR)
	tar xfvJ $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/$(LIBFLAC).tar.xz:
	mkdir -p $(@D)
	wget -O $@ http://downloads.xiph.org/releases/flac/$(@F)
#	wget -O $@ http://sourceforge.net/projects/flac/files/flac-src/$(LIBFLAC)-src/$(@F)/download

###############################################################################
LIBVORBIS_OPTS+=--prefix=$(PREFIX)
ifeq (mingw,$(OS)) # {
LIBVORBIS_OPTS+=--disable-shared
endif # }
$(LIBVORBIS_INSTALL): build/$(LIBVORBIS)/Makefile
	cd $(<D) && $(MAKE) install
build/$(LIBVORBIS)/Makefile: $(LIBOGG_INSTALL)
build/$(LIBVORBIS)/Makefile: $(UNPACKDIR)/$(LIBVORBIS)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(LIBVORBIS_OPTS)
	touch $@
$(UNPACKDIR)/$(LIBVORBIS)/configure: $(TOOLDIR)/$(LIBVORBIS).tar.gz
	mkdir -p $(UNPACKDIR)
	tar xfvz $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/$(LIBVORBIS).tar.gz:
	mkdir -p $(@D)
	wget -O $@ http://downloads.xiph.org/releases/vorbis/$(@F)

###############################################################################
LIBOGG_OPTS+=--prefix=$(PREFIX)
ifeq (mingw,$(OS)) # {
LIBOGG_OPTS+=--disable-shared
endif # }
$(LIBOGG_INSTALL): build/$(LIBOGG)/Makefile
	cd $(<D) && $(MAKE) install
build/$(LIBOGG)/Makefile: $(UNPACKDIR)/$(LIBOGG)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(LIBOGG_OPTS)
	touch $@
$(UNPACKDIR)/$(LIBOGG)/configure: $(TOOLDIR)/$(LIBOGG).tar.gz
	mkdir -p $(UNPACKDIR)
	tar xfvz $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/$(LIBOGG).tar.gz:
	mkdir -p $(@D)
	wget -O $@ http://downloads.xiph.org/releases/ogg/$(@F)

###############################################################################
PKG_CONFIG_OPTS+='--prefix=$(PREFIX)'
PKG_CONFIG_OPTS+=--disable-shared
PKG_CONFIG_OPTS+=--with-internal-glib

$(PKG_CONFIG_INSTALL): build/$(PKG_CONFIG)/Makefile
	cd $(<D) && $(MAKE) install
build/$(PKG_CONFIG)/Makefile: $(UNPACKDIR)/$(PKG_CONFIG)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(PKG_CONFIG_OPTS)
	touch $@
$(UNPACKDIR)/$(PKG_CONFIG)/configure: $(TOOLDIR)/$(PKG_CONFIG).tar.gz
	mkdir -p $(UNPACKDIR)
	tar xfvz $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/$(PKG_CONFIG).tar.gz:
	mkdir -p $(@D)
	wget -O $@ pkgconfig.freedesktop.org/releases/$(@F)
#bin/pkg-config: build/$(PKG_CONFIG)/Makefile
#	cd $(<D) && $(MAKE) && $(MAKE) install
#build/$(PKG_CONFIG)/Makefile: OPTS=$(PKG_CONFIG_OPTS)
#
#$(TOOLDIR)/$(PKG_CONFIG).tar.gz:
#	$(call DOWNLOAD,http://pkgconfig.freedesktop.org/releases//$(@F))


###############################################################################
FFMPEG_OPTS+=--prefix=$(PREFIX)
FFMPEG_OPTS+=--yasmexe=$(BINDIR)/yasm
FFMPEG_OPTS+=--enable-gpl
FFMPEG_OPTS+=--enable-version3
FFMPEG_OPTS+=--enable-nonfree
FFMPEG_OPTS+=--enable-shared
FFMPEG_OPTS+=--disable-doc
FFMPEG_OPTS+=--disable-w32threads
ifeq (mingw,$(OS)) # {
FFMPEG_OPTS+='--extra-ldflags=-static-libgcc -static-libstdc++'
FFMPEG_OPTS+='--extra-libs=$(shell gcc -print-file-name=libwinpthread.a)'
endif # }
$(FFMPEG_INSTALL): build/$(FFMPEG)/Makefile
	cd $(<D) && $(MAKE) install
build/$(FFMPEG)/Makefile: $(YASM_INSTALL)
build/$(FFMPEG)/Makefile: $(UNPACKDIR)/$(FFMPEG)/configure
	rm -rf $(@D)
	mkdir -p $(@D)
	cd $(@D) && $< $(FFMPEG_OPTS)
	touch $@
$(UNPACKDIR)/$(FFMPEG)/configure: $(TOOLDIR)/ffmpeg-snapshot.tar.bz2
	rm -rf $(@D)
	mkdir -p $(UNPACKDIR)
	tar xfvj $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/ffmpeg-snapshot.tar.bz2:
	mkdir -p $(@D)
	wget -O $@ http://ffmpeg.org/releases/$(@F)

###############################################################################
YASM_OPTS+=--prefix=$(PREFIX)
$(YASM_INSTALL): build/$(YASM)/Makefile
	cd $(<D) && $(MAKE) install
build/$(YASM)/Makefile: $(UNPACKDIR)/$(YASM)/configure
	mkdir -p $(@D)
	cd $(@D) && $< $(YASM_OPTS)
	touch $@
$(UNPACKDIR)/$(YASM)/configure: $(TOOLDIR)/$(YASM).tar.gz
	mkdir -p $(UNPACKDIR)
	tar xfvz $< -C $(UNPACKDIR)
	touch $@
$(TOOLDIR)/$(YASM).tar.gz:
	mkdir -p $(@D)
	wget -O $@ www.tortall.net/projects/yasm/releases/$(@F)

###############################################################################
RELEASE=bs1770gain-$(BS1770GAIN_VERSION)

RELEASE_BIN+=-C $(SRCDIR)/bs1770gain
RELEASE_BIN+=doc
RELEASE_BIN+=-C $(PREFIX)
RELEASE_BIN+=bin/bs1770gain-tools
release-bin: $(BS1770GAIN_INSTALL)
ifeq (mingw,$(OS))
RELEXT=win$(BITS)
RELEASE_BIN+=bin/bs1770gain.exe
release-bin: $(RELDIR)/$(RELEASE)/$(RELEASE)-$(RELEXT).7z
else
RELEXT=$(OS)$(BITS)
RELEASE_BIN+=bin/bs1770gain
release-bin: $(RELDIR)/$(RELEASE)/$(RELEASE)-$(RELEXT).tar.bz2
endif

RELEASE_SRC+=-C $(SRCDIR)
RELEASE_SRC+=COPYING
RELEASE_SRC+=bits
RELEASE_SRC+=configure
RELEASE_SRC+=Makefile
RELEASE_SRC+=libffsox-2
RELEASE_SRC+=lib1770-2
RELEASE_SRC+=examples
RELEASE_SRC+=bs1770gain
release-src: $(RELDIR)/$(RELEASE)/$(RELEASE)-src.tar.gz

RELEASE_TOOLS+=-C $(dir $(TOOLDIR))
RELEASE_TOOLS+=$(notdir $(TOOLDIR))
release-tools: $(RELDIR)/$(RELEASE)/$(RELEASE)-tools.tar.gz

%.7z: %.tar.bz2
	rm -rf $@ tmp
	mkdir -p tmp
	tar xfvj $< -C tmp
	cd tmp && 7za a $@ *
	rm -rf tmp $<
$(RELDIR)/$(RELEASE)/$(RELEASE)-$(RELEXT).tar.bz2: FORCE
	mkdir -p $(@D)
	tar cfvj $@ $(RELEASE_BIN) '--transform=s,^,$(RELEASE)/,g'
$(RELDIR)/$(RELEASE)/$(RELEASE)-src.tar.gz: FORCE
	mkdir -p $(@D)
	tar cfvz $@ $(RELEASE_SRC) '--transform=s,^,$(RELEASE)/,g'
$(RELDIR)/$(RELEASE)/$(RELEASE)-tools.tar.gz: FORCE
	mkdir -p $(@D)
	tar cfvz $@ $(RELEASE_TOOLS) '--transform=s,^,$(RELEASE)/,g'

###############################################################################
ifeq (mingw,$(OS)) # {
.PRECIOUS: %-2.dll
%-2.dll: %.dll
	cp $< $@

.PRECIOUS: $(LIBDIR)/%.dll.a $(LIBDIR)/%.def
$(BINDIR)/%.dll $(LIBDIR)/%.dll.a: $(LIBDIR)/%.def $(LIBDIR)/%.a
	mkdir -p $(@D)
	gcc -shared -static-libgcc -o $@ -Wl,--out-implib,lib/$(@F).a $^ $(LDFLAGS) $(LIBS)
$(LIBDIR)/%.def: $(LIBDIR)/%.a
	echo "EXPORTS" > $@
ifeq (64,$(BITS)) # {
	nm $<|sed -n "s/.* \(D\|R\) \(.*\)/\2 DATA/p" >> $@
	nm $<|sed -n "s/.* T //p" >> $@
else # } {
	nm $<|sed -n "s/.* \(D\|R\) _\(.*\)/\2 DATA/p" >> $@
	nm $<|sed -n "s/.* T _//p" >> $@
endif # }
endif # }

###############################################################################
.PHONY: FORCE
FORCE:
