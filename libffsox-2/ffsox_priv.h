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
#include <ffsox.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define MESSAGE(m)          FFSOX_MESSAGE(m)

///////////////////////////////////////////////////////////////////////////////
#define LIST_NEXT(n,l)      FFSOX_LIST_NEXT(n,l)
#define LIST_FOREACH(n,l)   FFSOX_LIST_FOREACH(n,l)

///////////////////////////////////////////////////////////////////////////////
#define STATE_RUN           FFSOX_STATE_RUN
#define STATE_FLUSH         FFSOX_STATE_FLUSH
#define STATE_END           FFSOX_STATE_END

#define MACHINE_STAY        FFSOX_MACHINE_STAY
#define MACHINE_PUSH        FFSOX_MACHINE_PUSH
#define MACHINE_POP         FFSOX_MACHINE_POP

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_list_t list_t;
typedef ffsox_format_t format_t;
typedef ffsox_stream_t stream_t;
typedef ffsox_frame_t frame_t;
typedef ffsox_machine_t machine_t;
typedef ffsox_read_list_t read_list_t;
typedef ffsox_sink_t sink_t;

///////////////////////////////////////////////////////////////////////////////
typedef ffsox_node_vmt_t node_vmt_t;
  typedef ffsox_source_vmt_t source_vmt_t;
  typedef ffsox_read_vmt_t read_vmt_t;
    typedef ffsox_read_copy_vmt_t read_copy_vmt_t;
    typedef ffsox_read_decode_vmt_t read_decode_vmt_t;
  typedef ffsox_write_vmt_t write_vmt_t;
    typedef ffsox_write_copy_vmt_t write_copy_vmt_t;
    typedef ffsox_write_encode_vmt_t write_encode_vmt_t;
  typedef ffsox_filter_vmt_t filter_vmt_t;

typedef ffsox_node_t node_t;
  typedef ffsox_source_t source_t;
  typedef ffsox_read_t read_t;
    typedef ffsox_read_copy_t read_copy_t;
    typedef ffsox_read_decode_t read_decode_t;
  typedef ffsox_write_t write_t;
    typedef ffsox_write_copy_t write_copy_t;
    typedef ffsox_write_encode_t write_encode_t;
  typedef ffsox_filter_t filter_t;

#ifdef __cpluplus
}
#endif
#endif // }
