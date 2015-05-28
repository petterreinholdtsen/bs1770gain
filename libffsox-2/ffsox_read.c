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

static read_vmt_t vmt;

int ffsox_read_create(read_t *ni, ffsox_source_t *si, int stream_index)
{
  if (ffsox_node_create(&ni->node)<0) {
    MESSAGE("creating node");
    goto base;
  }

  ni->vmt=ffsox_read_get_vmt();
  ni->s.fc=si->f.fc;
  ni->s.stream_index=stream_index;
  ni->s.st=ni->s.fc->streams[stream_index];
  ni->s.cc=ni->s.st->codec;
  ni->s.codec=ni->s.cc->codec;
  ni->write=NULL;
  ni->prev=NULL;

  return 0;
base:
  return -1;
}

////////
static void read_set_packet(read_t *n, AVPacket *pkt)
{
}

const read_vmt_t *ffsox_read_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="read";
    vmt.set_packet=read_set_packet;
  }

  return &vmt;
}
