/*
 * ffsox_basename.c
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
#include <ffsox_priv.h>

int ffsox_sink_create(sink_t *s, const char *path)
{
  s->f.path=path;
  s->f.fc=NULL;

  if (avformat_alloc_output_context2(&s->f.fc,NULL,NULL,path)<0) {
    DMESSAGE("allocating output context");
    goto fc;
  }

  s->streams=NULL;

  return 0;
// cleanup:
  avformat_free_context(s->f.fc);
fc:
  return -1;
}

void ffsox_sink_cleanup(sink_t *s)
{
  pbu_list_free(s->streams);
  avformat_free_context(s->f.fc);
}

int ffsox_sink_append(sink_t *sink, stream_t *si, stream_t *so)
{
  ffsox_stream_list_t stream;

  stream.si=si;
  stream.so=so;

  return PBU_LIST_APPEND(sink->streams,stream);
}

int ffsox_sink_open(sink_t *s)
{
  if (0==(s->f.fc->oformat->flags&AVFMT_NOFILE)) {
    if (avio_open(&s->f.fc->pb,s->f.path,AVIO_FLAG_WRITE)<0) {
      DMESSAGE("opening output file");
      goto open;
    }
  }

  if (avformat_write_header(s->f.fc,NULL)<0) {
    DMESSAGE("writing header");
    goto header;
  }

  return 0;
// close:
  av_write_trailer(s->f.fc);
header:
  if (0==(s->f.fc->flags&AVFMT_NOFILE))
    avio_close(s->f.fc->pb);
open:
  return -1;
}

void ffsox_sink_close(sink_t *s)
{
  av_write_trailer(s->f.fc);

  if (0==(s->f.fc->flags&AVFMT_NOFILE))
    avio_close(s->f.fc->pb);
}
