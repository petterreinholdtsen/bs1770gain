/*
 * ffsox_audiostream.c
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
#include <ffsox.h>

static int ffsox_codec_blacklist(int codec_id)
{
  switch (codec_id) {
#if 0 // {
  case AV_CODEC_ID_MJPEG:
  case AV_CODEC_ID_MJPEGB:
  case AV_CODEC_ID_LJPEG:
  case AV_CODEC_ID_JPEGLS:
  case AV_CODEC_ID_JPEG2000:
  case AV_CODEC_ID_SMVJPEG:
  case AV_CODEC_ID_PNG:
  case AV_CODEC_ID_GIF:
    return 1;
#endif // }
  default:
    return 0;
  }
}

int ffsox_audiostream(AVFormatContext *fc, int *aip, int *vip)
{
  AVCodecContext *cc;
  int i,ai,vi;

  ai=-1;
  vi=-1;

  for (i=0;i<(int)fc->nb_streams;++i) {
    cc=fc->streams[i]->codec;

    if (ffsox_codec_blacklist(cc->codec_id))
      continue;

    switch (cc->codec_type) {
      case AVMEDIA_TYPE_AUDIO:
        if (NULL!=aip&&0<=*aip) {
          if (*aip==i)
            ai=i;
        }
        else if (ai<0||2==cc->channels)
          ai=i;

        break;
      case AVMEDIA_TYPE_VIDEO:
        if (NULL!=vip&&0<=*vip) {
          if (*vip==i)
            vi=i;
        }
        else if (vi<0)
          vi=i;

        break;
      default:
        break;
    }
  };

  if (NULL!=aip)
    *aip=ai;

  if (NULL!=vip)
    *vip=vi;

  return ai;
}
