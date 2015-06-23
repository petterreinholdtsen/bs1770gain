/*
 * ffsox_stream.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@users.sf.net>
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
#include <ffsox_priv.h>

int ffsox_stream_new(stream_t *s, sink_t *so, AVCodec *codec)
{
  s->fc=so->f.fc;
  s->stream_index=s->fc->nb_streams;

  if (NULL==(s->st=avformat_new_stream(s->fc,codec))) {
    DMESSAGE("creating output stream");
    goto st;
  }

  s->cc=s->st->codec;
  s->codec=s->cc->codec;

  return 0;
st:
  return -1;
}

int ffsox_stream_interleaved_write(stream_t *s, AVPacket *pkt)
{
  pkt->stream_index=s->stream_index;

#if 0 // {
  fprintf(stderr,"%d: %I64d, %I64d\n",pkt->stream_index,pkt->pts,pkt->dts);
#endif // }

  return av_interleaved_write_frame(s->fc,pkt);
}
