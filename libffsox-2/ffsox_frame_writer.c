/*
 * ffsox_frame_writer.c
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

static frame_writer_vmt_t vmt;

int ffsox_frame_writer_create(frame_writer_t *fw, sink_t *so,
    frame_reader_t *fr, int codec_id, int sample_fmt, double q)
{
  // initialize the base class.
  if (ffsox_frame_consumer_create(&fw->frame_consumer)<0) {
    DMESSAGE("creating frame consumer");
    goto base;
  }

  // set the vmt.
  fw->vmt=ffsox_frame_writer_get_vmt();
  fw->q=q;

  // create a new output stream.
  if (ffsox_stream_new(&fw->so,so,NULL)<0) {
    DMESSAGE("creating stream");
    goto ost;
  }

  // initialize the output stream.
  if (codec_id<0)
    codec_id=fr->si.cc->codec_id;

  if (sample_fmt<0)
    sample_fmt=fr->si.cc->sample_fmt;

  if (NULL==(fw->so.codec=avcodec_find_encoder(codec_id)))
    goto codec;

  fw->so.cc->sample_fmt=sample_fmt;
  fw->so.cc->sample_rate=fr->si.cc->sample_rate;
  fw->so.cc->channels=fr->si.cc->channels;
  fw->so.cc->channel_layout=fr->si.cc->channel_layout;
#if 0 // {
  fw->so.cc->time_base=(AVRational){1,fw->so.cc->sample_rate};
#else // } {
  fw->so.cc->time_base.num=1;
  fw->so.cc->time_base.den=fw->so.cc->sample_rate;
#endif
  //fw->so.cc->time_base=fr->si.cc->time_base;

  if (avcodec_open2(fw->so.cc,fw->so.codec,NULL)<0)
    goto open;

  if (fw->so.fc->oformat->flags&AVFMT_GLOBALHEADER)
    fw->so.cc->flags|=CODEC_FLAG_GLOBAL_HEADER;

  fw->so.st->time_base=fw->so.cc->time_base;
  //fw->so.st->time_base=fr->si.st->time_base;

  // initializing frame.
  if (ffsox_frame_create_cc(&fw->fo,fw->so.cc)<0) {
    DMESSAGE("allocating frame");
    goto frame;
  }

  // initialize the packet.
  memset(&fw->pkt,0,sizeof fw->pkt);
  av_init_packet(&fw->pkt);

  if (ffsox_sink_append(so,&fr->si,&fw->so)<0) {
    DMESSAGE("appending output stream");
    goto append;
  }

  return 0;
// cleanup:
append:
  ffsox_frame_cleanup(&fw->fo);
frame:
  avcodec_close(fw->so.cc);
open:
codec:
ost:
  vmt.parent->cleanup(&fw->frame_consumer);
base:
  return -1;
}

ffsox_frame_writer_t *ffsox_frame_writer_new(sink_t *so, frame_reader_t *fr,
    int codec_id, int sample_fmt, double q)
{
  frame_writer_t *fw;

  if (NULL==(fw=MALLOC(sizeof *fw))) {
    DMESSAGE("allocating write encode node");
    goto malloc;
  }

  if (ffsox_frame_writer_create(fw,so,fr,codec_id,sample_fmt,q)<0) {
    DMESSAGE("creating write encode node");
    goto create;
  }

  return fw;
create:
  FREE(fw);
malloc:
  return NULL;
}

////////
static void frame_writer_cleanup(frame_writer_t *fw)
{
  ffsox_frame_cleanup(&fw->fo);
  avcodec_close(fw->so.cc);
  vmt.parent->cleanup(&fw->frame_consumer);
}

static int frame_writer_encode(frame_writer_t *fw, AVFrame *frame,
    int *got_packet)
{
  stream_t *so=&fw->so;
  AVStream *st=so->st;
  AVCodecContext *cc=so->cc;
  AVPacket *pkt=&fw->pkt;

  if (avcodec_encode_audio2(cc,pkt,frame,got_packet)<0) {
    DMESSAGE("encoding audio");
    goto encode;
  }

  if (0!=*got_packet) {
    av_packet_rescale_ts(pkt,cc->time_base,st->time_base);

    if (ffsox_stream_interleaved_write(so,pkt)<0) {
      DMESSAGE("writing packet");
      goto write;
    }
  }

  return 0;
write:
encode:
  return -1;
}

static int frame_writer_convert(frame_writer_t *fw, int *got_packet)
{
  stream_t *so=&fw->so;
  AVCodecContext *cc=so->cc;
  AVRational q={1,cc->sample_rate};
  frame_t *fi=fw->fi;
  frame_t *fo=&fw->fo;
  AVFrame *frame;

  if (NULL!=fi) {
    if (ffsox_frame_convert(fi,fo,fw->q)<0) {
      DMESSAGE("converting frame");
      goto convert;
    }
  }

  if (ffsox_frame_complete(fo)) {
    frame=fo->frame;
    frame->pts=av_rescale_q(fo->nb_samples.stream,q,cc->time_base);
    fo->nb_samples.stream+=fo->nb_samples.frame;

    if (frame_writer_encode(fw,frame,got_packet)<0) {
      DMESSAGE("encoding");
      goto encode;
    }

    ffsox_frame_reset(fo);
  }

  return 0;
convert:
encode:
  return -1;
}

static int frame_writer_run(frame_writer_t *fw)
{
  frame_t *fi=fw->fi;
  frame_t *fo=&fw->fo;
  int got_packet;

  switch (fw->state) {
  case STATE_RUN:
    // fi may be NULL if we start the loop with the frame writer.
    if (NULL!=fi) {
      while (0==ffsox_frame_complete(fi)) {
        if (frame_writer_convert(fw,&got_packet)<0) {
          DMESSAGE("converting");
          return -1;
        }
      }

      ffsox_frame_reset(fi);
    }

    return MACHINE_POP;
  case STATE_FLUSH:
    if (0<fo->nb_samples.frame) {
      fo->frame->nb_samples=fo->nb_samples.frame;
      
      if (frame_writer_convert(fw,&got_packet)<0) {
        DMESSAGE("converting");
        return -1;
      }
    }

    do {
      if (frame_writer_encode(fw,NULL,&got_packet)<0) {
        DMESSAGE("encoding");
        return -1;
      }
    } while (0!=got_packet);

    fw->state=STATE_END;

    return MACHINE_POP;
  case STATE_END:
    return MACHINE_POP;
  default:
    DMESSAGE("illegal encode state");
    return -1;
  }
}

const frame_writer_vmt_t *ffsox_frame_writer_get_vmt(void)
{
  const frame_consumer_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_frame_consumer_get_vmt();
    vmt.frame_consumer=*parent;
    vmt.parent=parent;
    vmt.name="frame_writer";
    vmt.cleanup=frame_writer_cleanup;
    vmt.run=frame_writer_run;
  }

  return &vmt;
}
