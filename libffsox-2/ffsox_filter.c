/*
 * ffsox_filter.c
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

static filter_vmt_t vmt;

int ffsox_filter_create(filter_t *n, write_encode_t *we, double q)
{
  int nb_samples;

  if (ffsox_node_create(&n->node)<0) {
    MESSAGE("creating node");
    goto base;
  }

  n->vmt=ffsox_filter_get_vmt();
  n->q=q;

  if (ffsox_frame_create(&n->f)<0)
    goto frame;

  if (we->s.codec->capabilities&CODEC_CAP_VARIABLE_FRAME_SIZE)
    nb_samples=10000;
  else
    nb_samples=we->s.cc->frame_size;

  n->f.frame->format=we->s.cc->sample_fmt;
  n->f.frame->channel_layout=we->s.cc->channel_layout;
  n->f.frame->channels=we->s.cc->channels;
  n->f.frame->sample_rate=we->s.cc->sample_rate;
  n->f.frame->nb_samples=nb_samples;

  if (0<nb_samples&&av_frame_get_buffer(n->f.frame,0)<0)
    goto buffer;

  n->prev=NULL;
  n->next=NULL;

  return 0;
// cleanup:
buffer:
  ffsox_frame_cleanup(&n->f);
frame:
base:
  return -1;
}

filter_t *ffsox_filter_new(write_encode_t *we, double q)
{
  filter_t *n;

  if (NULL==(n=malloc(sizeof *n))) {
    MESSAGE("allocating filter node");
    goto malloc;
  }

  if (ffsox_filter_create(n,we,q)<0) {
    MESSAGE("creating filter node");
    goto create;
  }

  return n;
create:
  free(n);
malloc:
  return NULL;
}

////////
static void filter_cleanup(filter_t *n)
{
  ffsox_frame_cleanup(&n->f);
}

static node_t *filter_prev(filter_t *n)
{
  return NULL==n->prev?NULL:&n->prev->node;
}

static node_t *filter_next(filter_t *n)
{
  return NULL==n->next?NULL:&n->next->node;
}

static int filter_run(filter_t *n)
{
  frame_t *fr=&n->prev->f;
  frame_t *fw=&n->f;

  switch (n->state) {
  case STATE_RUN:
    while (0==ffsox_frame_complete(fr)) {
      if (ffsox_frame_convert(fr,fw,n->q)<0) {
        MESSAGE("filtering audio");
        return -1;
      }

      if (0!=ffsox_frame_complete(fw))
        return MACHINE_PUSH;
    }

    ffsox_frame_reset(fr);
    return MACHINE_POP;
  case STATE_FLUSH:
    if (0ll<fw->nb_samples.frame) {
      fw->frame->nb_samples=fw->nb_samples.frame;
      return MACHINE_PUSH;
    }
    else {
      n->next->state=STATE_FLUSH;
      n->state=STATE_END;
      return MACHINE_PUSH;
    }
  case STATE_END:
    return MACHINE_POP;
  default:
    MESSAGE("illegal filter state");
    return -1;
  }
}

const filter_vmt_t *ffsox_filter_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="filter";
    vmt.cleanup=filter_cleanup;
    vmt.prev=filter_prev;
    vmt.next=filter_next;
    vmt.run=filter_run;
  }

  return &vmt;
}
