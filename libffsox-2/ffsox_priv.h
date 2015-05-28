/*
 * ffsox_priv.h
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
#ifndef __FFSOX_PRIV_H__
#define __FFSOX_PRIV_H__ // {
#include <pbutil_priv.h>
#include <ffsox.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define STATE_RUN           FFSOX_STATE_RUN
#define STATE_FLUSH         FFSOX_STATE_FLUSH
#define STATE_END           FFSOX_STATE_END

#define MACHINE_PUSH        FFSOX_MACHINE_PUSH
#define MACHINE_POP         FFSOX_MACHINE_POP

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_read_ptr_t read_ptr_t;
typedef ffsox_convert_t convert_t;
typedef ffsox_format_t format_t;
typedef ffsox_stream_t stream_t;
typedef ffsox_frame_t frame_t;
typedef ffsox_machine_t machine_t;
typedef ffsox_read_list_t read_list_t;
typedef ffsox_packet_consumer_list_t packet_consumer_list_t;
typedef ffsox_stream_list_t stream_list_t;
typedef ffsox_sink_t sink_t;

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_node_vmt_t node_vmt_t;
  typedef ffsox_source_vmt_t source_vmt_t;
  typedef ffsox_packet_consumer_vmt_t packet_consumer_vmt_t;
    typedef ffsox_packet_writer_vmt_t packet_writer_vmt_t;
    typedef ffsox_frame_reader_vmt_t frame_reader_vmt_t;
  typedef ffsox_frame_consumer_vmt_t frame_consumer_vmt_t;
    typedef ffsox_frame_writer_vmt_t frame_writer_vmt_t;
    typedef ffsox_sox_reader_vmt_t sox_reader_vmt_t;

typedef ffsox_node_t node_t;
  typedef ffsox_source_t source_t;
  typedef ffsox_packet_consumer_t packet_consumer_t;
    typedef ffsox_packet_writer_t packet_writer_t;
    typedef ffsox_frame_reader_t frame_reader_t;
  typedef ffsox_frame_consumer_t frame_consumer_t;
    typedef ffsox_frame_writer_t frame_writer_t;
    typedef ffsox_sox_reader_t sox_reader_t;

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_source_callback_t source_cb_t;

#ifdef __cpluplus
}
#endif
#endif // }
