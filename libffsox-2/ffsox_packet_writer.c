/*
 * ffsox_packet_writer.c
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

static packet_writer_vmt_t vmt;

int ffsox_packet_writer_create(packet_writer_t *pw, source_t *si,
    int stream_index, sink_t *so)
{
  AVRational ar;

  // initialize the base calss.
  if (ffsox_packet_consumer_create(&pw->packet_consumer,si,stream_index)<0) {
    DMESSAGE("creating packet consumer");
    goto base;
  }

  // set the vmt.
  pw->vmt=ffsox_packet_writer_get_vmt();

  // create a new output stream.
  if (ffsox_stream_new(&pw->so,so,NULL)<0) {
    DMESSAGE("creating stream");
    goto ost;
  }

  // initialize the output stream form the input stream.
  if (avcodec_copy_context(pw->so.cc,pw->si.cc)<0) {
    DMESSAGE("copying context");
    goto occ;
  }

  switch (pw->si.cc->codec_type) {
  case AVMEDIA_TYPE_VIDEO:
    ar=pw->si.st->sample_aspect_ratio.num
        ?pw->si.st->sample_aspect_ratio
        :pw->si.cc->sample_aspect_ratio;
    pw->so.st->sample_aspect_ratio=ar;
    pw->so.cc->sample_aspect_ratio=ar;
    pw->so.st->avg_frame_rate=pw->si.st->avg_frame_rate;
    break;
  case AVMEDIA_TYPE_AUDIO:
    break;
  default:
    break;
  }

  pw->so.cc->codec_tag=0;

  if (pw->so.fc->oformat->flags&AVFMT_GLOBALHEADER)
    pw->so.cc->flags|=CODEC_FLAG_GLOBAL_HEADER;

  if (ffsox_sink_append(so,&pw->si,&pw->so)<0) {
    DMESSAGE("appending output stream");
    goto append;
  }

  return 0;
append:
occ:
ost:
  vmt.parent->cleanup(&pw->packet_consumer);
base:
  return -1;
}

ffsox_packet_writer_t *ffsox_packet_writer_new(ffsox_source_t *si,
    int stream_index, ffsox_sink_t *so)
{
  packet_writer_t *pw;

  if (NULL==(pw=MALLOC(sizeof *pw))) {
    DMESSAGE("allocating write copy node");
    goto malloc;
  }

  if (ffsox_packet_writer_create(pw,si,stream_index,so)<0) {
    DMESSAGE("creating write copy node");
    goto create;
  }

  return pw;
create:
  FREE(pw);
malloc:
  return NULL;
}

////////
static int packet_writer_run(packet_writer_t *n)
{
  switch (n->state) {
  case STATE_RUN:
    return MACHINE_POP;
  case STATE_FLUSH:
    n->state=STATE_END;
    return MACHINE_POP;
  case STATE_END:
    return MACHINE_POP;
  default:
    DMESSAGE("illegal packet writer state");
    return -1;
  }
}

static int packet_writer_set_packet(packet_writer_t *n, AVPacket *pkt)
{
  AVStream *sti=n->si.st;
  AVStream *sto=n->so.st;

  if (NULL==pkt)
    n->state=STATE_FLUSH;
  else {
    if (pkt->pts!=AV_NOPTS_VALUE)
      pkt->pts=av_rescale_q(pkt->pts,sti->time_base,sto->time_base);

    if (pkt->dts!=AV_NOPTS_VALUE)
      pkt->dts=av_rescale_q(pkt->dts,sti->time_base,sto->time_base);

    pkt->duration=av_rescale_q(pkt->duration,sti->time_base,sto->time_base);

    if (ffsox_stream_interleaved_write(&n->so,pkt)<0) {
      DMESSAGE("writing packet");
      return -1;
    }
  }

  return 0;
}

const packet_writer_vmt_t *ffsox_packet_writer_get_vmt(void)
{
  const packet_consumer_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_packet_consumer_get_vmt();
    vmt.packet_consumer=*parent;
    vmt.parent=parent;
    vmt.name="packet_writer";
    vmt.run=packet_writer_run;
    vmt.set_packet=packet_writer_set_packet;
  }

  return &vmt;
}
