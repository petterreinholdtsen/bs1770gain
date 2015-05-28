/*
 * bs1770gain.h
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
#ifndef __BS1770GAIN_PRIV_H__
#define __BS1770GAIN_PRIV_H__ // {
#include <ffsox_priv.h>
#include <bs1770gain.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
typedef bs1770gain_block_options_t block_options_t;
typedef bs1770gain_options_t options_t;
typedef bs1770gain_tag_t tag_t;
typedef bs1770gain_tree_vmt_t tree_vmt_t;
typedef bs1770gain_tree_t tree_t;
typedef bs1770gain_head_t head_t;
typedef bs1770gain_stats_t stats_t;
typedef bs1770gain_album_t album_t;
typedef bs1770gain_track_t track_t;
//typedef bs1770gain_read_t read_t;

#ifdef __cpluplus
}
#endif
#endif // }
