/*
 * ffsox_write_copy.c
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

static write_copy_vmt_t vmt;

int ffsox_write_copy_create(write_copy_t *no, sink_t *so,
    read_copy_t *ni)
{
  AVRational ar;

  if (ffsox_write_create(&no->write,so,ni->s.codec)<0) {
    MESSAGE("creating write node");
    goto base;
  }

  no->vmt=ffsox_write_copy_get_vmt();

  if (avcodec_copy_context(no->s.cc,ni->s.cc)<0) {
    MESSAGE("copying context");
    goto occ;
  }

  switch (ni->s.cc->codec_type) {
  case AVMEDIA_TYPE_VIDEO:
    ar=ni->s.st->sample_aspect_ratio.num
        ?ni->s.st->sample_aspect_ratio
        :ni->s.cc->sample_aspect_ratio;
    no->s.st->sample_aspect_ratio=ar;
    no->s.cc->sample_aspect_ratio=ar;
    no->s.st->avg_frame_rate=ni->s.st->avg_frame_rate;
    break;
  case AVMEDIA_TYPE_AUDIO:
    break;
  default:
    break;
  }

  no->s.st->time_base=ni->s.st->time_base;
  //no->s.cc->codec_tag=0;

  if (no->s.fc->oformat->flags&AVFMT_GLOBALHEADER)
    no->s.cc->flags|=CODEC_FLAG_GLOBAL_HEADER;

  no->prev=NULL;
  no->pkt=&ni->pkt;

  return 0;
occ:
base:
  return -1;
}

write_copy_t *ffsox_write_copy_new(sink_t *so, read_copy_t *ni)
{
  write_copy_t *no;

  if (NULL==(no=malloc(sizeof *no))) {
    MESSAGE("allocating write copy node");
    goto malloc;
  }

  if (ffsox_write_copy_create(no,so,ni)<0) {
    MESSAGE("creating write copy node");
    goto create;
  }

  return no;
create:
  free(no);
malloc:
  return NULL;
}

////////
static node_t *write_copy_prev(write_copy_t *n)
{
  return NULL==n->prev?NULL:&n->prev->node;
}

static int write_copy_run(write_copy_t *n)
{
  switch (n->state) {
  case STATE_RUN:
    if (ffsox_write_interleaved(&n->write,*n->pkt)<0) {
      MESSAGE("writing packet");
      return -1;
    }

    *n->pkt=NULL;

    return MACHINE_POP;
  case STATE_FLUSH:
    n->state=STATE_END;
    return MACHINE_POP;
  case STATE_END:
    return MACHINE_POP;
  default:
    MESSAGE("illegal write copy state");
    return -1;
  }
}

const write_copy_vmt_t *ffsox_write_copy_get_vmt(void)
{
  const write_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_write_get_vmt();
    vmt.write=*parent;
    vmt.parent=parent;
    vmt.name="write_copy";
    vmt.prev=write_copy_prev;
    vmt.run=write_copy_run;
  }

  return &vmt;
}
