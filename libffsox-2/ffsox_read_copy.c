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

static read_copy_vmt_t vmt;

int ffsox_read_copy_create(read_copy_t *ni, source_t *si,
    int stream_index)
{
  if (ffsox_read_create(&ni->read,si,stream_index)<0) {
    MESSAGE("creating read node");
    goto base;
  }

  ni->vmt=ffsox_read_copy_get_vmt();
  ni->prev=NULL;
  ni->next=NULL;
  ni->pkt=&si->pkt;

  return 0;
base:
  return -1;
}

read_copy_t *ffsox_read_copy_new(source_t *s, int stream_index)
{
  read_copy_t *n;

  if (NULL==(n=malloc(sizeof *n))) {
    MESSAGE("allocating read copy node");
    goto malloc;
  }

  if (ffsox_read_copy_create(n,s,stream_index)<0) {
    MESSAGE("creating read copy node");
    goto create;
  }

  return n;
create:
  free(n);
malloc:
  return NULL;
}

////////
static void read_copy_cleanup(read_copy_t *n)
{
  if (NULL!=n->next)
    ffsox_node_destroy(&n->next->node);
}

static node_t *read_copy_prev(read_copy_t *n)
{
  return NULL==n->prev?NULL:&n->prev->node;
}

static node_t *read_copy_next(read_copy_t *n)
{
  return NULL==n->next?NULL:&n->next->node;
}

static int read_copy_run(read_copy_t *n)
{
  switch (n->state) {
  case STATE_RUN:
    return NULL==n->pkt?MACHINE_POP:MACHINE_PUSH;
  case STATE_FLUSH:
    n->next->state=STATE_END;
    n->state=STATE_END;
    return MACHINE_POP;
  case STATE_END:
    return MACHINE_POP;
  default:
    MESSAGE("illegal read copy state");
    return -1;
  }
}

static void read_copy_set_packet(read_copy_t *n, AVPacket *pkt)
{
  n->pkt=pkt;
}

const read_copy_vmt_t *ffsox_read_copy_get_vmt(void)
{
  const read_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_read_get_vmt();
    vmt.read=*parent;
    vmt.parent=parent;
    vmt.name="read_copy";
    vmt.cleanup=read_copy_cleanup;
    vmt.prev=read_copy_prev;
    vmt.next=read_copy_next;
    vmt.run=read_copy_run;
    vmt.set_packet=read_copy_set_packet;
  }

  return &vmt;
}
