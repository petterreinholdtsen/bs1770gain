/*
 * ffsox_read.c
 * Copyright (C) 2014, 2015 Peter Belkner <pbelkner@users.sf.net>
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
#include <ffsox.h>

AVCodec *ffsox_find_decoder(enum AVCodecID id)
{
  AVCodec *p;

  switch (id) {
    case AV_CODEC_ID_MP1:
      p=avcodec_find_decoder_by_name("mp1float");
      break;
    case AV_CODEC_ID_MP2:
      p=avcodec_find_decoder_by_name("mp2float");
      break;
    case AV_CODEC_ID_MP3:
      p=avcodec_find_decoder_by_name("mp3float");
      break;
    default:
      p=NULL;
      break;
  }

  return NULL==p?avcodec_find_decoder(id):p;
}
