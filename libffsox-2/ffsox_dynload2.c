/*
 * ffsox_dynload2.c
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
#include <pbutil_priv.h>
#include <ffsox_dynload.h>
#if defined (FFSOX_DYNLOAD2) // [
#if defined (_WIN32) // [
#include <windows.h>
#else // ] [
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#define DLOPEN_FLAG RTLD_LAZY
//#define DLOPEN_FLAG (RTLD_NOW|RTLD_GLOBAL)

///////////////////////////////////////////////////////////////////////////////
static struct _ffsox_avutil {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*avutil_version)(void);
  AVFrame *(*av_frame_alloc)(void);
  void (*av_frame_free)(AVFrame **frame);
  int (*av_get_channel_layout_nb_channels)(uint64_t channel_layout);
  int64_t (*av_frame_get_best_effort_timestamp)(const AVFrame *frame);
  void (*av_frame_set_best_effort_timestamp)(AVFrame *frame, int64_t val);
  int (*av_log_get_level)(void);
  void (*av_log_set_level)(int level);
  const char *(*av_get_sample_fmt_name)(enum AVSampleFormat sample_fmt);
#if 0 // [
  void (*av_log)(void *avcl, int level, const char *fmt, ...)
      av_printf_format(3, 4);
#else // ] [
  void (*av_vlog)(void *avcl, int level, const char *fmt, va_list vl);
#endif // ]
  int64_t (*av_rescale_q_rnd)(int64_t a, AVRational bq, AVRational cq,
      enum AVRounding) av_const;
  int64_t (*av_rescale_q)(int64_t a, AVRational bq, AVRational cq) av_const;
  void (*av_frame_set_channel_layout)(AVFrame *frame, int64_t val);
  int64_t (*av_frame_get_channel_layout)(const AVFrame *frame);
  void (*av_frame_set_channels)(AVFrame *frame, int val);
  int (*av_frame_get_channels)(const AVFrame *frame);
  void (*av_frame_set_sample_rate)(AVFrame *frame, int val);
  int  (*av_frame_get_sample_rate)(const AVFrame *frame);
  int (*av_samples_alloc)(uint8_t **audio_data, int *linesize,
      int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt,
      int align);
  void (*av_free)(void *ptr);
  void (*av_freep)(void *ptr);
  AVDictionaryEntry *(*av_dict_get)(const AVDictionary *m, const char *key,
      const AVDictionaryEntry *prev, int flags);
  int (*av_dict_set)(AVDictionary **pm, const char *key, const char *value,
      int flags);
  void (*av_dict_free)(AVDictionary **m);
  int (*av_frame_get_buffer)(AVFrame *frame, int align);
#if defined (FFSOX_FILTER_CHANNELS) // [
  int (*av_get_channel_layout_channel_index)(uint64_t channel_layout,
      int index);
#endif // ]
} avutil;

static int avutil_load(void *p, const char *sym);

unsigned avutil_version(void)
{
  if (avutil_load(&avutil.avutil_version,__func__)<0)
    return 0u;

  return avutil.avutil_version();
}

AVFrame *av_frame_alloc(void)
{
  if (avutil_load(&avutil.av_frame_alloc,__func__)<0)
    return NULL;

  return avutil.av_frame_alloc();
}

void av_frame_free(AVFrame **frame)
{
  if (avutil_load(&avutil.av_frame_free,__func__)<0)
    return;

  avutil.av_frame_free(frame);
}

int av_get_channel_layout_nb_channels(uint64_t channel_layout)
{
  if (avutil_load(&avutil.av_get_channel_layout_nb_channels,__func__)<0)
    return -1;

  return avutil.av_get_channel_layout_nb_channels(channel_layout);
}

int64_t av_frame_get_best_effort_timestamp(const AVFrame *frame)
{
  if (avutil_load(&avutil.av_frame_get_best_effort_timestamp,__func__)<0)
    return -1;

  return avutil.av_frame_get_best_effort_timestamp(frame);
}

void av_frame_set_best_effort_timestamp(AVFrame *frame, int64_t val)
{
  if (avutil_load(&avutil.av_frame_set_best_effort_timestamp,__func__)<0)
    return;

  avutil.av_frame_set_best_effort_timestamp(frame,val);
}

int av_log_get_level(void)
{
  if (avutil_load(&avutil.av_log_get_level,__func__)<0)
    return -1;

  return avutil.av_log_get_level();
}

void av_log_set_level(int level)
{
  if (avutil_load(&avutil.av_log_set_level,__func__)<0)
    return;

  avutil.av_log_set_level(level);
}

const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt)
{
  if (avutil_load(&avutil.av_get_sample_fmt_name,__func__)<0)
    return NULL;

  return avutil.av_get_sample_fmt_name(sample_fmt);
}

void av_log(void *avcl, int level, const char *fmt, ...)
{
  va_list ap;

  if (avutil_load(&avutil.av_vlog,__func__)<0)
    return;

  va_start(ap,fmt);
  avutil.av_vlog(avcl,level,fmt,ap);
  va_end(ap);
}

int64_t av_rescale_q_rnd(int64_t a, AVRational bq, AVRational cq,
    enum AVRounding rnd)
{
  if (avutil_load(&avutil.av_rescale_q_rnd,__func__)<0)
    return -1;

  return avutil.av_rescale_q_rnd(a,bq,cq,rnd);
}

int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq)
{
  if (avutil_load(&avutil.av_rescale_q,__func__)<0)
    return -1;

  return avutil.av_rescale_q(a,bq,cq);
}

void av_frame_set_channel_layout(AVFrame *frame, int64_t val)
{
  if (avutil_load(&avutil.av_frame_set_channel_layout,__func__)<0)
    return;

  avutil.av_frame_set_channel_layout(frame,val);
}

int64_t av_frame_get_channel_layout(const AVFrame *frame)
{
  if (avutil_load(&avutil.av_frame_get_channel_layout,__func__)<0)
    return -1;

  return avutil.av_frame_get_channel_layout(frame);
}

void av_frame_set_channels(AVFrame *frame, int val)
{
  if (avutil_load(&avutil.av_frame_set_channels,__func__)<0)
    return;

  avutil.av_frame_set_channels(frame,val);
}

int av_frame_get_channels(const AVFrame *frame)
{
  if (avutil_load(&avutil.av_frame_get_channels,__func__)<0)
    return -1;

  return avutil.av_frame_get_channels(frame);
}

void av_frame_set_sample_rate(AVFrame *frame, int val)
{
  if (avutil_load(&avutil.av_frame_set_sample_rate,__func__)<0)
    return;

  avutil.av_frame_set_sample_rate(frame,val);
}

int  av_frame_get_sample_rate(const AVFrame *frame)
{
  if (avutil_load(&avutil.av_frame_get_sample_rate,__func__)<0)
    return -1;

  return avutil.av_frame_get_sample_rate(frame);
}

int av_samples_alloc(uint8_t **audio_data, int *linesize,
    int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt,
    int align)
{
  if (avutil_load(&avutil.av_samples_alloc,__func__)<0)
    return -1;

  return avutil.av_samples_alloc(audio_data,linesize,nb_channels,
      nb_samples,sample_fmt,align);
}

void av_free(void *ptr)
{
  if (avutil_load(&avutil.av_free,__func__)<0)
    return;

  avutil.av_free(ptr);
}

void av_freep(void *ptr)
{
  if (avutil_load(&avutil.av_freep,__func__)<0)
    return;

  avutil.av_freep(ptr);
}

AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key,
    const AVDictionaryEntry *prev, int flags)
{
  if (avutil_load(&avutil.av_dict_get,__func__)<0)
    return NULL;

  return avutil.av_dict_get(m,key,prev,flags);
}

int av_dict_set(AVDictionary **pm, const char *key, const char *value,
    int flags)
{
  if (avutil_load(&avutil.av_dict_set,__func__)<0)
    return -1;

  return avutil.av_dict_set(pm,key,value,flags);
}

void av_dict_free(AVDictionary **m)
{
  if (avutil_load(&avutil.av_dict_free,__func__)<0)
    return;

  avutil.av_dict_free(m);
}

int av_frame_get_buffer(AVFrame *frame, int align)
{
  if (avutil_load(&avutil.av_frame_get_buffer,__func__)<0)
    return -1;

  return avutil.av_frame_get_buffer(frame,align);
}

#if defined (FFSOX_FILTER_CHANNELS) // [
int av_get_channel_layout_channel_index(uint64_t channel_layout,
    int index)
{
  if (avutil_load(&avutil.av_get_channel_layout_channel_index,__func__)<0)
    return -1;

  return avutil.av_get_channel_layout_channel_index(channel_layout,index);
}
#endif // ]

///////////////////////////////////////////////////////////////////////////////
static struct _ffsox_swresample {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*swresample_version)(void);
} swresample;

static int swresample_load(void *p, const char *sym);

unsigned swresample_version(void)
{
  if (swresample_load(&swresample.swresample_version,__func__)<0)
    return 0u;

  return swresample.swresample_version();
}

///////////////////////////////////////////////////////////////////////////////
static struct _ffsox_avcodec {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*avcodec_version)(void);
  AVCodec *(*avcodec_find_decoder)(enum AVCodecID id);
  AVCodec *(*avcodec_find_decoder_by_name)(const char *name);
  AVCodec *(*avcodec_find_encoder)(enum AVCodecID id);
  int (*avcodec_open2)(AVCodecContext *avctx, const AVCodec *codec,
      AVDictionary **options);
  void (*av_init_packet)(AVPacket *pkt);
  int (*avcodec_decode_audio4)(AVCodecContext *avctx, AVFrame *frame,
      int *got_frame_ptr, const AVPacket *avpkt);
  int (*avcodec_encode_audio2)(AVCodecContext *avctx, AVPacket *avpkt,
      const AVFrame *frame, int *got_packet_ptr);
  int (*avcodec_decode_video2)(AVCodecContext *avctx, AVFrame *picture,
      int *got_picture_ptr, const AVPacket *avpkt);
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // [
  attribute_deprecated
  void (*av_free_packet)(AVPacket *pkt);
#else // ] [
  void (*av_packet_unref)(AVPacket *pkt);
#endif // ]
  int (*avcodec_close)(AVCodecContext *avctx);
  int (*avcodec_copy_context)(AVCodecContext *dest, const AVCodecContext *src);
  void (*av_packet_rescale_ts)(AVPacket *pkt, AVRational tb_src,
      AVRational tb_dst);
} avcodec;

static int avcodec_load(void *p, const char *sym);

unsigned avcodec_version(void)
{
  if (avcodec_load(&avcodec.avcodec_version,__func__)<0)
    return 0u;

  return avcodec.avcodec_version();
}

AVCodec *avcodec_find_decoder(enum AVCodecID id)
{
  if (avcodec_load(&avcodec.avcodec_find_decoder,__func__)<0)
    return NULL;

  return avcodec.avcodec_find_decoder(id);
}

AVCodec *avcodec_find_decoder_by_name(const char *name)
{
  if (avcodec_load(&avcodec.avcodec_find_decoder_by_name,__func__)<0)
    return NULL;

  return avcodec.avcodec_find_decoder_by_name(name);
}

AVCodec *avcodec_find_encoder(enum AVCodecID id)
{
  if (avcodec_load(&avcodec.avcodec_find_encoder,__func__)<0)
    return NULL;

  return avcodec.avcodec_find_encoder(id);
}

int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec,
    AVDictionary **options)
{
  if (avcodec_load(&avcodec.avcodec_open2,__func__)<0)
    return -1;

  return avcodec.avcodec_open2(avctx,codec,options);
}

void av_init_packet(AVPacket *pkt)
{
  if (avcodec_load(&avcodec.av_init_packet,__func__)<0)
    return;

  avcodec.av_init_packet(pkt);
}

int avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
    int *got_frame_ptr, const AVPacket *avpkt)
{
  if (avcodec_load(&avcodec.avcodec_decode_audio4,__func__)<0)
    return -1;

  return avcodec.avcodec_decode_audio4(avctx,frame,got_frame_ptr,avpkt);
}

int avcodec_encode_audio2(AVCodecContext *avctx, AVPacket *avpkt,
    const AVFrame *frame, int *got_packet_ptr)
{
  if (avcodec_load(&avcodec.avcodec_encode_audio2,__func__)<0)
    return -1;

  return avcodec.avcodec_encode_audio2(avctx,avpkt,frame,got_packet_ptr);
}

int avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture,
    int *got_picture_ptr,const AVPacket *avpkt)
{
  if (avcodec_load(&avcodec.avcodec_decode_video2,__func__)<0)
    return -1;

  return avcodec.avcodec_decode_video2(avctx,picture,got_picture_ptr,avpkt);
}

#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // [
void av_free_packet(AVPacket *pkt)
{
  if (avcodec_load(&avcodec.av_free_packet,__func__)<0)
    return;

  avcodec.av_free_packet(pkt);
}
#else // ] [
void av_packet_unref(AVPacket *pkt)
{
  if (avcodec_load(&avcodec.av_packet_unref,__func__)<0)
    return;

  avcodec.av_packet_unref(pkt);
}
#endif // ]

int avcodec_close(AVCodecContext *avctx)
{
  if (avcodec_load(&avcodec.avcodec_close,__func__)<0)
    return -1;

  return avcodec.avcodec_close(avctx);
}

int avcodec_copy_context(AVCodecContext *dest, const AVCodecContext *src)
{
  if (avcodec_load(&avcodec.avcodec_copy_context,__func__)<0)
    return -1;

  return avcodec.avcodec_copy_context(dest,src);
}

void av_packet_rescale_ts(AVPacket *pkt, AVRational tb_src,
    AVRational tb_dst)
{
  if (avcodec_load(&avcodec.av_packet_rescale_ts,__func__)<0)
    return;

  avcodec.av_packet_rescale_ts(pkt,tb_src,tb_dst);
}

///////////////////////////////////////////////////////////////////////////////
static struct _ffsox_avformat {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  unsigned (*avformat_version)(void);
  void (*av_register_all)(void);
  int (*avformat_open_input)(AVFormatContext **ps, const char *filename,
      AVInputFormat *fmt, AVDictionary **options);
  int (*avformat_find_stream_info)(AVFormatContext *ic,
      AVDictionary **options);
  int (*av_read_frame)(AVFormatContext *s, AVPacket *pkt);
  void (*avformat_close_input)(AVFormatContext **s);
  int (*avformat_alloc_output_context2)(AVFormatContext **ctx,
      AVOutputFormat *oformat, const char *format_name,
      const char *filename);
  void (*avformat_free_context)(AVFormatContext *s);
  AVStream *(*avformat_new_stream)(AVFormatContext *s, const AVCodec *c);
  int (*avio_open)(AVIOContext **s, const char *url, int flags);
  int (*avio_close)(AVIOContext *s);
  int (*avformat_write_header)(AVFormatContext *s, AVDictionary **options);
  int (*av_interleaved_write_frame)(AVFormatContext *s, AVPacket *pkt);
  int (*av_write_trailer)(AVFormatContext *s);
  int (*av_find_default_stream_index)(AVFormatContext *s);
  int (*avformat_seek_file)(AVFormatContext *s, int stream_index,
      int64_t min_ts, int64_t ts, int64_t max_ts, int flags);
  void (*av_dump_format)(AVFormatContext *ic, int index, const char *url,
      int is_output);
} avformat;

static int avformat_load(void *p, const char *sym);

unsigned avformat_version(void)
{
  if (avformat_load(&avformat.avformat_version,__func__)<0)
    return 0u;

  return avformat.avformat_version();
}

void av_register_all(void)
{
  if (avformat_load(&avformat.av_register_all,__func__)<0)
    return;

  avformat.av_register_all();
}

int avformat_open_input(AVFormatContext **ps, const char *filename,
    AVInputFormat *fmt, AVDictionary **options)
{
  if (avformat_load(&avformat.avformat_open_input,__func__)<0)
    return -1;

  return avformat.avformat_open_input(ps,filename,fmt,options);
}

int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options)
{
  if (avformat_load(&avformat.avformat_find_stream_info,__func__)<0)
    return -1;

  return avformat.avformat_find_stream_info(ic,options);
}

int av_read_frame(AVFormatContext *s, AVPacket *pkt)
{
  if (avformat_load(&avformat.av_read_frame,__func__)<0)
    return -1;

  return avformat.av_read_frame(s,pkt);
}

void avformat_close_input(AVFormatContext **s)
{
  if (avformat_load(&avformat.avformat_close_input,__func__)<0)
    return;

  avformat.avformat_close_input(s);
}

int avformat_alloc_output_context2(AVFormatContext **ctx,
    AVOutputFormat *oformat, const char *format_name,
    const char *filename)
{
  if (avformat_load(&avformat.avformat_alloc_output_context2,__func__)<0)
    return -1;

  return avformat.avformat_alloc_output_context2(ctx,oformat,format_name,
      filename);
}

void avformat_free_context(AVFormatContext *s)
{
  if (avformat_load(&avformat.avformat_free_context,__func__)<0)
    return;

  avformat.avformat_free_context(s);
}

AVStream *avformat_new_stream(AVFormatContext *s, const AVCodec *c)
{
  if (avformat_load(&avformat.avformat_new_stream,__func__)<0)
    return NULL;

  return avformat.avformat_new_stream(s,c);
}

int avio_open(AVIOContext **s, const char *url, int flags)
{
  if (avformat_load(&avformat.avio_open,__func__)<0)
    return -1;

  return avformat.avio_open(s,url,flags);
}

int avio_close(AVIOContext *s)
{
  if (avformat_load(&avformat.avio_close,__func__)<0)
    return -1;

  return avformat.avio_close(s);
}

int avformat_write_header(AVFormatContext *s, AVDictionary **options)
{
  if (avformat_load(&avformat.avformat_write_header,__func__)<0)
    return -1;

  return avformat.avformat_write_header(s,options);
}

int av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt)
{
  if (avformat_load(&avformat.av_interleaved_write_frame,__func__)<0)
    return -1;

  return avformat.av_interleaved_write_frame(s,pkt);
}

int av_write_trailer(AVFormatContext *s)
{
  if (avformat_load(&avformat.av_write_trailer,__func__)<0)
    return -1;

  return avformat.av_write_trailer(s);
}

int av_find_default_stream_index(AVFormatContext *s)
{
  if (avformat_load(&avformat.av_find_default_stream_index,__func__)<0)
    return -1;

  return avformat.av_find_default_stream_index(s);
}

int avformat_seek_file(AVFormatContext *s, int stream_index,
    int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
{
  if (avformat_load(&avformat.avformat_seek_file,__func__)<0)
    return -1;

  return avformat.avformat_seek_file(s,stream_index,min_ts,ts,max_ts,flags);
}

void av_dump_format(AVFormatContext *ic, int index, const char *url,
    int is_output)
{
  if (avformat_load(&avformat.av_dump_format,__func__)<0)
    return;

  avformat.av_dump_format(ic,index,url,is_output);
}

///////////////////////////////////////////////////////////////////////////////
static struct _ffsox_libsox {
#if defined (_WIN32) // [
  HANDLE hLib;
#else // ] [
  void *hLib;
#endif // ]
  sox_version_info_t const *(LSX_API *sox_version_info)(void);
  int (LSX_API *sox_init)(void);
  int (LSX_API *sox_quit)(void);
  LSX_RETURN_OPT sox_effects_chain_t *(LSX_API *sox_create_effects_chain)(
      LSX_PARAM_IN sox_encodinginfo_t const * in_enc,
      LSX_PARAM_IN sox_encodinginfo_t const * out_enc);
  void (LSX_API *sox_delete_effects_chain)(
      LSX_PARAM_INOUT sox_effects_chain_t *ecp);
  LSX_RETURN_OPT sox_effect_handler_t const *(LSX_API *sox_find_effect)(
      LSX_PARAM_IN_Z char const * name);
  LSX_RETURN_OPT sox_effect_t *(LSX_API *sox_create_effect)(
      LSX_PARAM_IN sox_effect_handler_t const * eh);
  int (LSX_API *sox_effect_options)(
      LSX_PARAM_IN sox_effect_t *effp,
      int argc,
      LSX_PARAM_IN_COUNT(argc) char * const argv[]);
  int (LSX_API *sox_add_effect)(
      LSX_PARAM_INOUT sox_effects_chain_t * chain,
      LSX_PARAM_INOUT sox_effect_t * effp,
      LSX_PARAM_INOUT sox_signalinfo_t * in,
      LSX_PARAM_IN    sox_signalinfo_t const * out);
  int (LSX_API *sox_flow_effects)(
      LSX_PARAM_INOUT  sox_effects_chain_t * chain,
      LSX_PARAM_IN_OPT sox_flow_effects_callback callback,
      LSX_PARAM_IN_OPT void * client_data);
  LSX_RETURN_OPT sox_format_t *(LSX_API *sox_open_read)(
      LSX_PARAM_IN_Z char const * path,
      LSX_PARAM_IN_OPT sox_signalinfo_t const * signal,
      LSX_PARAM_IN_OPT sox_encodinginfo_t const * encoding,
      LSX_PARAM_IN_OPT_Z char const * filetype);
  size_t (LSX_API *sox_read)(
      LSX_PARAM_INOUT sox_format_t * ft,
      LSX_PARAM_OUT_CAP_POST_COUNT(len,return) sox_sample_t *buf,
      size_t len);
  LSX_RETURN_OPT sox_format_t *(LSX_API *sox_open_write)(
      LSX_PARAM_IN_Z char const * path,
      LSX_PARAM_IN sox_signalinfo_t const * signal,
      LSX_PARAM_IN_OPT sox_encodinginfo_t const * encoding,
      LSX_PARAM_IN_OPT_Z char const * filetype,
      LSX_PARAM_IN_OPT sox_oob_t const * oob,
      LSX_PARAM_IN_OPT sox_bool (LSX_API * overwrite_permitted)(
          LSX_PARAM_IN_Z char const * filename));
  size_t (LSX_API *sox_write)(
      LSX_PARAM_INOUT sox_format_t * ft,
      LSX_PARAM_IN_COUNT(len) sox_sample_t const * buf,
      size_t len);
  int (LSX_API *sox_close)(LSX_PARAM_INOUT sox_format_t * ft);
  void (LSX_API *sox_init_encodinginfo)(
      LSX_PARAM_OUT sox_encodinginfo_t * e);
} libsox;

static int libsox_load(void *p, const char *sym);

sox_version_info_t const *LSX_API sox_version_info(void)
{
  if (libsox_load(&libsox.sox_version_info,__func__)<0)
    return NULL;

  return libsox.sox_version_info();
}

int LSX_API sox_init(void)
{
  if (libsox_load(&libsox.sox_init,__func__)<0)
    return SOX_EOF;

  return libsox.sox_init();
}

int LSX_API sox_quit(void)
{
  if (libsox_load(&libsox.sox_quit,__func__)<0)
    return SOX_EOF;

  return libsox.sox_quit();
}

LSX_RETURN_OPT sox_effects_chain_t *LSX_API sox_create_effects_chain(
    LSX_PARAM_IN sox_encodinginfo_t const * in_enc,
    LSX_PARAM_IN sox_encodinginfo_t const * out_enc)
{
  if (libsox_load(&libsox.sox_create_effects_chain,__func__)<0)
    return NULL;

  return libsox.sox_create_effects_chain(in_enc,out_enc);
}

void LSX_API sox_delete_effects_chain(LSX_PARAM_INOUT sox_effects_chain_t *ecp)
{
  if (libsox_load(&libsox.sox_delete_effects_chain,__func__)<0)
    return;

  libsox.sox_delete_effects_chain(ecp);
}

LSX_RETURN_OPT sox_effect_handler_t const *LSX_API sox_find_effect(
    LSX_PARAM_IN_Z char const * name)
{
  if (libsox_load(&libsox.sox_find_effect,__func__)<0)
    return NULL;

  return libsox.sox_find_effect(name);
}

LSX_RETURN_OPT sox_effect_t *LSX_API sox_create_effect(
    LSX_PARAM_IN sox_effect_handler_t const * eh)
{
  if (libsox_load(&libsox.sox_create_effect,__func__)<0)
    return NULL;

  return libsox.sox_create_effect(eh);
}

int LSX_API sox_effect_options(
    LSX_PARAM_IN sox_effect_t *effp,
    int argc,
    LSX_PARAM_IN_COUNT(argc) char * const argv[])
{
  if (libsox_load(&libsox.sox_effect_options,__func__)<0)
    return SOX_EOF;

  return libsox.sox_effect_options(effp,argc,argv);
}

int LSX_API sox_add_effect(
    LSX_PARAM_INOUT sox_effects_chain_t * chain,
    LSX_PARAM_INOUT sox_effect_t * effp,
    LSX_PARAM_INOUT sox_signalinfo_t * in,
    LSX_PARAM_IN    sox_signalinfo_t const * out)
{
  if (libsox_load(&libsox.sox_add_effect,__func__)<0)
    return SOX_EOF;

  return libsox.sox_add_effect(chain,effp,in,out);
}

int LSX_API sox_flow_effects(
    LSX_PARAM_INOUT  sox_effects_chain_t * chain,
    LSX_PARAM_IN_OPT sox_flow_effects_callback callback,
    LSX_PARAM_IN_OPT void * client_data)
{
  if (libsox_load(&libsox.sox_flow_effects,__func__)<0)
    return SOX_EOF;

  return libsox.sox_flow_effects(chain,callback,client_data);
}

LSX_RETURN_OPT sox_format_t *LSX_API sox_open_read(
    LSX_PARAM_IN_Z char const * path,
    LSX_PARAM_IN_OPT sox_signalinfo_t const * signal,
    LSX_PARAM_IN_OPT sox_encodinginfo_t const * encoding,
    LSX_PARAM_IN_OPT_Z char const * filetype)
{
  if (libsox_load(&libsox.sox_open_read,__func__)<0)
    return 0u;

  return libsox.sox_open_read(path,signal,encoding,filetype);
}

size_t LSX_API sox_read(
    LSX_PARAM_INOUT sox_format_t * ft,
    LSX_PARAM_OUT_CAP_POST_COUNT(len,return) sox_sample_t *buf,
    size_t len)
{
  if (libsox_load(&libsox.sox_read,__func__)<0)
    return 0u;

  return libsox.sox_read(ft,buf,len);
}

LSX_RETURN_OPT sox_format_t *LSX_API sox_open_write(
    LSX_PARAM_IN_Z char const * path,
    LSX_PARAM_IN sox_signalinfo_t const * signal,
    LSX_PARAM_IN_OPT sox_encodinginfo_t const * encoding,
    LSX_PARAM_IN_OPT_Z char const * filetype,
    LSX_PARAM_IN_OPT sox_oob_t const * oob,
    LSX_PARAM_IN_OPT sox_bool (LSX_API * overwrite_permitted)(
        LSX_PARAM_IN_Z char const * filename))
{
  if (libsox_load(&libsox.sox_open_write,__func__)<0)
    return NULL;

  return libsox.sox_open_write(path,signal,encoding,filetype,oob,
      overwrite_permitted);
}

size_t LSX_API sox_write(
    LSX_PARAM_INOUT sox_format_t * ft,
    LSX_PARAM_IN_COUNT(len) sox_sample_t const * buf,
    size_t len)
{
  if (libsox_load(&libsox.sox_write,__func__)<0)
    return 0u;

  return libsox.sox_write(ft,buf,len);
}

int LSX_API sox_close(LSX_PARAM_INOUT sox_format_t * ft)
{
  if (libsox_load(&libsox.sox_close,__func__)<0)
    return SOX_EOF;

  return libsox.sox_close(ft);
}

void LSX_API sox_init_encodinginfo(LSX_PARAM_OUT sox_encodinginfo_t * e)
{
  if (libsox_load(&libsox.sox_init_encodinginfo,__func__)<0)
    return;

  libsox.sox_init_encodinginfo(e);
}

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
static wchar_t path[MAX_PATH],*pp=path;
#else // ] [
static char path[PATH_MAX],*pp=path;
#endif // ]

#if 0 // [
#if defined (_WIN32) // [
static int load(HMODULE hLib, const char *sym, void *p)
#else // ] [
static int load(void *hLib, const char *sym, void *p)
#endif // ]
{
  void **fp=p;

  if (!*fp) {
#if defined (_WIN32) // [
    *fp=GetProcAddress(hLib,sym);
#else // ] [
    *fp=dlsym(hLib,sym);
#endif // ]

    if (!*fp) {
      PBU_DVMESSAGE("loading %s",sym);
      return -1;
    }
  }

  return 0;
}
#else // ] [
#if defined (_WIN32) // [
static int load(HMODULE hLib, const char *sym, void *p)
#else // ] [
static int load(void *hLib, const char *sym, void *p)
#endif // ]
{
  void **fp=p;

  if (!*fp) {
#if defined (_WIN32) // [
    *fp=GetProcAddress(hLib,sym);
#else // ] [
    *fp=dlsym(hLib,sym);
#endif // ]

    if (!*fp) {
      PBU_DVMESSAGE("loading %s",sym);
      return -1;
    }
  }

  return 0;
}
#endif // ]

static int avutil_load(void *p,const char *sym)
{
#if defined (_WIN32) // [
  static const wchar_t AVUTIL[]=PBU_WIDEN(FFSOX_AVUTIL);
#else // ] [
  static const char AVUTIL[]=FFSOX_AVUTIL;
#endif // ]

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_AVUTIL=(sizeof AVUTIL)/(sizeof AVUTIL[0]),
  };

  if (!avutil.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_AVUTIL)) {
      PBU_DMESSAGE("loading avutil");
      return -1;
   }

#if defined (_WIN32) // [
    wcscpy(pp,AVUTIL);
    avutil.hLib=LoadLibraryW(path);
		DVWRITELNW(L"%p \"%s\"",avutil.hLib,path);
#else // ] [
    strcpy(pp,AVUTIL);
    avutil.hLib=dlopen(path,DLOPEN_FLAG);
		DVWRITELN("%p \"%s\"",avutil.hLib,path);
#endif // ]

#if defined (_WIN32) // [
    if (!avutil.hLib&&!(avutil.hLib=LoadLibraryW(AVUTIL))) {
      PBU_DMESSAGE("loading avutil");
		  DVWRITELNW(L"%p \"%s\"",avutil.hLib,path);
      return -1;
    }
#else // ] [
    if (!avutil.hLib&&!(avutil.hLib=dlopen(AVUTIL,DLOPEN_FLAG))) {
      PBU_DMESSAGE("loading avutil");
		  DVWRITELN(L"%p \"%s\"",avutil.hLib,path);
      return -1;
    }
#endif // ]
  }

  return load(avutil.hLib,sym,p);
}

static int swresample_load(void *p, const char *sym)
{
#if defined (_WIN32) // [
  static const wchar_t SWRESAMPLE[]=PBU_WIDEN(FFSOX_SWRESAMPLE);
#else // ] [
  static const char SWRESAMPLE[]=FFSOX_SWRESAMPLE;
#endif // ]

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_SWRESAMPLE=(sizeof SWRESAMPLE)/(sizeof SWRESAMPLE[0]),
  };

  if (!swresample.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_SWRESAMPLE)) {
      PBU_DMESSAGE("loading swresample");
      return -1;
   }

#if defined (_WIN32) // [
    wcscpy(pp,SWRESAMPLE);
    swresample.hLib=LoadLibraryW(path);
		DVWRITELNW(L"%p \"%s\"",avutil.hLib,path);
#else // ] [
    strcpy(pp,SWRESAMPLE);
    swresample.hLib=dlopen(path,DLOPEN_FLAG);
		DVWRITELN("%p \"%s\"",avutil.hLib,path);
#endif // ]

#if defined (_WIN32) // [
    if (!swresample.hLib&&!(swresample.hLib=LoadLibraryW(SWRESAMPLE))) {
      PBU_DMESSAGE("loading swresample");
		  DVWRITELNW(L"%p \"%s\"",avutil.hLib,path);
      return -1;
    }
#else // ] [
    if (!swresample.hLib&&!(swresample.hLib=dlopen(SWRESAMPLE,DLOPEN_FLAG))) {
      PBU_DMESSAGE("loading swresample");
		  DVWRITELN("%p \"%s\"",avutil.hLib,path);
      return -1;
    }
#endif // ]
  }

  return load(swresample.hLib,sym,p);
}

static int avcodec_load(void *p, const char *sym)
{
#if defined (_WIN32) // [
  static const wchar_t AVCODEC[]=PBU_WIDEN(FFSOX_AVCODEC);
#else // ] [
  static const char AVCODEC[]=FFSOX_AVCODEC;
#endif // ]

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_AVCODEC=(sizeof AVCODEC)/(sizeof AVCODEC[0]),
  };

  if (!avcodec.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_AVCODEC)) {
      PBU_DMESSAGE("loading avcodec");
      return -1;
    }

#if defined (_WIN32) // [
    wcscpy(pp,AVCODEC);
    avcodec.hLib=LoadLibraryW(path);
#else // ] [
    strcpy(pp,AVCODEC);
    avcodec.hLib=dlopen(path,DLOPEN_FLAG);
#endif // ]

#if defined (_WIN32) // [
    if (!avcodec.hLib&&!(avcodec.hLib=LoadLibraryW(AVCODEC))) {
      PBU_DMESSAGE("loading avcodec");
#if defined (PBU_DEBUG) // [
      fwprintf(stderr,L"%d: \"%s\"\n",__LINE__,path);
      fflush(stderr);
#endif // ]
      return -1;
    }
#else // ] [
    if (!avcodec.hLib&&!(avcodec.hLib=dlopen(AVCODEC,DLOPEN_FLAG))) {
      PBU_DMESSAGE("loading avcodec");
#if defined (PBU_DEBUG) // [
      fprintf(stderr,"%d: \"%s\"\n",__LINE__,path);
      fprintf(stderr,"%d: \"%s\"\n",__LINE__,AVCODEC);
      fprintf(stderr,"%d: dlopen failed: %s.\n",__LINE__,dlerror());
      fflush(stderr);
#endif // ]
      return -1;
    }
#endif // ]
  }

  return load(avcodec.hLib,sym,p);
}

static int avformat_load(void *p, const char *sym)
{
#if defined (_WIN32) // [
  static const wchar_t AVFORMAT[]=PBU_WIDEN(FFSOX_AVFORMAT);
#else // ] [
  static const char AVFORMAT[]=FFSOX_AVFORMAT;
#endif // ]

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_AVFORMAT=(sizeof AVFORMAT)/(sizeof AVFORMAT[0]),
  };

  if (!avformat.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_AVFORMAT)) {
      PBU_DMESSAGE("loading avformat");
      return -1;
    }

#if defined (_WIN32) // [
    wcscpy(pp,AVFORMAT);
    avformat.hLib=LoadLibraryW(path);
#else // ] [
    strcpy(pp,AVFORMAT);
    avformat.hLib=dlopen(path,DLOPEN_FLAG);
#endif // ]

#if defined (_WIN32) // [
    if (!avformat.hLib&&!(avformat.hLib=LoadLibraryW(AVFORMAT))) {
      PBU_DMESSAGE("loading avformat");
#if defined (PBU_DEBUG) // [
      fwprintf(stderr,L"%d: \"%s\"\n",__LINE__,path);
      fflush(stderr);
#endif // ]
      return -1;
    }
#else // ] [
    if (!avformat.hLib&&!(avformat.hLib=dlopen(AVFORMAT,DLOPEN_FLAG))) {
      PBU_DMESSAGE("loading avformat");
#if defined (PBU_DEBUG) // [
      fprintf(stderr,"%d: \"%s\"\n",__LINE__,path);
      fflush(stderr);
#endif // ]
      return -1;
    }
#endif // ]
  }

  return load(avformat.hLib,sym,p);
}

static int libsox_load(void *p, const char *sym)
{
#if defined (_WIN32) // [
  static const wchar_t LIBSOX[]=PBU_WIDEN(FFSOX_LIBSOX);
#else // ] [
  static const char LIBSOX[]=FFSOX_LIBSOX;
#endif // ]

  enum {
    SIZE_PATH=(sizeof path)/(sizeof path[0]),
    SIZE_LIBSOX=(sizeof LIBSOX)/(sizeof LIBSOX[0]),
  };

  if (!libsox.hLib) {
    if ((path+SIZE_PATH)<=(pp+SIZE_LIBSOX)) {
      PBU_DMESSAGE("loading libsox");
      return -1;
    }

#if defined (_WIN32) // [
    wcscpy(pp,LIBSOX);
    libsox.hLib=LoadLibraryW(path);
#else // ] [
    strcpy(pp,LIBSOX);
    libsox.hLib=dlopen(path,DLOPEN_FLAG);
#endif // ]

#if defined (_WIN32) // [
    if (!libsox.hLib&&!(libsox.hLib=LoadLibraryW(LIBSOX))) {
      PBU_DMESSAGE("loading libsox");
#if defined (PBU_DEBUG) // [
      fwprintf(stderr,L"%d: \"%s\"\n",__LINE__,path);
      fflush(stderr);
#endif // ]
      return -1;
    }
#else // ] [
    if (!libsox.hLib&&!(libsox.hLib=dlopen(LIBSOX,DLOPEN_FLAG))) {
      PBU_DMESSAGE("loading libsox");
#if defined (PBU_DEBUG) // [
      fprintf(stderr,"%d: \"%s\"\n",__LINE__,path);
      fflush(stderr);
#endif // ]
      return -1;
    }
#endif // ]
  }

  return load(libsox.hLib,sym,p);
}

static int ffsox_dynload_absolute(const char *dirname)
{
  enum { SIZE=(sizeof path)/(sizeof path[0]) };
  int code=-1,size;
#if defined (_WIN32) // [
  wchar_t *mp=path+SIZE;

  /////////////////////////////////////////////////////////////////////////////
  size=MultiByteToWideChar(
    CP_ACP,     // UINT                              CodePage,
    0ul,        // DWORD                             dwFlags,
    dirname,    // _In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
    -1,         // int                               cbMultiByte,
    pp,         // LPWSTR                            lpWideCharStr,
    mp-pp       // int                               cchWideChar
  );

  if (mp<=pp+size+1)
    goto exit;

  pp+=size-1;
  *pp++=L'\\';
  *pp=L'\0';
#else // ] [
  char *mp=path+SIZE;

	size=strlen(dirname)+2;

  if (mp<=pp+size+1)
    goto exit;

	strcpy(path,dirname);
  pp+=size-1;
  *pp++='/';
  *pp='\0';
#endif // ]
  /////////////////////////////////////////////////////////////////////////////
  code=0;
exit:
  return code;
}

static int ffsox_dynload_relative(const char *dirname)
{
  enum { SIZE=(sizeof path)/(sizeof path[0]) };
  int code=-1;
#if defined (_WIN32) // [
  wchar_t *mp=path+SIZE;
#else // ] [
  char process_path[64];
  char *mp=path+SIZE;
#endif // ]
  int len,size;

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
  len=GetModuleFileNameW(NULL,pp,mp-pp);

	if (ERROR_INSUFFICIENT_BUFFER==GetLastError())
    goto exit;
#else // ] [
  sprintf(process_path,"/proc/%d/exe",getpid());

  if ((len=readlink(process_path,path,SIZE-1))<0)
    goto exit;

	//len=strlen(pp);
  pp=path+len;
  *pp='\0';
#endif // ]

  if (mp<=pp+len+1)
    goto exit;

  pp+=len;

  /////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
  while (path<pp&&L'\\'!=pp[-1]&&L'/'!=pp[-1])
    --pp;

  size=MultiByteToWideChar(
    CP_ACP,     // UINT                              CodePage,
    0ul,        // DWORD                             dwFlags,
    dirname,    // _In_NLS_string_(cbMultiByte)LPCCH lpMultiByteStr,
    -1,         // int                               cbMultiByte,
    pp,         // LPWSTR                            lpWideCharStr,
    mp-pp       // int                               cchWideChar
  );
#else // ] [
  while (path<pp&&'/'!=pp[-1])
    --pp;

	size=strlen(dirname)+1;
#endif // ]

  if (mp<=pp+size+1)
    goto exit;

#if ! defined (_WIN32) // [
	strcpy(pp,dirname);
#endif // ]

  pp+=size-1;
#if defined (_WIN32) // [
  *pp++=L'\\';
  *pp=L'\0';
#else // ] [
  *pp++='/';
  *pp='\0';
#endif // ]

  /////////////////////////////////////////////////////////////////////////////
  code=0;
exit:
  return code;
}

int ffsox_dynload(const char *dirname)
{
  int code=-1;

  if (NULL==dirname||'/'==dirname[0])
    code=ffsox_dynload_absolute(dirname);
  else if ('\\'==dirname[0]||(dirname[0]!=0&&dirname[1]==':'))
    code=ffsox_dynload_absolute(dirname);
  else
    code=ffsox_dynload_relative(dirname);

  return code;
}

void ffsox_unload(void)
{
  if (libsox.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(libsox.hLib);
#else // ] [
    dlclose(libsox.hLib);
#endif // ]
	}

  if (avformat.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(avformat.hLib);
#else // ] [
    dlclose(avformat.hLib);
#endif // ]
	}

  if (avcodec.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(avcodec.hLib);
#else // ] [
    dlclose(avcodec.hLib);
#endif // ]
	}

  if (avutil.hLib) {
#if defined (_WIN32) // [
    FreeLibrary(avutil.hLib);
#else // ] [
    dlclose(avutil.hLib);
#endif // ]
	}
}
#endif // ]
