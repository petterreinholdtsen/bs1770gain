/*
 * ffsox_dynload.h
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
#ifndef __FFSOX_DYNLOAD_H__
#define __FFSOX_DYNLOAD_H__ // [
#if defined (HAVE_CONFIG_H) // [
#include <config.h>
#endif // ]
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#if defined (AV_CODEC_FLAG_GLOBAL_HEADER) // [
// just fine!
#elif defined (CODEC_FLAG_GLOBAL_HEADER) // ] [
// define it!
#define AV_CODEC_FLAG_GLOBAL_HEADER CODEC_FLAG_GLOBAL_HEADER
#else // ] [
// one out of the two should be defined.
#error AV_CODEC_FLAG_GLOBAL_HEADER
#endif // ]
#if defined (_MSC_VER) && defined (AV_TIME_BASE_Q) // [
  #undef AV_TIME_BASE_Q
#endif // ]
#include <sox.h>
#if defined (__GNUC__) && defined (__LP64__) && defined (LSX_API) // [
  #undef LSX_API
  #define LSX_API
#endif // ]
#ifdef __cpluplus // [
extern "C" {
#endif // ]

#if defined (HAVE_FFSOX_DYNLOAD) // [
///////////////////////////////////////////////////////////////////////////////
//#define FFSOX_DYNLOAD
#if ! defined (FFSOX_DYNLOAD) // [
#define FFSOX_DYNLOAD2
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#if ! defined (FFSOX_AVUTIL_V) // [
  #define FFSOX_AVUTIL_V PBU_STR(LIBAVUTIL_VERSION_MAJOR)
#endif // ]
#if ! defined (FFSOX_SWRESAMPLE_V) // [
  #define FFSOX_SWRESAMPLE_V PBU_STR(LIBSWRESAMPLE_VERSION_MAJOR)
#endif // ]
#if ! defined (FFSOX_AVCODEC_V) // [
  #define FFSOX_AVCODEC_V PBU_STR(LIBAVCODEC_VERSION_MAJOR)
#endif // ]
#if ! defined (FFSOX_AVFORMAT_V) // [
  #define FFSOX_AVFORMAT_V PBU_STR(LIBAVFORMAT_VERSION_MAJOR)
#endif // ]
#if ! defined (FFSOX_LIBSOX_V) // [
  #define FFSOX_LIBSOX_V "3"
#endif // ]

///////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32) // [
#define FFSOX_AVUTIL "avutil-" FFSOX_AVUTIL_V ".dll"
#define FFSOX_SWRESAMPLE "swresample-" FFSOX_SWRESAMPLE_V  ".dll"
#define FFSOX_AVCODEC "avcodec-" FFSOX_AVCODEC_V  ".dll"
#define FFSOX_AVFORMAT "avformat-" FFSOX_AVFORMAT_V  ".dll"
#define FFSOX_LIBSOX "libsox-" FFSOX_LIBSOX_V  ".dll"
#else // ] [
#define FFSOX_AVUTIL "libavutil.so." FFSOX_AVUTIL_V
#define FFSOX_SWRESAMPLE "libswresample.so." FFSOX_SWRESAMPLE_V
#define FFSOX_AVCODEC "libavcodec.so." FFSOX_AVCODEC_V
#define FFSOX_AVFORMAT "libavformat.so." FFSOX_AVFORMAT_V
#define FFSOX_LIBSOX "libsox.so." FFSOX_LIBSOX_V
#endif // ]

///////////////////////////////////////////////////////////////////////////////
int ffsox_dynload(const char *dirname);
void ffsox_unload(void);
#endif // ]

///////////////////////////////////////////////////////////////////////////////
//#define FFSOX_FILTER_CHANNELS

#if 0 // [
#define FFSOX_DEPRECATED_AV_FREE_PACKET
#endif // ]

#if defined (FFSOX_DYNLOAD2) // [
#elif defined (FFSOX_DYNLOAD) // ] [
///////////////////////////////////////////////////////////////////////////////
typedef struct ffsox_avutil ffsox_avutil_t;
typedef struct ffsox_avcodec ffsox_avcodec_t;
typedef struct ffsox_avformat ffsox_avformat_t;

typedef struct ffsox_libsox ffsox_libsox_t;

///////////////////////////////////////////////////////////////////////////////
struct ffsox_avutil {
#if defined (__GNUC__) // {
  typeof (avutil_version) *avutil_version;
  typeof (av_frame_alloc) *av_frame_alloc;
  typeof (av_frame_free) *av_frame_free;
  typeof (av_get_channel_layout_nb_channels)
      *av_get_channel_layout_nb_channels;
  typeof (av_frame_get_best_effort_timestamp)
      *av_frame_get_best_effort_timestamp;
  typeof (av_frame_set_best_effort_timestamp)
      *av_frame_set_best_effort_timestamp;
  typeof (av_log_get_level) *av_log_get_level;
  typeof (av_log_set_level) *av_log_set_level;
  typeof (av_get_sample_fmt_name) *av_get_sample_fmt_name;
  typeof (av_log) *av_log;
  typeof (av_rescale_q_rnd) *av_rescale_q_rnd;
  typeof (av_rescale_q) *av_rescale_q;
  typeof (av_frame_set_channel_layout) *av_frame_set_channel_layout;
  typeof (av_frame_get_channel_layout) *av_frame_get_channel_layout;
  typeof (av_frame_set_channels) *av_frame_set_channels;
#if defined (FFSOX_AV_FRAME_GET_CHANNELS) // [
  typeof (av_frame_get_channels) *av_frame_get_channels;
#endif // ]
  typeof (av_frame_set_sample_rate) *av_frame_set_sample_rate;
  typeof (av_frame_get_sample_rate) *av_frame_get_sample_rate;
  typeof (av_samples_alloc) *av_samples_alloc;
  typeof (av_free) *av_free;
  typeof (av_freep) *av_freep;
  typeof (av_dict_get) *av_dict_get;
  typeof (av_dict_set) *av_dict_set;
  typeof (av_dict_free) *av_dict_free;
  typeof (av_frame_get_buffer) *av_frame_get_buffer;
#if defined (FFSOX_FILTER_CHANNELS) // [
  typeof (av_get_channel_layout_channel_index)
      *av_get_channel_layout_channel_index;
#endif // ]
#else // } {
  void *avutil_version;
  AVFrame *(*av_frame_alloc)(void);
  void (*av_frame_free)(AVFrame **frame);
  int (*av_get_channel_layout_nb_channels)(uint64_t channel_layout);
  int64_t (*av_frame_get_best_effort_timestamp)(const AVFrame *frame);
  void (*av_frame_set_best_effort_timestamp)(AVFrame *frame, int64_t val);
  int (*av_log_get_level)(void);
  void (*av_log_set_level)(int level);
  const char *(*av_get_sample_fmt_name)(enum AVSampleFormat sample_fmt);
  void (*av_log)(void *avcl, int level, const char *fmt, ...)
      av_printf_format(3, 4);
  int64_t (*av_rescale_q_rnd)(int64_t a, AVRational bq, AVRational cq,
      enum AVRounding) av_const;
  int64_t (*av_rescale_q)(int64_t a, AVRational bq, AVRational cq) av_const;
  void (*av_frame_set_channel_layout)(AVFrame *frame, int64_t val);
  int64_t (*av_frame_get_channel_layout)(const AVFrame *frame);
  void (*av_frame_set_channels)(AVFrame *frame, int val);
#if defined (FFSOX_AV_FRAME_GET_CHANNELS) // [
  int (*av_frame_get_channels)(const AVFrame *frame);
#endif // ]
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
#if defined (FFSOX_FILTER_CHANNELS) // [
  int (*av_get_channel_layout_channel_index)(uint64_t channel_layout,
      int index);
#endif // ]
#endif // }
};

struct ffsox_avcodec {
#if defined (__GNUC__) // {
  typeof (avcodec_version) *avcodec_version;
  typeof (avcodec_find_decoder) *avcodec_find_decoder;
  typeof (avcodec_find_decoder_by_name) *avcodec_find_decoder_by_name;
  typeof (avcodec_find_encoder) *avcodec_find_encoder;
  typeof (avcodec_open2) *avcodec_open2;
  typeof (av_init_packet) *av_init_packet;
  typeof (avcodec_decode_audio4) *avcodec_decode_audio4;
  typeof (avcodec_encode_audio2) *avcodec_encode_audio2;
  typeof (avcodec_decode_video2) *avcodec_decode_video2;
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // {
  typeof (av_free_packet) *av_free_packet;
#else // } {
  typeof (av_packet_unref) *av_packet_unref;
#endif // }
  typeof (avcodec_close) *avcodec_close;
  typeof (avcodec_copy_context) *avcodec_copy_context;
  typeof (av_packet_rescale_ts) *av_packet_rescale_ts;
#else // } {
  void *avcodec_version;
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
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // {
  attribute_deprecated
  void (*av_free_packet)(AVPacket *pkt);
#else // } {
  void (*av_packet_unref)(AVPacket *pkt);
#endif // }
  int (*avcodec_close)(AVCodecContext *avctx);
  int (*avcodec_copy_context)(AVCodecContext *dest, const AVCodecContext *src);
  void (*av_packet_rescale_ts)(AVPacket *pkt, AVRational tb_src,
      AVRational tb_dst);
#endif // }
};

struct ffsox_avformat {
#if defined (__GNUC__) // {
  typeof (avformat_version) *avformat_version;
  typeof (av_register_all) *av_register_all;
  typeof (avformat_open_input) *avformat_open_input;
  typeof (avformat_find_stream_info) *avformat_find_stream_info;
  typeof (av_read_frame) *av_read_frame;
  typeof (avformat_close_input) *avformat_close_input;
  typeof (avformat_alloc_output_context2) *avformat_alloc_output_context2;
  typeof (avformat_free_context) *avformat_free_context;
  typeof (avformat_new_stream) *avformat_new_stream;
  typeof (avio_open) *avio_open;
  typeof (avio_close) *avio_close;
  typeof (avformat_write_header) *avformat_write_header;
  typeof (av_interleaved_write_frame) *av_interleaved_write_frame;
  typeof (av_write_trailer) *av_write_trailer;
  typeof (av_find_default_stream_index) *av_find_default_stream_index;
  typeof (avformat_seek_file) *avformat_seek_file;
  typeof (av_dump_format) *av_dump_format;
#else // } {
  void *avformat_version;
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
#endif // }
};

extern ffsox_avutil_t ffsox_avutil;
extern ffsox_avcodec_t ffsox_avcodec;
extern ffsox_avformat_t ffsox_avformat;

///////////////////////////////////////////////////////////////////////////////
struct ffsox_libsox {
#if defined (__GNUC__) // {
  typeof (sox_init) *sox_init;
  typeof (sox_quit) *sox_quit;
  typeof (sox_create_effects_chain) *sox_create_effects_chain;
  typeof (sox_delete_effects_chain) *sox_delete_effects_chain;
  typeof (sox_find_effect) *sox_find_effect;
  typeof (sox_create_effect) *sox_create_effect;
  typeof (sox_effect_options) *sox_effect_options;
  typeof (sox_add_effect) *sox_add_effect;
  typeof (sox_flow_effects) *sox_flow_effects;
  typeof (sox_open_read) *sox_open_read;
  typeof (sox_read) *sox_read;
  typeof (sox_open_write) *sox_open_write;
  typeof (sox_write) *sox_write;
  typeof (sox_close) *sox_close;
  typeof (sox_init_encodinginfo) *sox_init_encodinginfo;
#else // } {
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
#endif // }
};

extern ffsox_libsox_t ffsox_libsox;

#if ! defined (FFSOX_DYNLOAD_PRIV) // {
///////////////////////////////////////////////////////////////////////////////
#define avutil_version (*ffsox_avutil.avutil_version)
#define av_frame_alloc (*ffsox_avutil.av_frame_alloc)
#define av_frame_free (*ffsox_avutil.av_frame_free)
#define av_get_channel_layout_nb_channels \
    (*ffsox_avutil.av_get_channel_layout_nb_channels)
#define av_frame_get_best_effort_timestamp \
    (*ffsox_avutil.av_frame_get_best_effort_timestamp)
#define av_frame_set_best_effort_timestamp \
    (*ffsox_avutil.av_frame_set_best_effort_timestamp)
#define av_log_get_level (*ffsox_avutil.av_log_get_level)
#define av_log_set_level (*ffsox_avutil.av_log_set_level)
#define av_get_sample_fmt_name (*ffsox_avutil.av_get_sample_fmt_name)
#define av_log (*ffsox_avutil.av_log)
#define av_rescale_q_rnd (*ffsox_avutil.av_rescale_q_rnd)
#define av_rescale_q (*ffsox_avutil.av_rescale_q)
#define av_frame_set_channel_layout \
    (*ffsox_avutil.av_frame_set_channel_layout)
#define av_frame_get_channel_layout \
    (*ffsox_avutil.av_frame_get_channel_layout)
#define av_frame_set_channels (*ffsox_avutil.av_frame_set_channels)
#if defined (FFSOX_AV_FRAME_GET_CHANNELS) // [
#define av_frame_get_channels (*ffsox_avutil.av_frame_get_channels)
#endif // ]
#define av_frame_set_sample_rate (*ffsox_avutil.av_frame_set_sample_rate)
#define av_frame_get_sample_rate (*ffsox_avutil.av_frame_get_sample_rate)
#define av_samples_alloc (*ffsox_avutil.av_samples_alloc)
#define av_free (*ffsox_avutil.av_free)
#define av_freep (*ffsox_avutil.av_freep)
#define av_dict_get (*ffsox_avutil.av_dict_get)
#define av_dict_set (*ffsox_avutil.av_dict_set)
#define av_dict_free (*ffsox_avutil.av_dict_free)
#define av_frame_get_buffer (*ffsox_avutil.av_frame_get_buffer)
#if defined (FFSOX_FILTER_CHANNELS) // [
#define av_get_channel_layout_channel_index \
    (*ffsox_avutil.av_get_channel_layout_channel_index)
#endif // ]

#if ! defined (PBU_MALLOC_DEBUG) // {
#define avcodec_version (*ffsox_avcodec.avcodec_version)
#define avcodec_find_decoder (*ffsox_avcodec.avcodec_find_decoder)
#define avcodec_find_decoder_by_name \
    (*ffsox_avcodec.avcodec_find_decoder_by_name)
#define avcodec_find_encoder (*ffsox_avcodec.avcodec_find_encoder)
#define avcodec_open2 (*ffsox_avcodec.avcodec_open2)
#define av_init_packet (*ffsox_avcodec.av_init_packet)
#define avcodec_decode_audio4 (*ffsox_avcodec.avcodec_decode_audio4)
#define avcodec_encode_audio2 (*ffsox_avcodec.avcodec_encode_audio2)
#define avcodec_decode_video2 (*ffsox_avcodec.avcodec_decode_video2)
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // {
#define av_free_packet (*ffsox_avcodec.av_free_packet)
#else // } {
#define av_packet_unref (*ffsox_avcodec.av_packet_unref)
#endif // }
#define avcodec_close (*ffsox_avcodec.avcodec_close)
#define avcodec_copy_context (*ffsox_avcodec.avcodec_copy_context)
#define av_packet_rescale_ts (*ffsox_avcodec.av_packet_rescale_ts)
#else // } {
AVCodec *ffsox_avcodec_find_decoder(enum AVCodecID id);
AVCodec *ffsox_avcodec_find_decoder_by_name(const char *name);
AVCodec *ffsox_avcodec_find_encoder(enum AVCodecID id);
int ffsox_avcodec_open2(AVCodecContext *avctx, const AVCodec *codec,
    AVDictionary **options);
void ffsox_av_init_packet(AVPacket *pkt);
int ffsox_avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
    int *got_frame_ptr, const AVPacket *avpkt);
int ffsox_avcodec_encode_audio2(AVCodecContext *avctx, AVPacket *avpkt,
    const AVFrame *frame, int *got_packet_ptr);
int ffsox_avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture,
    int *got_picture_ptr, const AVPacket *avpkt);
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // {
attribute_deprecated
void ffsox_av_free_packet(AVPacket *pkt);
#else // } {
void ffsox_av_packet_unref(AVPacket *pkt);
#endif // }
int ffsox_avcodec_close(AVCodecContext *avctx);
int ffsox_avcodec_copy_context(AVCodecContext *dest, const AVCodecContext *src);
void ffsox_av_packet_rescale_ts(AVPacket *pkt, AVRational tb_src,
    AVRational tb_dst);

#define avcodec_find_decoder ffsox_avcodec_find_decoder
#define avcodec_find_decoder_by_name \
    ffsox_avcodec_find_decoder_by_name
#define avcodec_find_encoder ffsox_avcodec_find_encoder
#define avcodec_open2 ffsox_avcodec_open2
#define av_init_packet ffsox_av_init_packet
#define avcodec_decode_audio4 ffsox_avcodec_decode_audio4
#define avcodec_encode_audio2 ffsox_avcodec_encode_audio2
#define avcodec_decode_video2 ffsox_avcodec_decode_video2
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // {
#define av_free_packet ffsox_av_free_packet
#else // } {
#define av_packet_unref ffsox_av_packet_unref
#endif // }
#define avcodec_close ffsox_avcodec_close
#define avcodec_copy_context ffsox_avcodec_copy_context
#define av_packet_rescale_ts ffsox_av_packet_rescale_ts
#endif // }

#if ! defined (PBU_MALLOC_DEBUG) // {
#define avformat_version (*ffsox_avformat.avformat_version)
#define av_register_all (*ffsox_avformat.av_register_all)
#define avformat_open_input (*ffsox_avformat.avformat_open_input)
#define avformat_find_stream_info \
    (*ffsox_avformat.avformat_find_stream_info)
#define av_read_frame (*ffsox_avformat.av_read_frame)
#define avformat_close_input (*ffsox_avformat.avformat_close_input)
#define avformat_alloc_output_context2 \
    (*ffsox_avformat.avformat_alloc_output_context2)
#define avformat_free_context (*ffsox_avformat.avformat_free_context)
#define avformat_new_stream (*ffsox_avformat.avformat_new_stream)
#define avio_open (*ffsox_avformat.avio_open)
#define avio_close (*ffsox_avformat.avio_close)
#define avformat_write_header (*ffsox_avformat.avformat_write_header)
#define av_interleaved_write_frame \
    (*ffsox_avformat.av_interleaved_write_frame)
#define av_write_trailer (*ffsox_avformat.av_write_trailer)
#define av_find_default_stream_index \
    (*ffsox_avformat.av_find_default_stream_index)
#define avformat_seek_file (*ffsox_avformat.avformat_seek_file)
#define av_dump_format (*ffsox_avformat.av_dump_format)
#else // } {
void ffsox_av_register_all(void);
int ffsox_avformat_open_input(AVFormatContext **ps, const char *filename,
    AVInputFormat *fmt, AVDictionary **options);
int ffsox_avformat_find_stream_info(AVFormatContext *ic,
    AVDictionary **options);
int ffsox_av_read_frame(AVFormatContext *s, AVPacket *pkt);
void ffsox_avformat_close_input(AVFormatContext **s);
int ffsox_avformat_alloc_output_context2(AVFormatContext **ctx,
    AVOutputFormat *oformat, const char *format_name,
    const char *filename);
void ffsox_avformat_free_context(AVFormatContext *s);
AVStream *ffsox_avformat_new_stream(AVFormatContext *s, const AVCodec *c);
int ffsox_avio_open(AVIOContext **s, const char *url, int flags);
int ffsox_avio_close(AVIOContext *s);
int ffsox_avformat_write_header(AVFormatContext *s, AVDictionary **options);
int ffsox_av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt);
int ffsox_av_write_trailer(AVFormatContext *s);
int ffsox_av_find_default_stream_index(AVFormatContext *s);
int ffsox_avformat_seek_file(AVFormatContext *s, int stream_index,
    int64_t min_ts, int64_t ts, int64_t max_ts, int flags);
void ffsox_av_dump_format(AVFormatContext *ic, int index, const char *url,
    int is_output);

#define av_register_all ffsox_av_register_all
#define avformat_open_input ffsox_avformat_open_input
#define avformat_find_stream_info \
    ffsox_avformat_find_stream_info
#define av_read_frame ffsox_av_read_frame
#define avformat_close_input ffsox_avformat_close_input
#define avformat_alloc_output_context2 \
    ffsox_avformat_alloc_output_context2
#define avformat_free_context ffsox_avformat_free_context
#define avformat_new_stream ffsox_avformat_new_stream
#define avio_open ffsox_avio_open
#define avio_close ffsox_avio_close
#define avformat_write_header ffsox_avformat_write_header
#define av_interleaved_write_frame \
    ffsox_av_interleaved_write_frame
#define av_write_trailer ffsox_av_write_trailer
#define av_find_default_stream_index \
    ffsox_av_find_default_stream_index
#define avformat_seek_file ffsox_avformat_seek_file
#define av_dump_format ffsox_av_dump_format
#endif // }

///////////////////////////////////////////////////////////////////////////////
#define sox_init (*ffsox_libsox.sox_init)
#define sox_quit (*ffsox_libsox.sox_quit)
#define sox_create_effects_chain (*ffsox_libsox.sox_create_effects_chain)
#define sox_delete_effects_chain (*ffsox_libsox.sox_delete_effects_chain)
#define sox_find_effect (*ffsox_libsox.sox_find_effect)
#define sox_create_effect (*ffsox_libsox.sox_create_effect)
#define sox_effect_options (*ffsox_libsox.sox_effect_options)
#define sox_add_effect (*ffsox_libsox.sox_add_effect)
#define sox_flow_effects (*ffsox_libsox.sox_flow_effects)
#define sox_open_read (*ffsox_libsox.sox_open_read)
#define sox_read (*ffsox_libsox.sox_read)
#define sox_open_write (*ffsox_libsox.sox_open_write)
#define sox_write (*ffsox_libsox.sox_write)
#define sox_close (*ffsox_libsox.sox_close)
#define sox_init_encodinginfo (*ffsox_libsox.sox_init_encodinginfo)
#endif // }
#endif // ]

#ifdef __cpluplus // [
}
#endif // ]
#endif // ]
