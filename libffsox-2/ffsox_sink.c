/*
 * ffsox_basename.c
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
#include <ffsox_priv.h>

int ffsox_sink_create(sink_t *s, const char *path)
{
  s->f.path=path;
  s->f.fc=NULL;

  if (avformat_alloc_output_context2(&s->f.fc,NULL,NULL,path)<0) {
    MESSAGE("allocating output context");
    goto fc;
  }

  return 0;
// cleanup:
  avformat_free_context(s->f.fc);
fc:
  return -1;
}

void ffsox_sink_cleanup(sink_t *s)
{
  avformat_free_context(s->f.fc);
}

int ffsox_sink_open(sink_t *s)
{
  if (0==(s->f.fc->oformat->flags&AVFMT_NOFILE)) {
    if (avio_open(&s->f.fc->pb,s->f.path,AVIO_FLAG_WRITE)<0) {
      MESSAGE("opening output file");
      goto open;
    }
  }

  if (avformat_write_header(s->f.fc,NULL)<0) {
    MESSAGE("writing header");
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
