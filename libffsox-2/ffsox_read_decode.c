/*
 * ffsox_read_decode.c
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

static read_decode_vmt_t vmt;

int ffsox_read_decode_create(read_decode_t *rd, source_t *si,
    int stream_index, double drc)
{
  AVDictionary *opts=NULL;
  char buf[32];

  if (ffsox_read_create(&rd->read,si,stream_index)<0) {
    MESSAGE("creating read node");
    goto base;
  }

  rd->vmt=ffsox_read_decode_get_vmt();

  // find a decoder.
  rd->s.codec=NULL;

  switch (rd->s.cc->codec_id) {
  case AV_CODEC_ID_MP1:
    rd->s.codec=avcodec_find_decoder_by_name("mp1float");
    break;
  case AV_CODEC_ID_MP2:
    rd->s.codec=avcodec_find_decoder_by_name("mp2float");
    break;
  case AV_CODEC_ID_MP3:
    rd->s.codec=avcodec_find_decoder_by_name("mp3float");
    break;
  default:
    break;
  }

  if (NULL==rd->s.codec) {
    if (NULL==(rd->s.codec=avcodec_find_decoder(rd->s.cc->codec_id))) {
      MESSAGE("finding decoder");
      goto find;
    }
  }

  // we want to have stereo.
  // TODO: should be an option.
  rd->s.cc->request_channel_layout=AV_CH_LAYOUT_STEREO;

  if (AV_CODEC_ID_AC3==rd->s.cc->codec_id) {
    // avoid dynamic range compression.
    sprintf(buf,"%0.2f",drc);
    av_dict_set(&opts,"drc_scale",buf,0);
  }

  if (avcodec_open2(rd->s.cc,rd->s.codec,&opts)<0) {
    av_dict_free(&opts);
    MESSAGE("opening decoder");
    goto open;
  }

  av_dict_free(&opts);

  if (ffsox_frame_create(&rd->f)<0)
    goto frame;

  rd->prev=NULL;
  rd->next=NULL;
  memset(&rd->pkt,0,sizeof rd->pkt);

  return 0;
// close:
  ffsox_frame_cleanup(&rd->f);
frame:
  avcodec_close(rd->s.cc);
open:
find:
base:
  return -1;
}

read_decode_t *ffsox_read_decode_new(source_t *s, int stream_index,
    double drc)
{
  read_decode_t *n;

  if (NULL==(n=malloc(sizeof *n))) {
    MESSAGE("allocating read copy node");
    goto malloc;
  }

  if (ffsox_read_decode_create(n,s,stream_index,drc)<0) {
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
static void read_decode_cleanup(read_decode_t *n)
{
  if (NULL!=n->next)
    ffsox_node_destroy(&n->next->node);

  ffsox_frame_cleanup(&n->f);
  avcodec_close(n->s.cc);
}

static node_t *read_decode_prev(read_decode_t *n)
{
  return NULL==n->prev?NULL:&n->prev->node;
}

static node_t *read_decode_next(read_decode_t *n)
{
  return NULL==n->next?NULL:&n->next->node;
}

static int read_decode_run(read_decode_t *n)
{
  AVCodecContext *cc=n->s.cc;
  frame_t *f=&n->f;
  AVFrame *frame=f->frame;
  AVPacket *pkt=&n->pkt;
  int got_frame,size;

  switch (n->state) {
  case STATE_RUN:
    while (0<pkt->size) {
      if (0ll<f->nb_samples.frame) {
        MESSAGE("frame not consumed");
        return -1;
      }

      if ((size=avcodec_decode_audio4(cc,frame,&got_frame,pkt))<0) {
        MESSAGE("decoding audio");
        return -1;
      }

      pkt->size-=size;
      pkt->data+=size;

      if (0!=got_frame)
        return MACHINE_PUSH;
    }

    return MACHINE_POP;
  case STATE_FLUSH:
    pkt->size=0;
    pkt->data=NULL;

    if (avcodec_decode_audio4(cc,frame,&got_frame,pkt)<0) {
      MESSAGE("decoding audio");
      return -1;
    }

    if (0!=got_frame)
      return MACHINE_PUSH;
    else {
      n->next->state=STATE_FLUSH;
      n->state=STATE_END;
      return MACHINE_PUSH;
    }
  case STATE_END:
    return MACHINE_POP;
  default:
    MESSAGE("illegal read decode state");
    return -1;
  }
}

static void read_decode_set_packet(read_decode_t *n, AVPacket *pkt)
{
  if (NULL==pkt) {
    n->pkt.size=0;
    n->pkt.data=NULL;
  }
  else
    n->pkt=*pkt;
}

const read_decode_vmt_t *ffsox_read_decode_get_vmt(void)
{
  const read_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_read_get_vmt();
    vmt.read=*parent;
    vmt.parent=parent;
    vmt.name="read_decode";
    vmt.cleanup=read_decode_cleanup;
    vmt.prev=read_decode_prev;
    vmt.next=read_decode_next;
    vmt.run=read_decode_run;
    vmt.set_packet=read_decode_set_packet;
  }

  return &vmt;
}
