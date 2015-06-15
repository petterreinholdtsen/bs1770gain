/*
 * ffsox_sox_reader.c
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

static sox_reader_vmt_t vmt;

int ffsox_sox_reader_create(sox_reader_t *sa, frame_reader_t *fr, double q,
    intercept_t *intercept)
{
  AVCodecContext *cc=fr->si.cc;
  AVFrame *frame;

  // initialize the base class.
  if (ffsox_frame_consumer_create(&sa->frame_consumer)<0) {
    DMESSAGE("creating frame consumer");
    goto base;
  }

  // set the vmt.
  sa->vmt=ffsox_sox_reader_get_vmt();

  // initialize sox encoding.
  sa->encoding.encoding=SOX_ENCODING_SIGN2;
  sa->encoding.bits_per_sample=32;
  sa->encoding.opposite_endian=0;
  sox_init_encodinginfo(&sa->encoding);

  // initialize sox signal.
  sa->signal.rate=cc->sample_rate;
  sa->signal.channels=cc->channels;
  sa->signal.precision=32;
  sa->signal.length=0;
  sa->signal.mult=NULL;

  // initializing frame.
  if (ffsox_frame_create(&sa->fo)<0) {
    DMESSAGE("allocating frame");
    goto frame;
  }

  frame=sa->fo.frame;
  frame->format=AV_SAMPLE_FMT_S32;
  frame->channel_layout=cc->channel_layout;
  frame->channels=cc->channels;
  frame->sample_rate=cc->sample_rate;
  frame->nb_samples=0;

  sa->q=q;
  sa->intercept=intercept;
  sa->clips=0;
  sa->sox_errno=0;
  sa->prev=&fr->node;
  fr->next=&sa->frame_consumer;
  sa->next=NULL;

  return 0;
// cleanup:
  ffsox_frame_cleanup(&sa->fo);
frame:
  vmt.parent->cleanup(&sa->frame_consumer);
base:
  return -1;
}

sox_reader_t *ffsox_sox_reader_new(frame_reader_t *fr, double q,
    intercept_t *intercept)
{
  sox_reader_t *sa;

  if (NULL==(sa=MALLOC(sizeof *sa))) {
    DMESSAGE("allocating write encode node");
    goto malloc;
  }

  if (ffsox_sox_reader_create(sa,fr,q,intercept)<0) {
    DMESSAGE("creating write encode node");
    goto create;
  }

  return sa;
create:
  FREE(sa);
malloc:
  return NULL;
}

////////
size_t ffsox_sox_reader_read(sox_reader_t *sa, sox_sample_t *buf, size_t len)
{
  AVFrame *frame=sa->fo.frame;
  int channels=frame->channels;
  machine_t m;

  if (STATE_END==sa->state)
    return 0;
  else {
    frame->data[0]=(uint8_t *)buf;
    frame->nb_samples=len/channels;

    if (ffsox_machine_run(&m,&sa->node)<0) {
      DMESSAGE("running machine");
      goto machine;
    }

    return frame->nb_samples*channels;
  }
machine:
  sa->sox_errno=SOX_EOF;

  return 0;
}

////////
static void sox_reader_cleanup(sox_reader_t *sa)
{
  sa->fo.frame->data[0]=NULL;
  sa->fo.frame->nb_samples=0;
  ffsox_frame_cleanup(&sa->fo);
  vmt.parent->cleanup(&sa->frame_consumer);
}

static node_t *sox_reader_next(sox_reader_t *sa)
{
  return NULL==sa->next?NULL:&sa->next->node;
}

static int sox_reader_next_set_frame(sox_reader_t *sa, ffsox_frame_t *fo)
{
  if (NULL!=fo)
    ffsox_frame_reset(fo);

  if (NULL!=sa->next) {
  	if (NULL==fo)
	  sa->next->state=STATE_FLUSH;

    if (sa->next->vmt->set_frame(sa->next,fo)<0) {
      DMESSAGE("setting frame");
      return -1;
    }
  }

  return MACHINE_PUSH;
}

static int sox_reader_run(sox_reader_t *sa)
{
  frame_t *fi=sa->fi;
  frame_t *fo=&sa->fo;

  switch (sa->state) {
  case STATE_RUN:
    if (NULL!=fi) {
      while (0==ffsox_frame_complete(fi)) {
        if (ffsox_frame_convert_sox(fi,fo,sa->q,sa->intercept,&sa->clips)<0) {
          DMESSAGE("converting");
          return -1;
        }

        if (0!=ffsox_frame_complete(fo))
          return sox_reader_next_set_frame(sa,fo);
      }

      ffsox_frame_reset(fi);
    }

    return MACHINE_POP;
  case STATE_FLUSH:
    if (0<fo->nb_samples.frame) {
      fo->frame->nb_samples=fo->nb_samples.frame;

      return sox_reader_next_set_frame(sa,fo);
    }
    else {
      sa->state=STATE_END;

      return sox_reader_next_set_frame(sa,NULL);
    }
  case STATE_END:
    return MACHINE_POP;
  default:
    DMESSAGE("illegal read decode state");
    return -1;
  }
}

static int sox_reader_set_frame(sox_reader_t *sa, frame_t *fi)
{
  if (NULL==(sa->fi=fi))
    sa->state=STATE_FLUSH;

  return 0;
}

const sox_reader_vmt_t *ffsox_sox_reader_get_vmt(void)
{
  const frame_consumer_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_frame_consumer_get_vmt();
    vmt.frame_consumer=*parent;
    vmt.parent=parent;
    vmt.name="sox_reader";
    vmt.cleanup=sox_reader_cleanup;
    vmt.next=sox_reader_next;
    vmt.run=sox_reader_run;
    vmt.set_frame=sox_reader_set_frame;
  }

  return &vmt;
}
