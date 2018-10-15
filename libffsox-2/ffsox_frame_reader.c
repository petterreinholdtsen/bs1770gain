/*
 * ffsox_frame_reader.c
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

static frame_reader_vmt_t vmt;

int ffsox_frame_reader_create(frame_reader_t *fr, source_t *si,
    int stream_index, int stereo, double drc)
{
  AVDictionary *opts=NULL;
  char buf[32];

  // initialize the base calss.
#if defined (FFSOX_FIX_883198_MISSING_CODEC_ID) // [
  if (ffsox_packet_consumer_create(&fr->packet_consumer,si,stream_index,0)<0) {
    DMESSAGE("creating packet consumer");
    goto base;
  }
#else // ] [
  if (ffsox_packet_consumer_create(&fr->packet_consumer,si,stream_index)<0) {
    DMESSAGE("creating packet consumer");
    goto base;
  }
#endif // ]

  // set the vmt.
  fr->vmt=ffsox_frame_reader_get_vmt();

  // find a decoder.
  if (NULL==(fr->si.codec=ffsox_find_decoder(fr->si.cc->codec_id))) {
    DMESSAGE("finding decoder");
    goto find;
  }

#if defined (FFSOX_FIX_881131_CHANNEL_UNDERFOW) // [
  if (fr->si.cc->channels<=0) {
    DVMESSAGE("channel underflow (%d - should be 1 at min)",
        fr->si.cc->channels);
    goto channel_underflow;
  }
#endif // ]

#if defined (FFSOX_FIX_881132_CHANNEL_OVERFLOW) // [
  if (AV_NUM_DATA_POINTERS<=fr->si.cc->channels) {
		DVMESSAGE("channel overflow (%d - should be %d at max)",
				fr->si.cc->channels,AV_NUM_DATA_POINTERS);
		goto channel_overflow;
	}
#endif // ]

#if defined (FFSOX_FIX_883198_MISSING_CODEC_ID) // [
  // link us to the packet consumer list.
  if (ffsox_source_append(si,&fr->packet_consumer)<0) {
    DMESSAGE("appending packet consumer");
    goto append;
  }
#endif // ]

  // if we want to have stereo:
  if (stereo)
    fr->si.cc->request_channel_layout=AV_CH_LAYOUT_STEREO;

  if (AV_CODEC_ID_AC3==fr->si.cc->codec_id) {
    // avoid dynamic range compression.
    sprintf(buf,"%0.2f",drc);
    av_dict_set(&opts,"drc_scale",buf,0);
  }

  if (avcodec_open2(fr->si.cc,fr->si.codec,&opts)<0) {
    av_dict_free(&opts);
    DMESSAGE("opening decoder");
    goto open;
  }

  av_dict_free(&opts);

  if (ffsox_frame_create(&fr->fo)<0)
    goto frame;

  fr->next=NULL;
  memset(&fr->pkt,0,sizeof fr->pkt);

  return 0;
// close:
  ffsox_frame_cleanup(&fr->fo);
frame:
  avcodec_close(fr->si.cc);
open:
#if defined (FFSOX_FIX_883198_MISSING_CODEC_ID) // [
append:
#endif // ]
#if defined (FFSOX_FIX_881132_CHANNEL_OVERFLOW) // [
channel_overflow:
#endif // ]
#if defined (FFSOX_FIX_881131_CHANNEL_UNDERFOW) // [
channel_underflow:
#endif // ]
find:
  vmt.parent->cleanup(&fr->packet_consumer);
base:
  return -1;
}

frame_reader_t *ffsox_frame_reader_new(source_t *si, int stream_index,
    int stereo, double drc)
{
  frame_reader_t *fr;

  if (NULL==(fr=MALLOC(sizeof *fr))) {
    DMESSAGE("allocating frame reader");
    goto malloc;
  }

  if (ffsox_frame_reader_create(fr,si,stream_index,stereo,drc)<0) {
    DMESSAGE("creating frame reader");
    goto create;
  }

  return fr;
create:
  FREE(fr);
malloc:
  return NULL;
}

////////
static void frame_reader_cleanup(frame_reader_t *fr)
{
  ffsox_frame_cleanup(&fr->fo);
  avcodec_close(fr->si.cc);
  vmt.parent->cleanup(&fr->packet_consumer);
}

static node_t *frame_reader_next(frame_reader_t *n)
{
  return NULL==n->next?NULL:&n->next->node;
}

static int frame_reader_next_set_frame(frame_reader_t *n, frame_t *fo)
{
//DMARKLN();
  if (NULL!=n->next) {
//DMARKLN();
    if (NULL==fo) {
//DMARKLN();
      n->next->state=STATE_FLUSH;
//DMARKLN();
	}

//DMARKLN();
    if (n->next->vmt->set_frame(n->next,fo)<0) {
//DMARKLN();
      DMESSAGE("setting frame");
      return -1;
    }
//DMARKLN();
  }
//DMARKLN();

  return MACHINE_PUSH;
}

#define FRAME_READER_SKIP_ERROR
#define FRAME_READER_RUN_CODE
static int frame_reader_run(frame_reader_t *n)
{
#if defined (FRAME_READER_RUN_CODE) // [
  int code=-1;
#endif // ]
  AVCodecContext *cc=n->si.cc;
  frame_t *fo=&n->fo;
  AVFrame *frame=fo->frame;
  AVPacket *pkt=&n->pkt;
  int got_frame,size;

//DMARKLN();
  switch (n->state) {
  case STATE_RUN:
//DMARKLN();
    while (0<pkt->size) {
//DMARKLN();
      if (0ll<fo->nb_samples.frame) {
        DMESSAGE("frame not consumed");
#if defined (FRAME_READER_RUN_CODE) // [
        goto exit;
#else // ] [
        return -1;
#endif // ]
      }

#if 0 // [
      if ((size=avcodec_decode_audio4(cc,frame,&got_frame,pkt))<0) {
#else // ] [
//DMARKLN();
      size=avcodec_decode_audio4(cc,frame,&got_frame,pkt);
//DVWRITELN("size: %d",size);

			if (size<0) {
				//DMARKLN();
#endif // ]
#if defined (FRAME_READER_SKIP_ERROR) // [
        // skip the package.
#if defined (PBU_DEBUG) // [
        DMESSAGE("decoding audio, skipping audio package");
#endif // ]
        pkt->size=0;
#if defined (FRAME_READER_RUN_CODE) // [
        code=0;
        goto exit;
#else // ] [
        return 0;
#endif // ]
#else // ] [
        DMESSAGE("decoding audio");
#if defined (FRAME_READER_RUN_CODE) // [
        goto exit;
#else // ] [
        return -1;
#endif // ]
#endif // ]
#if 1 // [
      }
//DMARKLN();
#else // ] [
      }
#endif // ]

#if 0 // [
      pkt->size-=size;
      pkt->data+=size;
#else // ][
//DVWRITELN("size: %d, pkt->size: %d",size,pkt->size);
      pkt->size-=size;
      pkt->data+=size;
//DVWRITELN("size: %d, pkt->size: %d",size,pkt->size);
//DVWRITELN("got_frame: %d",got_frame);
#endif // ]

      if (0!=got_frame) {
#if defined (FRAME_READER_RUN_CODE) // [
        code=frame_reader_next_set_frame(n,fo);
        goto exit;
#else // ] [
        return frame_reader_next_set_frame(n,fo);
#endif // ]
      } 
    }

    return MACHINE_POP;
  case STATE_FLUSH:
    pkt->size=0;
    pkt->data=NULL;

//DVWRITELN("cc: %p, codec_id: %0x, OPUS: %0x, frame: %p, got_frame: %d, pkt: %p",cc,cc->codec_id,AV_CODEC_ID_OPUS,frame,got_frame,pkt);
    if (AV_CODEC_ID_OPUS==cc->codec_id) {
      n->state=STATE_END;

      return frame_reader_next_set_frame(n,NULL);
    }
    else if (avcodec_decode_audio4(cc,frame,&got_frame,pkt)<0) {
      DMESSAGE("decoding audio");
#if defined (FRAME_READER_RUN_CODE) // [
        goto exit;
#else // ] [
        return -1;
#endif // ]
    }
//DMARKLN();

    if (0==got_frame) {
      n->state=STATE_END;

      return frame_reader_next_set_frame(n,NULL);
    }
    else {
#if defined (FRAME_READER_RUN_CODE) // [
        code=frame_reader_next_set_frame(n,fo);
        goto exit;
#else // ] [
        return frame_reader_next_set_frame(n,fo);
#endif // ]
    }
  case STATE_END:
//DMARKLN();
#if defined (FRAME_READER_RUN_CODE) // [
        code=MACHINE_POP;
        goto exit;
#else // ] [
        return MACHINE_POP;
#endif // ]
  default:
//DMARKLN();
    DMESSAGE("illegal read decode state");
#if defined (FRAME_READER_RUN_CODE) // [
    goto exit;
#else // ] [
    return -1;
#endif // ]
  }
#if defined (FRAME_READER_RUN_CODE) // [
exit: 
  return code;
#endif // ]
}

static int frame_reader_set_packet(frame_reader_t *n, AVPacket *pkt)
{
  if (NULL==pkt) {
    n->state=STATE_FLUSH;
    n->pkt.size=0;
    n->pkt.data=NULL;
  }
  else
    n->pkt=*pkt;

  return 0;
}

const frame_reader_vmt_t *ffsox_frame_reader_get_vmt(void)
{
  const packet_consumer_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_packet_consumer_get_vmt();
    vmt.packet_consumer=*parent;
    vmt.parent=parent;
    vmt.name="frame_reader";
    vmt.cleanup=frame_reader_cleanup;
    vmt.next=frame_reader_next;
    vmt.run=frame_reader_run;
    vmt.set_packet=frame_reader_set_packet;
  }

  return &vmt;
}
