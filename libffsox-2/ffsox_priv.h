/*
 * ffsox_priv.h
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
#ifndef __FFSOX_PRIV_H__
#define __FFSOX_PRIV_H__ // {
#include <pbutil_priv.h>
#include <ffsox.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined (WIN32) // {
#define HR_ERROR_CASE                 FFSOX_HR_ERROR_CASE
#define REFTIMES_PER_SEC              FFSOX_REFTIMES_PER_SEC
#define REFTIMES_PER_MILLISEC         FFSOX_REFTIMES_PER_MILLISEC
#endif // }

#define STATE_RUN                     FFSOX_STATE_RUN
#define STATE_FLUSH                   FFSOX_STATE_FLUSH
#define STATE_END                     FFSOX_STATE_END

#define MACHINE_PUSH                  FFSOX_MACHINE_PUSH
#define MACHINE_POP                   FFSOX_MACHINE_POP

#define AGGREGATE_MOMENTARY_MAXIMUM   FFSOX_AGGREGATE_MOMENTARY_MAXIMUM
#define AGGREGATE_MOMENTARY_MEAN      FFSOX_AGGREGATE_MOMENTARY_MEAN
#define AGGREGATE_MOMENTARY_RANGE     FFSOX_AGGREGATE_MOMENTARY_RANGE
#define AGGREGATE_SHORTTERM_MAXIMUM   FFSOX_AGGREGATE_SHORTTERM_MAXIMUM
#define AGGREGATE_SHORTTERM_MEAN      FFSOX_AGGREGATE_SHORTTERM_MEAN
#define AGGREGATE_SHORTTERM_RANGE     FFSOX_AGGREGATE_SHORTTERM_RANGE
#define AGGREGATE_SAMPLEPEAK          FFSOX_AGGREGATE_SAMPLEPEAK
#define AGGREGATE_TRUEPEAK            FFSOX_AGGREGATE_TRUEPEAK
#define AGGREGATE_MOMENTARY           FFSOX_AGGREGATE_MOMENTARY
#define AGGREGATE_SHORTTERM           FFSOX_AGGREGATE_SHORTTERM
#define AGGREGATE_ALL                 FFSOX_AGGREGATE_ALL

#define AUDIO_PLAYER_RENDER           FFSOX_AUDIO_PLAYER_RENDER
#define AUDIO_PLAYER_END              FFSOX_AUDIO_PLAYER_END

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_intercept_t intercept_t;
typedef ffsox_convert_t convert_t;
typedef ffsox_format_t format_t;
typedef ffsox_stream_t stream_t;
typedef ffsox_frame_t frame_t;
typedef ffsox_machine_t machine_t;
typedef ffsox_read_list_t read_list_t;
typedef ffsox_packet_consumer_list_t packet_consumer_list_t;
typedef ffsox_stream_list_t stream_list_t;
typedef ffsox_sink_t sink_t;

typedef ffsox_aggregate_t aggregate_t;
typedef ffsox_collect_t collect_t;

typedef ffsox_block_config_t collect_block_t;
typedef ffsox_collect_config_t collect_config_t;
typedef ffsox_analyze_config_t analyze_config_t;

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_node_vmt_t node_vmt_t;
  typedef ffsox_source_vmt_t source_vmt_t;
  typedef ffsox_packet_consumer_vmt_t packet_consumer_vmt_t;
    typedef ffsox_packet_writer_vmt_t packet_writer_vmt_t;
    typedef ffsox_frame_reader_vmt_t frame_reader_vmt_t;
  typedef ffsox_frame_consumer_vmt_t frame_consumer_vmt_t;
    typedef ffsox_frame_writer_vmt_t frame_writer_vmt_t;
#if defined (WIN32) // {
    typedef ffsox_audio_player_vmt_t audio_player_vmt_t;
#endif // }
    typedef ffsox_sox_reader_vmt_t sox_reader_vmt_t;

typedef ffsox_node_t node_t;
  typedef ffsox_source_t source_t;
  typedef ffsox_packet_consumer_t packet_consumer_t;
    typedef ffsox_packet_writer_t packet_writer_t;
    typedef ffsox_frame_reader_t frame_reader_t;
  typedef ffsox_frame_consumer_t frame_consumer_t;
    typedef ffsox_frame_writer_t frame_writer_t;
#if defined (WIN32) // {
    typedef ffsox_audio_player_t audio_player_t;
#endif // }
    typedef ffsox_sox_reader_t sox_reader_t;

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_pull_callback_t pull_cb_t;
typedef ffsox_source_callback_t source_cb_t;

#ifdef __cpluplus
}
#endif
#endif // }
