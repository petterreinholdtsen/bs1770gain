/*
 * bs1770gain_dynload.h
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
#ifndef __BS1770GAIN_DYNLOAD_H__
#define __BS1770GAIN_DYNLOAD_H__ // {
#include <libavformat/avformat.h>
#include <sox.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
typedef struct bs1770gain_avutil bs1770gain_avutil_t;
typedef struct bs1770gain_avcodec bs1770gain_avcodec_t;
typedef struct bs1770gain_avformat bs1770gain_avformat_t;

typedef struct bs1770gain_libsox bs1770gain_libsox_t;

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_avutil {
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
  typeof (av_frame_get_channels) *av_frame_get_channels;
  typeof (av_frame_set_sample_rate) *av_frame_set_sample_rate;
  typeof (av_frame_get_sample_rate) *av_frame_get_sample_rate;
  typeof (av_samples_alloc) *av_samples_alloc;
  typeof (av_free) *av_free;
  typeof (av_freep) *av_freep;
  typeof (av_dict_get) *av_dict_get;
  typeof (av_dict_set) *av_dict_set;
  typeof (av_dict_free) *av_dict_free;
};

struct bs1770gain_avcodec {
  typeof (avcodec_find_decoder) *avcodec_find_decoder;
  typeof (avcodec_find_decoder_by_name) *avcodec_find_decoder_by_name;
  typeof (avcodec_find_encoder) *avcodec_find_encoder;
  typeof (avcodec_open2) *avcodec_open2;
  typeof (av_init_packet) *av_init_packet;
  typeof (avcodec_decode_audio4) *avcodec_decode_audio4;
  typeof (avcodec_encode_audio2) *avcodec_encode_audio2;
  typeof (avcodec_decode_video2) *avcodec_decode_video2;
  typeof (av_free_packet) *av_free_packet;
  typeof (avcodec_close) *avcodec_close;
  typeof (avcodec_copy_context) *avcodec_copy_context;
  typeof (av_packet_rescale_ts) *av_packet_rescale_ts;
};

struct bs1770gain_avformat {
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
};

extern bs1770gain_avutil_t bs1770gain_avutil;
extern bs1770gain_avcodec_t bs1770gain_avcodec;
extern bs1770gain_avformat_t bs1770gain_avformat;

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_libsox {
  typeof (sox_init) *sox_init;
  typeof (sox_quit) *sox_quit;
  typeof (sox_create_effects_chain) *sox_create_effects_chain;
  typeof (sox_delete_effects_chain) *sox_delete_effects_chain;
  typeof (sox_find_effect) *sox_find_effect;
  typeof (sox_create_effect) *sox_create_effect;
  typeof (sox_effect_options) *sox_effect_options;
  typeof (sox_add_effect) *sox_add_effect;
  typeof (sox_flow_effects) *sox_flow_effects;
};

extern bs1770gain_libsox_t bs1770gain_libsox;

///////////////////////////////////////////////////////////////////////////////
int bs1770gain_dynload(const char *dirname);

#if ! defined (BS1770_DYNLOAD) // {
///////////////////////////////////////////////////////////////////////////////
#define av_frame_alloc (*bs1770gain_avutil.av_frame_alloc)
#define av_frame_free (*bs1770gain_avutil.av_frame_free)
#define av_frame_get_best_effort_timestamp \
    (*bs1770gain_avutil.av_frame_get_best_effort_timestamp)
#define av_frame_set_best_effort_timestamp \
    (*bs1770gain_avutil.av_frame_set_best_effort_timestamp)
#define av_log_get_level (*bs1770gain_avutil.av_log_get_level)
#define av_log_set_level (*bs1770gain_avutil.av_log_set_level)
#define av_get_sample_fmt_name (*bs1770gain_avutil.av_get_sample_fmt_name)
#define av_log (*bs1770gain_avutil.av_log)
#define av_rescale_q_rnd (*bs1770gain_avutil.av_rescale_q_rnd)
#define av_rescale_q (*bs1770gain_avutil.av_rescale_q)
#define av_frame_set_channel_layout \
    (*bs1770gain_avutil.av_frame_set_channel_layout)
#define av_frame_get_channel_layout \
    (*bs1770gain_avutil.av_frame_get_channel_layout)
#define av_frame_set_channels (*bs1770gain_avutil.av_frame_set_channels)
#define av_frame_get_channels (*bs1770gain_avutil.av_frame_get_channels)
#define av_frame_set_sample_rate (*bs1770gain_avutil.av_frame_set_sample_rate)
#define av_frame_get_sample_rate (*bs1770gain_avutil.av_frame_get_sample_rate)
#define av_samples_alloc (*bs1770gain_avutil.av_samples_alloc)
#define av_free (*bs1770gain_avutil.av_free)
#define av_freep (*bs1770gain_avutil.av_freep)
#define av_dict_get (*bs1770gain_avutil.av_dict_get)
#define av_dict_set (*bs1770gain_avutil.av_dict_set)
#define av_dict_free (*bs1770gain_avutil.av_dict_free)

#define avcodec_find_decoder (*bs1770gain_avcodec.avcodec_find_decoder)
#define avcodec_find_decoder_by_name \
    (*bs1770gain_avcodec.avcodec_find_decoder_by_name)
#define avcodec_find_encoder (*bs1770gain_avcodec.avcodec_find_encoder)
#define avcodec_open2 (*bs1770gain_avcodec.avcodec_open2)
#define av_init_packet (*bs1770gain_avcodec.av_init_packet)
#define avcodec_decode_audio4 (*bs1770gain_avcodec.avcodec_decode_audio4)
#define avcodec_encode_audio2 (*bs1770gain_avcodec.avcodec_encode_audio2)
#define avcodec_decode_video2 (*bs1770gain_avcodec.avcodec_decode_video2)
#define av_free_packet (*bs1770gain_avcodec.av_free_packet)
#define avcodec_close (*bs1770gain_avcodec.avcodec_close)
#define avcodec_copy_context (*bs1770gain_avcodec.avcodec_copy_context)
#define av_packet_rescale_ts (*bs1770gain_avcodec.av_packet_rescale_ts)

#define av_register_all (*bs1770gain_avformat.av_register_all)
#define avformat_open_input (*bs1770gain_avformat.avformat_open_input)
#define avformat_find_stream_info \
    (*bs1770gain_avformat.avformat_find_stream_info)
#define av_read_frame (*bs1770gain_avformat.av_read_frame)
#define avformat_close_input (*bs1770gain_avformat.avformat_close_input)
#define avformat_alloc_output_context2 \
  (*bs1770gain_avformat.avformat_alloc_output_context2)
#define avformat_free_context (*bs1770gain_avformat.avformat_free_context)
#define avformat_new_stream (*bs1770gain_avformat.avformat_new_stream)
#define avio_open (*bs1770gain_avformat.avio_open)
#define avio_close (*bs1770gain_avformat.avio_close)
#define avformat_write_header (*bs1770gain_avformat.avformat_write_header)
#define av_interleaved_write_frame \
    (*bs1770gain_avformat.av_interleaved_write_frame)
#define av_write_trailer (*bs1770gain_avformat.av_write_trailer)
#define av_find_default_stream_index \
    (*bs1770gain_avformat.av_find_default_stream_index)
#define avformat_seek_file (*bs1770gain_avformat.avformat_seek_file)
#define av_dump_format (*bs1770gain_avformat.av_dump_format)

///////////////////////////////////////////////////////////////////////////////
#define sox_init (*bs1770gain_libsox.sox_init)
#define sox_quit (*bs1770gain_libsox.sox_quit)
#define sox_create_effects_chain (*bs1770gain_libsox.sox_create_effects_chain)
#define sox_delete_effects_chain (*bs1770gain_libsox.sox_delete_effects_chain)
#define sox_find_effect (*bs1770gain_libsox.sox_find_effect)
#define sox_create_effect (*bs1770gain_libsox.sox_create_effect)
#define sox_effect_options (*bs1770gain_libsox.sox_effect_options)
#define sox_add_effect (*bs1770gain_libsox.sox_add_effect)
#define sox_flow_effects (*bs1770gain_libsox.sox_flow_effects)
#endif // }

#ifdef __cpluplus
}
#endif
#endif // }
