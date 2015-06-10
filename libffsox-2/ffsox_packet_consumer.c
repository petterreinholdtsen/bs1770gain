/*
 * ffsox_packet_consumer.c
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

static packet_consumer_vmt_t vmt;

int ffsox_packet_consumer_create(packet_consumer_t *pc, source_t *si,
    int stream_index)
{
  if (ffsox_node_create(&pc->node)<0) {
    MESSAGE("creating node");
    goto base;
  }

  pc->vmt=ffsox_packet_consumer_get_vmt();
  pc->si.fc=si->f.fc;
  pc->si.stream_index=stream_index;
  pc->si.st=pc->si.fc->streams[stream_index];
  pc->si.cc=pc->si.st->codec;
  pc->si.codec=pc->si.cc->codec;

  // link us to the packet consumer list.
  if (ffsox_source_append(si,pc)<0) {
    MESSAGE("appending packet consumer");
    goto append;
  }

  return 0;
append:
  vmt.parent->cleanup(&pc->node);
base:
  return -1;
}

////////
static node_t *packet_consumer_prev(packet_consumer_t *pc)
{
  return NULL==pc->prev?NULL:&pc->prev->node;
}

static int packet_consumer_set_packet(packet_consumer_t *pc, AVPacket *pkt)
{
  MESSAGE("not implemented");
  (void)pc;
  (void)pkt;

  return -1;
}

const packet_consumer_vmt_t *ffsox_packet_consumer_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="packet_consumer";
    vmt.prev=packet_consumer_prev;
    vmt.set_packet=packet_consumer_set_packet;
  }

  return &vmt;
}
