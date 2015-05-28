/*
 * bs1770gain_seek.c
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

int64_t bs1770gain_seek(AVFormatContext *ifc, const bs1770gain_options_t *o)
{
  AVStream *st;
  int si;
  int64_t ts;

  ts=0;

  if (o->begin>0) {
    si=av_find_default_stream_index(ifc);
    st=ifc->streams[si];
    ts=av_rescale_q(o->begin,AV_TIME_BASE_Q,st->time_base);
    BS1770GAIN_GOTO(avformat_seek_file(ifc,si,INT64_MIN,ts,INT64_MAX,0)<0,
        "seeking",seek);
    ts=av_rescale_q(st->cur_dts,st->time_base,AV_TIME_BASE_Q);
  }

  return ts;
seek:
  return -1;
}
