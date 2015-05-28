/*
 * ffsox_write_encode.c
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

static write_encode_vmt_t vmt;

int ffsox_write_encode_create(write_encode_t *we, sink_t *so,
    read_decode_t *rd, int codec_id, int sample_fmt)
{
  if (ffsox_write_create(&we->write,so,NULL)<0) {
    MESSAGE("creating write node");
    goto base;
  }

  we->vmt=ffsox_write_encode_get_vmt();

  if (NULL==(we->s.codec=avcodec_find_encoder(codec_id)))
      goto codec;

  we->s.cc->sample_fmt=sample_fmt;
  we->s.cc->sample_rate=rd->s.cc->sample_rate;
  we->s.cc->channels=rd->s.cc->channels;
  we->s.cc->channel_layout=rd->s.cc->channel_layout;
  we->s.cc->time_base=(AVRational){1,we->s.cc->sample_rate};
  //we->s.cc->time_base=rd->s.cc->time_base;

  if (avcodec_open2(we->s.cc,we->s.codec,NULL)<0)
    goto open;

  if (we->s.fc->oformat->flags&AVFMT_GLOBALHEADER)
    we->s.cc->flags|=CODEC_FLAG_GLOBAL_HEADER;

  we->s.st->time_base=we->s.cc->time_base;

  memset(&we->pkt,0,sizeof we->pkt);
  av_init_packet(&we->pkt);
  we->prev=NULL;

  return 0;
// cleanup:
  avcodec_close(we->s.cc);
open:
codec:
base:
  return -1;
}

write_encode_t *ffsox_write_encode_new(sink_t *so, read_decode_t *rd,
    int codec_id, int sample_fmt)
{
  write_encode_t *we;

  if (NULL==(we=malloc(sizeof *we))) {
    MESSAGE("allocating write encode node");
    goto malloc;
  }

  if (ffsox_write_encode_create(we,so,rd,codec_id,sample_fmt)<0) {
    MESSAGE("creating write encode node");
    goto create;
  }

  return we;
create:
  free(we);
malloc:
  return NULL;
}

////////
static node_t *write_encode_prev(write_encode_t *n)
{
  return NULL==n->prev?NULL:&n->prev->node;
}

static int write_encode_run(write_encode_t *n)
{
  AVCodecContext *cc=n->s.cc;
  AVPacket *pkt=&n->pkt;
  frame_t *f=&n->prev->f;
  AVFrame *frame=f->frame;
  AVRational q={1,cc->sample_rate};
  int got_packet;

  switch (n->state) {
  case STATE_RUN:
    frame->pts=av_rescale_q(f->nb_samples.stream,q,cc->time_base);
    ffsox_frame_reset(f);

    if (avcodec_encode_audio2(cc,pkt,frame,&got_packet)<0) {
      MESSAGE("encoding audio");
      return -1;
    }

    if (0!=got_packet) {
      av_packet_rescale_ts(pkt,n->s.cc->time_base,n->s.st->time_base);

      if (ffsox_write_interleaved(&n->write,pkt)<0) {
        MESSAGE("writing packet");
        return -1;
      }
    }

    return MACHINE_POP;
  case STATE_FLUSH:
    if (avcodec_encode_audio2(cc,pkt,NULL,&got_packet)<0) {
      MESSAGE("encoding audio");
      return -1;
    }

    if (0!=got_packet) {
      if (ffsox_write_interleaved(&n->write,pkt)<0) {
        MESSAGE("writing packet");
        return -1;
      }

      return MACHINE_STAY;
    }
    else {
      n->state=STATE_END;
      return MACHINE_POP;
    }
  case STATE_END:
    return MACHINE_POP;
  default:
    MESSAGE("illegal encode state");
    return -1;
  }
}

const write_encode_vmt_t *ffsox_write_encode_get_vmt(void)
{
  const write_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_write_get_vmt();
    vmt.write=*parent;
    vmt.parent=parent;
    vmt.name="write_encode";
    vmt.prev=write_encode_prev;
    vmt.run=write_encode_run;
  }

  return &vmt;
}
