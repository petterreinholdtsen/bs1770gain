/*
 * bs1770gain.h
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
#ifndef __BS1770GAIN_PRIV_H__
#define __BS1770GAIN_PRIV_H__ // {
#include <bs1770gain.h>
#include <ffsox_priv.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define EXTENSION_RENAME      BS1770GAIN_EXTENSION_RENAME
#define EXTENSION_CSV         BS1770GAIN_EXTENSION_CSV
#define EXTENSION_JPG         BS1770GAIN_EXTENSION_JPG
#define EXTENSION_TAGS        BS1770GAIN_EXTENSION_TAGS
#define EXTENSION_ALL         BS1770GAIN_EXTENSION_ALL

///////////////////////////////////////////////////////////////////////////////
typedef bs1770gain_block_options_t block_options_t;
typedef bs1770gain_options_t options_t;
typedef bs1770gain_tag_t tag_t;
typedef bs1770gain_tree_vmt_t tree_vmt_t;
typedef bs1770gain_tree_t tree_t;
typedef bs1770gain_album_t album_t;
typedef bs1770gain_track_t track_t;

#ifdef __cpluplus
}
#endif
#endif // }
