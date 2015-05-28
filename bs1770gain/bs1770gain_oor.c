/*
 * bs1770gain_oor.c
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

int bs1770gain_oor(AVPacket *p, const AVFormatContext *ifc,
    const bs1770gain_options_t *options)
{
  AVStream *ist;

  if (options->duration==0||AV_NOPTS_VALUE==p->pts)
    return 0;
  else {
    ist=ifc->streams[p->stream_index];

    return options->duration<av_rescale_q(p->pts,ist->time_base,
        AV_TIME_BASE_Q);
  }
}
