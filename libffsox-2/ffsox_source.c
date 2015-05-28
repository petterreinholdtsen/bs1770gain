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

static source_vmt_t vmt;

int ffsox_source_create(source_t *n, const char *path)
{
  if (ffsox_node_create(&n->node)<0) {
    MESSAGE("creating node");
    goto base;
  }

  n->vmt=ffsox_source_get_vmt();
  n->f.path=path;
  n->f.fc=NULL;

  if (avformat_open_input(&n->f.fc,path,0,0)<0) {
    MESSAGE("opening input file");
    goto fc;
  }

  if (avformat_find_stream_info(n->f.fc,0)<0) {
    MESSAGE("finding stream info");
    goto find;
  }

  n->reads.h=NULL;
  n->reads.n=NULL;
  n->next=NULL;
  memset(&n->pkt,0,sizeof n->pkt);
  av_init_packet(&n->pkt);

  return 0;
find:
  avformat_close_input(&n->f.fc);
fc:
  vmt.parent->cleanup(&n->node);
base:
  return -1;
}

////////
static void source_cleanup(source_t *n)
{
  avformat_close_input(&n->f.fc);
  vmt.parent->cleanup(&n->node);
}

static node_t *source_next(source_t *n)
{
  return NULL==n->next?NULL:&n->next->node;
}

static int source_run(source_t *n)
{
  read_t *read;

  switch (n->state) {
  case STATE_RUN:
    for (;;) {
      n->reads.n=n->reads.h;

      if (av_read_frame(n->f.fc,&n->pkt)<0) {
        n->state=STATE_FLUSH;
        goto flush;
      }

      while (NULL!=n->reads.n) {
        read=n->reads.n->read;

        if (n->pkt.stream_index==read->s.stream_index) {
          read->vmt->set_packet(read,&n->pkt);
          n->next=read;
          return MACHINE_PUSH;
        }

        LIST_NEXT(&n->reads.n,n->reads.h);
      }

      av_free_packet(&n->pkt);
    }
  case STATE_FLUSH:
  flush:
    if (NULL==n->reads.n) {
      n->next=NULL;
      n->state=STATE_END;
      return MACHINE_POP;
    }
    else {
      read=n->reads.n->read;
      read->state=STATE_FLUSH;
      read->vmt->set_packet(read,NULL);
      n->next=read;
      LIST_NEXT(&n->reads.n,n->reads.h);
      return MACHINE_PUSH;
    }
  case STATE_END:
    return MACHINE_POP;
  default:
    MESSAGE("illegal source state");
    return -1;
  }
}

const source_vmt_t *ffsox_source_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="source";
    vmt.cleanup=source_cleanup;
    vmt.next=source_next;
    vmt.run=source_run;
  }

  return &vmt;
}
