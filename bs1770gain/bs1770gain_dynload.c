/*
 * bs1770gain_dynload.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@snafu.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#if defined (WIN32) // {
#include <windows.h>
#else // } {
#include <unistd.h>
#include <dlfcn.h>
#endif // }
#if ! defined (BS1770_DYNLOAD) // {
#define BS1770_DYNLOAD
#endif // }
#include <bs1770gain.h>

///////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_AVUTIL_V "54"
#define BS1770GAIN_SWRESAMPLE_V "1"
#define BS1770GAIN_AVCODEC_V "56"
#define BS1770GAIN_AVFORMAT_V "56"

#define BS1770GAIN_LIBSOX_V "2"

///////////////////////////////////////////////////////////////////////////////
bs1770gain_avutil_t bs1770gain_avutil;
bs1770gain_avcodec_t bs1770gain_avcodec;
bs1770gain_avformat_t bs1770gain_avformat;
bs1770gain_libsox_t bs1770gain_libsox;

///////////////////////////////////////////////////////////////////////////////
#if defined (WIN32) // {
#define BS1770GAIN_AVUTIL "avutil-" BS1770GAIN_AVUTIL_V ".dll"
#define BS1770GAIN_SWRESAMPLE "swresample-" BS1770GAIN_SWRESAMPLE_V  ".dll"
#define BS1770GAIN_AVCODEC "avcodec-" BS1770GAIN_AVCODEC_V  ".dll"
#define BS1770GAIN_AVFORMAT "avformat-" BS1770GAIN_AVFORMAT_V  ".dll"

#define BS1770GAIN_LIBSOX "libsox-" BS1770GAIN_LIBSOX_V  ".dll"

#define BS1770GAIN_BIND(hLib,app,np) do { \
  if (NULL==(*(app)=(void *)GetProcAddress(hLib,np))) { \
    fprintf(stderr,"Error loding symbol \"%s\".\n",np); \
    goto loadlib; \
  } /* \
  else \
    fprintf(stderr,"Symbol \"%s\" successfully loaded.\n",np); */ \
} while (0)

