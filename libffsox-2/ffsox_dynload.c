/*
 * ffsox_dynload.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#define FFSOX_DYNLOAD_PRIV
#include <ffsox_priv.h>
#if defined (FFSOX_DYNLOAD) // [
#if ! defined (_WIN32) // [
#include <unistd.h>
#include <dlfcn.h>
#endif // ]

///////////////////////////////////////////////////////////////////////////////
ffsox_avutil_t ffsox_avutil;
ffsox_avcodec_t ffsox_avcodec;
ffsox_avformat_t ffsox_avformat;
ffsox_libsox_t ffsox_libsox;

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
#define FFSOX_UNLOAD(lib) FreeLibrary(lib)

#define FFSOX_BIND(hLib,app,np) do { \
  if (NULL==(*(app)=(void *)GetProcAddress(hLib,np))) { \
    fprintf(stderr,"Error loding symbol \"%s\".\n",np); \
    goto loadlib; \
  } \
} while (0)

static wchar_t *ffsox_root(void)
{
  int size,len;
  wchar_t *buf,*bp;

  size=MAX_PATH;

  if (NULL==(buf=MALLOC(size*sizeof *buf)))
    goto malloc;

  for (;;) {
    len=GetModuleFileNameW(NULL,buf,size-1);

	  if (ERROR_INSUFFICIENT_BUFFER==GetLastError()) {
	    if (NULL==(bp=REALLOC(buf,(size*=2)*sizeof *buf)))
	      goto realloc;
	    else
	      buf=bp;
	  }
	  else
	    break;
  }

  bp=buf+len;

  while (buf<bp) {
    *bp--=0;

    if (L'/'==*bp||L'\\'==*bp) {
      *bp--=0;
      break;
    }
  }

  return buf;
realloc:
  FREE(buf);
malloc:
  return NULL;
}

static HMODULE ffsox_loadlib_try(const wchar_t *ws1, const char *s2,
    const char *s3)
{
  HMODULE hLib=NULL;
  wchar_t *path;

  if (NULL==(path=ffsox_path3(ws1,s2,s3)))
    goto path;

  hLib=LoadLibraryW(path);
  FREE(path);
path:
  return hLib;
}

HMODULE ffsox_loadlib(const wchar_t *root, const char *dirname,
    const char *basename)
{
  HMODULE hLib;
  wchar_t *path,*cur,*next;

  hLib=NULL;

  if (NULL!=(path=_wgetenv(L"PATH"))) {
    if (NULL==(path=_WCSDUP(path)))
      goto path;

    cur=wcstok_r(path,L";",&next);

    while (NULL!=cur) {
      if (NULL!=(hLib=ffsox_loadlib_try(cur,NULL,basename))) {
        FREE(path);
        goto found; 
      }

      cur=wcstok_r(NULL,L";",&next);
    }

    FREE(path);
  }
path:
  hLib=ffsox_loadlib_try(root,dirname,basename);
found:
  return hLib;
}
#else // ] [
#define FFSOX_UNLOAD(lib) dlclose(lib)

#define FFSOX_BIND(lib,app,np) do { \
  if (NULL==(*(app)=dlsym(lib,np))) { \
    fprintf(stderr,"Error loding symbol \"%s\".\n",np); \
    goto loadlib; \
  } \
} while (0)

static char *ffsox_root(void)
{
  char path[64];
  char *buf,*bp;
  int size,len;

  sprintf(path,"/proc/%d/exe",getpid());
  size=PATH_MAX;

  if (NULL==(buf=MALLOC(size*sizeof *buf)))
    goto malloc;

  while ((len=readlink(path,buf,size-1))<0) {
    if (ENAMETOOLONG!=errno)
	    goto realloc;
	  else if (NULL==(bp=REALLOC(buf,(size*=2)*sizeof *buf)))
	    goto realloc;
	  else
	    buf=bp;
  }

  bp=buf+len;

  while (buf<bp) {
    *bp--=0;

    if (L'/'==*bp) {
      *bp--=0;
      break;
    }
  }

  return buf;
realloc:
  FREE(buf);
malloc:
  return NULL;
}

static void *ffsox_loadlib_try(const char *s1, const char *s2,
    const char *s3)
{
  void *lib=NULL;
  char *path;

  if (NULL==(path=ffsox_path3(s1,s2,s3)))
    goto path;

  lib=dlopen(path,RTLD_LAZY);
  FREE(path);
path:
  return lib;
}

static void *ffsox_loadlib(const char *root, const char *dirname,
    const char *basename)
{
  void *lib;
  char *path,*cur,*next;

  if (NULL!=(lib=ffsox_loadlib_try("/usr/lib",NULL,basename)))
    goto found;

  if (NULL!=(path=getenv("LD_LIBRARY_PATH"))) {
    if (NULL==(path=STRDUP(path)))
      goto path;

    cur=strtok_r(path,":",&next);

    while (NULL!=cur) {
      if (NULL!=(lib=ffsox_loadlib_try(cur,NULL,basename))) {
        FREE(path);
        goto found;
      }

      cur=strtok_r(NULL,":",&next);
    }

    FREE(path);
  }
path:
  lib=ffsox_loadlib_try(root,dirname,basename);
found:
  return lib;
}
#endif // ]

///////////////////////////////////////////////////////////////////////////////
static int ffsox_dynload_avutil(void *lib)
{
  int code=-1;

  FFSOX_BIND(lib,&ffsox_avutil.avutil_version,"avutil_version");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_alloc,"av_frame_alloc");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_free,"av_frame_free");
  FFSOX_BIND(lib,&ffsox_avutil.av_get_channel_layout_nb_channels,
      "av_get_channel_layout_nb_channels");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_get_best_effort_timestamp,
      "av_frame_get_best_effort_timestamp");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_set_best_effort_timestamp,
      "av_frame_set_best_effort_timestamp");
  FFSOX_BIND(lib,&ffsox_avutil.av_log_get_level,"av_log_get_level");
  FFSOX_BIND(lib,&ffsox_avutil.av_log_set_level,"av_log_set_level");
  FFSOX_BIND(lib,&ffsox_avutil.av_get_sample_fmt_name,
      "av_get_sample_fmt_name");
  FFSOX_BIND(lib,&ffsox_avutil.av_log,"av_log");
  FFSOX_BIND(lib,&ffsox_avutil.av_rescale_q,"av_rescale_q");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_set_channel_layout,
      "av_frame_set_channel_layout");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_get_channel_layout,
      "av_frame_get_channel_layout");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_set_channels,"av_frame_set_channels");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_get_channels,"av_frame_get_channels");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_set_sample_rate,
      "av_frame_set_sample_rate");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_get_sample_rate,
      "av_frame_get_sample_rate");
  FFSOX_BIND(lib,&ffsox_avutil.av_samples_alloc,"av_samples_alloc");
  FFSOX_BIND(lib,&ffsox_avutil.av_free,"av_free");
  FFSOX_BIND(lib,&ffsox_avutil.av_freep,"av_freep");
  FFSOX_BIND(lib,&ffsox_avutil.av_dict_get,"av_dict_get");
  FFSOX_BIND(lib,&ffsox_avutil.av_dict_set,"av_dict_set");
  FFSOX_BIND(lib,&ffsox_avutil.av_dict_free,"av_dict_free");
  FFSOX_BIND(lib,&ffsox_avutil.av_frame_get_buffer,"av_frame_get_buffer");
#if defined (FFSOX_FILTER_CHANNELS) // [
  FFSOX_BIND(lib,&ffsox_avutil.av_get_channel_layout_channel_index,
      "av_get_channel_layout_channel_index");
#endif // ]

  code=0;
loadlib:
  return code;
}

static int ffsox_dynload_swresample(void *lib)
{
#if 0 // {
  int code=-1;

  code=0;
loadlib:
  return code;
#else // } {
  (void)lib;

  return 0;
#endif // }
}

static int ffsox_dynload_avcodec(void *lib)
{
  int code=-1;

  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_version,"avcodec_version");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_find_decoder,"avcodec_find_decoder");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_find_decoder_by_name,
      "avcodec_find_decoder_by_name");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_find_encoder,"avcodec_find_encoder");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_open2,"avcodec_open2");
  FFSOX_BIND(lib,&ffsox_avcodec.av_init_packet,"av_init_packet");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_decode_audio4,"avcodec_decode_audio4");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_encode_audio2,"avcodec_encode_audio2");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_decode_video2,"avcodec_decode_video2");
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // {
  FFSOX_BIND(lib,&ffsox_avcodec.av_free_packet,"av_free_packet");
#else // } {
  FFSOX_BIND(lib,&ffsox_avcodec.av_packet_unref,"av_packet_unref");
#endif // }
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_close,"avcodec_close");
  FFSOX_BIND(lib,&ffsox_avcodec.avcodec_copy_context,"avcodec_copy_context");
  FFSOX_BIND(lib,&ffsox_avcodec.av_packet_rescale_ts,"av_packet_rescale_ts");

  code=0;
loadlib:
  return code;
}

static int ffsox_dynload_avformat(void *lib)
{
  int code=-1;

  FFSOX_BIND(lib,&ffsox_avformat.avformat_version,"avformat_version");
  FFSOX_BIND(lib,&ffsox_avformat.av_register_all,"av_register_all");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_open_input,"avformat_open_input");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_find_stream_info,
      "avformat_find_stream_info");
  FFSOX_BIND(lib,&ffsox_avformat.av_read_frame,"av_read_frame");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_close_input,"avformat_close_input");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_alloc_output_context2,
      "avformat_alloc_output_context2");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_free_context,
      "avformat_free_context");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_new_stream,"avformat_new_stream");
  FFSOX_BIND(lib,&ffsox_avformat.avio_open,"avio_open");
  FFSOX_BIND(lib,&ffsox_avformat.avio_close,"avio_close");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_write_header,
      "avformat_write_header");
  FFSOX_BIND(lib,&ffsox_avformat.av_interleaved_write_frame,
      "av_interleaved_write_frame");
  FFSOX_BIND(lib,&ffsox_avformat.av_write_trailer,"av_write_trailer");
  FFSOX_BIND(lib,&ffsox_avformat. av_find_default_stream_index,
      "av_find_default_stream_index");
  FFSOX_BIND(lib,&ffsox_avformat.avformat_seek_file,"avformat_seek_file");
  FFSOX_BIND(lib,&ffsox_avformat.av_dump_format,"av_dump_format");

  code=0;
loadlib:
  return code;
}

///////////////////////////////////////////////////////////////////////////////
static int ffsox_dynload_libsox(void *lib)
{
  int code=-1;

  FFSOX_BIND(lib,&ffsox_libsox.sox_init,"sox_init");
  FFSOX_BIND(lib,&ffsox_libsox.sox_quit,"sox_quit");
  FFSOX_BIND(lib,&ffsox_libsox.sox_create_effects_chain,
      "sox_create_effects_chain");
  FFSOX_BIND(lib,&ffsox_libsox.sox_delete_effects_chain,
      "sox_delete_effects_chain");
  FFSOX_BIND(lib,&ffsox_libsox.sox_find_effect,"sox_find_effect");
  FFSOX_BIND(lib,&ffsox_libsox.sox_create_effect,"sox_create_effect");
  FFSOX_BIND(lib,&ffsox_libsox.sox_effect_options,"sox_effect_options");
  FFSOX_BIND(lib,&ffsox_libsox.sox_add_effect,"sox_add_effect");
  FFSOX_BIND(lib,&ffsox_libsox.sox_flow_effects,"sox_flow_effects");
  FFSOX_BIND(lib,&ffsox_libsox.sox_open_read,"sox_open_read");
  FFSOX_BIND(lib,&ffsox_libsox.sox_read,"sox_read");
  FFSOX_BIND(lib,&ffsox_libsox.sox_open_write,"sox_open_write");
  FFSOX_BIND(lib,&ffsox_libsox.sox_write,"sox_write");
  FFSOX_BIND(lib,&ffsox_libsox.sox_close,"sox_close");
  FFSOX_BIND(lib,&ffsox_libsox.sox_init_encodinginfo,"sox_init_encodinginfo");

  code=0;
loadlib:
  return code;
}

///////////////////////////////////////////////////////////////////////////////
static void *avutil;
static void *swresample;
static void *avcodec;
static void *avformat;
static void *libsox;

int ffsox_dynload(const char *dirname)
{
  int code=-1;
#if defined (_WIN32) // {
  wchar_t *root;
#else // } {
  char *root;
#endif // }

  if (NULL==dirname||'/'==dirname[0])
    root=NULL;
#if defined (_WIN32) // {
  else if ('\\'==dirname[0]||(dirname[0]!=0&&dirname[1]==':'))
    root=NULL;
#endif // }
  else if (NULL==(root=ffsox_root()))
    goto exit;

  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(avutil=ffsox_loadlib(root,dirname,FFSOX_AVUTIL))) {
    DMESSAGE("loading avutil");
    goto root;
  }
    
  if (ffsox_dynload_avutil(avutil)<0) {
    DMESSAGE("loading avutil symbols");
    goto root;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(swresample=ffsox_loadlib(root,dirname,FFSOX_SWRESAMPLE))) {
    DMESSAGE("loading swresample");
    goto root;
  }
    
  if (ffsox_dynload_swresample(swresample)<0) {
    DMESSAGE("loading swresample symbols");
    goto root;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(avcodec=ffsox_loadlib(root,dirname,FFSOX_AVCODEC))) {
    DMESSAGE("loading avcodec");
    goto root;
  }
    
  if (ffsox_dynload_avcodec(avcodec)<0) {
    DMESSAGE("loading avcodec symbols");
    goto root;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(avformat=ffsox_loadlib(root,dirname,FFSOX_AVFORMAT))) {
    DMESSAGE("loading avformat");
    goto root;
  }
    
  if (ffsox_dynload_avformat(avformat)<0) {
    DMESSAGE("loading avformat symbols");
    goto root;
  }
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(libsox=ffsox_loadlib(root,dirname,FFSOX_LIBSOX))) {
    DMESSAGE("loading sox");
    goto root;
  }
    
  if (ffsox_dynload_libsox(libsox)<0) {
    DMESSAGE("loading sox symbols");
    goto root;
  }
  
  code=0;
root:
  if (NULL!=root)
    FREE(root);
exit:
  return code;
}

void ffsox_unload(void)
{
  if (NULL!=libsox) {
    memset(&ffsox_libsox,0,sizeof ffsox_libsox);
    FFSOX_UNLOAD(libsox);
    libsox=NULL;
  }

  if (NULL!=avformat) {
    memset(&ffsox_avformat,0,sizeof ffsox_avformat);
    FFSOX_UNLOAD(avformat);
    avformat=NULL;
  }

  if (NULL!=avcodec) {
    memset(&ffsox_avcodec,0,sizeof ffsox_avcodec);
    FFSOX_UNLOAD(avcodec);
    avcodec=NULL;
  }

  if (NULL!=swresample) {
    //memset(&ffsox_swresample,0,sizeof ffsox_swresample);
    FFSOX_UNLOAD(swresample);
    swresample=NULL;
  }

  if (NULL!=avutil) {
    memset(&ffsox_avutil,0,sizeof ffsox_avutil);
    FFSOX_UNLOAD(avutil);
    avutil=NULL;
  }
}
#endif // ]
