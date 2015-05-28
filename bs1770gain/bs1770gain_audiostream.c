/*
 * bs1770gain_audiostream.c
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
#include <bs1770gain.h>

int bs1770gain_audiostream(AVFormatContext *ic, int *aip, int *vip,
    const bs1770gain_options_t *options)
{
  AVStream *is;
  int i,ai,vi;

  ai=-1;
  vi=-1;

  for (i=0;i<ic->nb_streams;++i) {
    switch ((is=ic->streams[i])->codec->codec_type) {
      case AVMEDIA_TYPE_AUDIO:
        if (0<=options->audio) {
          if (options->audio==i)
            ai=i;
        }
        else if (ai<0||2==is->codec->channels)
          ai=i;

        break;
      case AVMEDIA_TYPE_VIDEO:
        if (0<=options->video) {
          if (options->video==i)
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