static wchar_t *bs1770gain_root(void)
{
  int size,len;
  wchar_t *buf,*bp;

  size=MAX_PATH;

  if (NULL==(buf=malloc(size*sizeof *buf)))
    goto error;

  for (;;) {
    len=GetModuleFileNameW(NULL,buf,size-1);

	  if (ERROR_INSUFFICIENT_BUFFER==GetLastError()) {
	    if (NULL==(bp=realloc(buf,(size*=2)*sizeof *buf)))
	      goto error;
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
error:
  if (NULL!=buf)
    free(buf);

  return NULL;
}

static HMODULE bs1770gain_loadlib_try(const wchar_t *ws1, const char *s2,
    const char *s3)
{
  HMODULE hLib=NULL;
  wchar_t *path;

  if (NULL==(path=bs1770gain_path3(ws1,s2,s3)))
    goto path;

  hLib=LoadLibraryW(path);
  free(path);
path:
  return hLib;
}

HMODULE bs1770gain_loadlib(const wchar_t *root, const char *dirname,
    const char *basename)
{
  HMODULE hLib;
  wchar_t *path,*cur,*next;

  hLib=NULL;

  if (NULL!=(path=_wgetenv(L"PATH"))) {
    if (NULL==(path=_wcsdup(path)))
      goto path;

    cur=wcstok_s(path,L";",&next);

    while (NULL!=cur) {
      if (NULL!=(hLib=bs1770gain_loadlib_try(cur,NULL,basename))) {
        free(path);
        goto found; 
      }

      cur=wcstok_s(NULL,L";",&next);
    }

    free(path);
  }
path:
  hLib=bs1770gain_loadlib_try(root,dirname,basename);
found:
  return hLib;
}
#else // } {
#define BS1770GAIN_AVUTIL "libavutil.so." BS1770GAIN_AVUTIL_V
#define BS1770GAIN_SWRESAMPLE "libswresample.so." BS1770GAIN_SWRESAMPLE_V
#define BS1770GAIN_AVCODEC "libavcodec.so." BS1770GAIN_AVCODEC_V
#define BS1770GAIN_AVFORMAT "libavformat.so." BS1770GAIN_AVFORMAT_V

#define BS1770GAIN_LIBSOX "libsox.so." BS1770GAIN_LIBSOX_V

#define BS1770GAIN_BIND(lib,app,np) do { \
  if (NULL==(*(app)=dlsym(lib,np))) { \
    fprintf(stderr,"Error loding symbol \"%s\".\n",np); \
    goto loadlib; \
  } /* \
  else \
    fprintf(stderr,"Symbol \"%s\" successfully loaded.\n",np); */ \
} while (0)

static char *bs1770gain_root(void)
{
  char path[64];
  char *buf,*bp;
  int size,len;

  sprintf(path,"/proc/%d/exe",getpid());
  size=PATH_MAX;

  if (NULL==(buf=malloc(size*sizeof *buf)))
    goto error;

  while ((len=readlink(path,buf,size-1))<0) {
    if (ENAMETOOLONG!=errno)
	    goto error;
	  else if (NULL==(bp=realloc(buf,(size*=2)*sizeof *buf)))
	    goto error;
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
error:
  if (NULL!=path)
    free(path);

  return NULL;
}

static void *bs1770gain_loadlib_try(const char *s1, const char *s2,
    const char *s3)
{
  void *lib=NULL;
  char *path;

  if (NULL==(path=bs1770gain_path3(s1,s2,s3)))
    goto path;

  lib=dlopen(path,RTLD_LAZY);
  free(path);
path:
  return lib;
}

static void *bs1770gain_loadlib(const char *root, const char *dirname,
    const char *basename)
{
  void *lib;
  char *path,*cur,*next;

  if (NULL!=(lib=bs1770gain_loadlib_try("/usr/lib",NULL,basename)))
    goto found;

  if (NULL!=(path=getenv("LD_LIBRARY_PATH"))) {
    if (NULL==(path=strdup(path)))
      goto path;

    cur=strtok_r(path,":",&next);

    while (NULL!=cur) {
      if (NULL!=(lib=bs1770gain_loadlib_try(cur,NULL,basename))) {
        free(path);
        goto found;
      }

      cur=strtok_r(NULL,":",&next);
    }

    free(path);
  }
path:
  lib=bs1770gain_loadlib_try(root,dirname,basename);
found:
  return lib;
}
#endif // }

///////////////////////////////////////////////////////////////////////////////
static int bs1770gain_dynload_avutil(void *lib)
{
  int code=-1;

  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_alloc,"av_frame_alloc");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_free,"av_frame_free");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_get_best_effort_timestamp,
      "av_frame_get_best_effort_timestamp");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_set_best_effort_timestamp,
      "av_frame_set_best_effort_timestamp");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_log_get_level,"av_log_get_level");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_log_set_level,"av_log_set_level");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_get_sample_fmt_name,
      "av_get_sample_fmt_name");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_log,"av_log");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_rescale_q,"av_rescale_q");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_set_channel_layout,
      "av_frame_set_channel_layout");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_get_channel_layout,
      "av_frame_get_channel_layout");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_set_channels,
      "av_frame_set_channels");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_get_channels,
      "av_frame_get_channels");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_set_sample_rate,
      "av_frame_set_sample_rate");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_frame_get_sample_rate,
      "av_frame_get_sample_rate");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_samples_alloc,"av_samples_alloc");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_free,"av_free");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_freep,"av_freep");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_dict_get,"av_dict_get");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_dict_set,"av_dict_set");
  BS1770GAIN_BIND(lib,&bs1770gain_avutil.av_dict_free,"av_dict_free");

  code=0;
loadlib:
  return code;
}

static int bs1770gain_dynload_swresample(void *lib)
{
#if 0 // {
  int code=-1;

  code=0;
loadlib:
  return code;
#else // } {
  return 0;
#endif // }
}

