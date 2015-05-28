/*
 * ffsox_frame_consumer.c
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

static frame_consumer_vmt_t vmt;

int ffsox_frame_consumer_create(frame_consumer_t *fc)
{
  if (ffsox_node_create(&fc->node)<0) {
    MESSAGE("creating node");
    goto base;
  }

  fc->vmt=ffsox_frame_consumer_get_vmt();
  fc->prev=NULL;
  fc->fi=NULL;

  return 0;
// cleanup:
  vmt.parent->cleanup(&fc->node);
base:
  return -1;
}

////////
static node_t *frame_consumer_prev(frame_consumer_t *n)
{
  return n->prev;
}

static int frame_consumer_set_frame(frame_consumer_t *n, ffsox_frame_t *fi)
{
  if (NULL==(n->fi=fi))
    n->state=STATE_FLUSH;

  return 0;
}

const frame_consumer_vmt_t *ffsox_frame_consumer_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="frame_consumer";
    vmt.prev=frame_consumer_prev;
    vmt.set_frame=frame_consumer_set_frame;
  }

  return &vmt;
}
