/*
 * ffsox_write.c
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

static write_vmt_t vmt;

int ffsox_write_create(write_t *no, sink_t *so, const AVCodec *codec)
{
  if (ffsox_node_create(&no->node)<0) {
    MESSAGE("creating node");
    goto base;
  }

  no->vmt=ffsox_write_get_vmt();

  no->s.fc=so->f.fc;
  no->s.stream_index=no->s.fc->nb_streams;

  if (NULL==(no->s.st=avformat_new_stream(no->s.fc,codec))) {
    MESSAGE("creating output stream");
    goto ost;
  }

  no->s.cc=no->s.st->codec;
  no->s.codec=no->s.cc->codec;

  return 0;
ost:
base:
  return -1;
}

int ffsox_write_interleaved(write_t *n, AVPacket *pkt)
{
  pkt->stream_index=n->s.stream_index;

  if (av_interleaved_write_frame(n->s.fc,pkt)<0)
    goto write;

  return 0;
write:
  return -1;
}

////////
const write_vmt_t *ffsox_write_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="write";
  }

  return &vmt;
}