static int bs1770gain_dynload_avcodec(void *lib)
{
  int code=-1;

  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_find_decoder,
      "avcodec_find_decoder");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_find_decoder_by_name,
      "avcodec_find_decoder_by_name");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_find_encoder,
      "avcodec_find_encoder");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_open2,"avcodec_open2");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.av_init_packet,"av_init_packet");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_decode_audio4,
      "avcodec_decode_audio4");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_encode_audio2,
      "avcodec_encode_audio2");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_decode_video2,
      "avcodec_decode_video2");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.av_free_packet,"av_free_packet");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_close,"avcodec_close");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.avcodec_copy_context,
      "avcodec_copy_context");
  BS1770GAIN_BIND(lib,&bs1770gain_avcodec.av_packet_rescale_ts,
      "av_packet_rescale_ts");

  code=0;
loadlib:
  return code;
}

static int bs1770gain_dynload_avformat(void *lib)
{
  int code=-1;

  BS1770GAIN_BIND(lib,&bs1770gain_avformat.av_register_all,
      "av_register_all");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_open_input,
      "avformat_open_input");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_find_stream_info,
      "avformat_find_stream_info");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.av_read_frame,"av_read_frame");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_close_input,
      "avformat_close_input");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_alloc_output_context2,
      "avformat_alloc_output_context2");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_free_context,
      "avformat_free_context");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_new_stream,
      "avformat_new_stream");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avio_open,"avio_open");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avio_close,"avio_close");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_write_header,
      "avformat_write_header");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.av_interleaved_write_frame,
      "av_interleaved_write_frame");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.av_write_trailer,
      "av_write_trailer");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat. av_find_default_stream_index,
      "av_find_default_stream_index");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.avformat_seek_file,
      "avformat_seek_file");
  BS1770GAIN_BIND(lib,&bs1770gain_avformat.av_dump_format,"av_dump_format");

  code=0;
loadlib:
  return code;
}

///////////////////////////////////////////////////////////////////////////////
static int bs1770gain_dynload_libsox(void *lib)
{
  int code=-1;

  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_init,"sox_init");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_quit,"sox_quit");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_create_effects_chain,
      "sox_create_effects_chain");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_delete_effects_chain,
      "sox_delete_effects_chain");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_find_effect,
      "sox_find_effect");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_create_effect,
      "sox_create_effect");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_effect_options,
      "sox_effect_options");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_add_effect,"sox_add_effect");
  BS1770GAIN_BIND(lib,&bs1770gain_libsox.sox_flow_effects,
      "sox_flow_effects");

  code=0;
loadlib:
  return code;
}

///////////////////////////////////////////////////////////////////////////////
int bs1770gain_dynload(const char *dirname)
{
  int code=-1;
#if defined (WIN32) // {
  wchar_t *root;
#else // } {
  char *root;
#endif // }
  void *lib;

  if ('/'==dirname[0])
    root=NULL;
#if defined (WIN32) // {
  else if ('\\'==dirname[0]||(dirname[0]!=0&&dirname[1]==':'))
    root=NULL;
#endif // }
  else if (NULL==(root=bs1770gain_root()))
    goto exit;

  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(lib=bs1770gain_loadlib(root,dirname,BS1770GAIN_AVUTIL)))
    goto root;
    
  if (bs1770gain_dynload_avutil(lib)<0)
    goto root;
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(lib=bs1770gain_loadlib(root,dirname,BS1770GAIN_SWRESAMPLE)))
    goto root;
    
  if (bs1770gain_dynload_swresample(lib)<0)
    goto root;
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(lib=bs1770gain_loadlib(root,dirname,BS1770GAIN_AVCODEC)))
    goto root;
    
  if (bs1770gain_dynload_avcodec(lib)<0)
    goto root;
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(lib=bs1770gain_loadlib(root,dirname,BS1770GAIN_AVFORMAT)))
    goto root;
    
  if (bs1770gain_dynload_avformat(lib)<0)
    goto root;
  
  /////////////////////////////////////////////////////////////////////////////
  if (NULL==(lib=bs1770gain_loadlib(root,dirname,BS1770GAIN_LIBSOX)))
    goto root;
    
  if (bs1770gain_dynload_libsox(lib)<0)
    goto root;
  
  code=0;
root:
  if (NULL!=root)
    free(root);
exit:
  return code;
}
