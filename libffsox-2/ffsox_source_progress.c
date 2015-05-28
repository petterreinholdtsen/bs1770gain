/*
 * mux3.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@snafu.de>
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
#include <ffsox_priv.h>

void ffsox_source_progress(const source_t *si, void *data)
{
  FILE *f=data;
  const AVPacket *pkt;
  const AVStream *st;
  int64_t duration;
  double percent;
  char buf[32];
  int i;

  pkt=&si->pkt;
  st=si->f.fc->streams[pkt->stream_index];
  duration=av_rescale_q(si->f.fc->duration,AV_TIME_BASE_Q,st->time_base);
  percent=0ll<pkt->dts&&0ll<duration?100.0*pkt->dts/duration:0.0;
  sprintf(buf,"%.0f%%",percent);
  fputs(buf,f);
  fflush(f);
  i=strlen(buf);

  while (0<i) {
    fputc('\b',f);
    --i;
  }
}
