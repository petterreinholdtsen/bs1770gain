/*
 * bs1770gain_stream.c
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
#include <bs1770gain_format.h>

///////////////////////////////////////////////////////////////////////////////
typedef bs1770gain_stream_t s_t;

///////////////////////////////////////////////////////////////////////////////
// common
s_t *bs1770gain_stream_create(s_t *s, AVFormatContext *fc)
{
  memset(s,0,sizeof *s);
  s->vmt=bs1770gain_stream_vmt();
  s->fc=fc;
  s->stream_index=-1;

  return s;
}

////////
static void bs1770gain_stream_cleanup(s_t *s)
{
}

static int bs1770gain_stream_decode(s_t *rs, AVPacket *pkt2, int *got_packet)
{
  return -1;
}

static int bs1770gain_stream_encode(s_t *ws)
{
  return -1;
}

static int bs1770gain_stream_flush(s_t *s)
{
  return -1;
}

const bs1770gain_stream_vmt_t *bs1770gain_stream_vmt(void)
{
  static bs1770gain_stream_vmt_t vmt;

  if (NULL==vmt.type) {
    vmt.type="bs1770gain_stream";
    vmt.tag=BS1770GAIN_STREAM_COMMON;
    vmt.cleanup=bs1770gain_stream_cleanup;
    vmt.decode=bs1770gain_stream_decode;
    vmt.encode=bs1770gain_stream_encode;
    vmt.flush=bs1770gain_stream_flush;
  }

  return &vmt;
}

////////
int bs1770gain_stream_frame_complete(s_t *s)
{
  // do we have processed (read or written, respectively) all samples
  // of the frame?
  return s->nb_samples.frame==s->frame->nb_samples;
}

int bs1770gain_stream_frame_convert(s_t *sr, s_t *sw)
{
  // we write from an arbitray input format to interleaved signed 32.
  //
  // read pointers:
  union {
#if 0 // {
    // interleaved.
    struct {
      const int16_t *rp;
    } s16i;
    // planar.
    struct {
      const float *rp[AV_NUM_DATA_POINTERS];
    } fltp;
#else // } {
    // interleaved.
    struct {
      const int8_t *rp;
    } u8i;
    struct {
      const int16_t *rp;
    } s16i;
    struct {
      const int32_t *rp;
    } s32i;
    struct {
      const float *rp;
    } flti;
    struct {
      const double *rp;
    } dbli;
    // planar.
    struct {
      const int8_t *rp[AV_NUM_DATA_POINTERS];
    } u8p;
    struct {
      const int16_t *rp[AV_NUM_DATA_POINTERS];
    } s16p;
    struct {
      const int32_t *rp[AV_NUM_DATA_POINTERS];
    } s32p;
    struct {
      const float *rp[AV_NUM_DATA_POINTERS];
    } fltp;
    struct {
      const double *rp[AV_NUM_DATA_POINTERS];
    } dblp;
#endif // }
  } u;

  double q=1.0;
  int nb_samples1,nb_samples2,nb_samples;
  int channels,ch;
  // write pointers:
  int32_t *wp,*mp;

  // get the number of channels
  channels=av_frame_get_channels(sr->frame);

  // we can write the minimum from input and output nb_samples
  nb_samples1=sr->frame->nb_samples-sr->nb_samples.frame;
  nb_samples2=sw->frame->nb_samples-sw->nb_samples.frame;
  nb_samples=nb_samples1<nb_samples2?nb_samples1:nb_samples2;

  // seek to the output offset
  wp=(int32_t *)sw->frame->data[0];
  wp+=channels*sw->nb_samples.frame;
  // the maximum is another nb_samples away from the write pointer
  mp=wp+channels*nb_samples;

  switch (sr->frame->format) {
#if 0 // {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_S16:
    // seek to the input offset
    u.s16i.rp=(const int16_t *)sr->frame->data[0];
    u.s16i.rp+=channels*sr->nb_samples.frame;

    // write the nb_samples samples
    while (wp<mp) {
      *wp=*u.s16i.rp++;
      *wp++<<=16;
    }

    break;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_FLTP:
    // seek to the input offsets
    for (ch=0;ch<channels;++ch) {
      u.fltp.rp[ch]=(float *)sr->frame->data[0];
      u.fltp.rp[ch]+=channels*sr->nb_samples.frame;
    }

    // write the nb_samples samples
    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor((*u.fltp.rp[ch]++)*INT32_MAX+0.5);
    }

    break;
#else // } {
  /// interleaved ///////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    u.u8i.rp=(const int8_t *)sr->frame->data[0];
    u.u8i.rp+=channels*sr->nb_samples.frame;

    while (wp<mp)
      *wp++=floor(q*(((int32_t)*u.u8i.rp++)<<24)+0.5);

    break;
  case AV_SAMPLE_FMT_S16:
    u.s16i.rp=(const int16_t *)sr->frame->data[0];
    u.s16i.rp+=channels*sr->nb_samples.frame;

    while (wp<mp)
      *wp++=floor(q*(((int32_t)*u.s16i.rp++)<<16)+0.5);

    break;
  case AV_SAMPLE_FMT_S32:
    u.s32i.rp=(const int32_t *)sr->frame->data[0];
    u.s16i.rp+=channels*sr->nb_samples.frame;

    while (wp<mp)
      *wp++=floor(q*(*u.s32i.rp++)+0.5);

    break;
  case AV_SAMPLE_FMT_FLT:
    u.flti.rp=(const float *)sr->frame->data[0];
    u.flti.rp+=channels*sr->nb_samples.frame;

    while (wp<mp)
      //*wp++=floor((*u.flti.rp++)*INT32_MAX*q+0.5);
      *wp++=floor(q*((double)INT32_MAX)*(*u.flti.rp++)+0.5);

    break;
  case AV_SAMPLE_FMT_DBL:
    u.dbli.rp=(const double *)sr->frame->data[0];
    u.dbli.rp+=channels*sr->nb_samples.frame;

    while (wp<mp)
      //*wp++=floor((*u.dbli.rp++)*INT32_MAX*q+0.5);
      *wp++=floor(q*((double)INT32_MAX)*(*u.dbli.rp++)+0.5);

    break;
  /// planar ////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    for (ch=0;ch<channels;++ch) {
      u.u8p.rp[ch]=(const int8_t *)sr->frame->data[ch];
      u.u8p.rp[ch]+=channels*sr->nb_samples.frame;
    }

    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor(q*(((int32_t)*u.u8p.rp[ch]++)<<24)+0.5);
    }

    break;
  case AV_SAMPLE_FMT_S16P:
    for (ch=0;ch<channels;++ch) {
      u.s16p.rp[ch]=(const int16_t *)sr->frame->data[ch];
      u.s16p.rp[ch]+=channels*sr->nb_samples.frame;
    }

    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor(q*(((int32_t)*u.s16p.rp[ch]++)<<16)+0.5);
    }

    break;
  case AV_SAMPLE_FMT_S32P:
    for (ch=0;ch<channels;++ch) {
      u.s32p.rp[ch]=(const int32_t *)sr->frame->data[ch];
      u.s32p.rp[ch]+=channels*sr->nb_samples.frame;
    }

    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor(q*(*u.s32p.rp[ch]++)+0.5);
    }

    break;
  case AV_SAMPLE_FMT_FLTP:
    for (ch=0;ch<channels;++ch) {
      u.fltp.rp[ch]=(const float *)sr->frame->data[ch];
      u.fltp.rp[ch]+=channels*sr->nb_samples.frame;
    }

    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor((*u.fltp.rp[ch]++)*INT32_MAX*q+0.5);
    }

    break;
  case AV_SAMPLE_FMT_DBLP:
    for (ch=0;ch<channels;++ch) {
      u.dblp.rp[ch]=(const double *)sr->frame->data[ch];
      u.dblp.rp[ch]+=channels*sr->nb_samples.frame;
    }

    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor((*u.dblp.rp[ch]++)*INT32_MAX*q+0.5);
    }

    break;
#endif // }
  /////////////////////////////////////////////////////////////////////////////
  default:
    return -1;
  }

  // advance the read and write offsets
  sr->nb_samples.frame+=nb_samples;
  sw->nb_samples.frame+=nb_samples;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// read
s_t *bs1770gain_stream_read_create(s_t *sr, AVFormatContext *fc,
    int stream_index)
{
  BS1770GAIN_GOTO(NULL==bs1770gain_stream_create(sr,fc),
      "creating stream",stream);
  sr->vmt=bs1770gain_stream_read_vmt();

  ////////
  sr->stream_index=stream_index;
  sr->st=fc->streams[stream_index];
  sr->cc=sr->st->codec;
  sr->codec=sr->cc->codec;
  ////////

  return sr;
// cleanup:
  bs1770gain_stream_cleanup(sr);
stream:
  return NULL;
}

////////
static void bs1770gain_stream_read_cleanup(s_t *sr)
{
  bs1770gain_stream_cleanup(sr);
}

static int bs1770gain_stream_read_decode(s_t *sr, AVPacket *pkt2,
    int *got_packet)
{
  s_t *sw;

  // always success, the writer does the work.
  // pass the packet to the writer.
  if (NULL!=(sw=sr->link))
    sw->write.copy.pkt=pkt2;

  return *got_packet=pkt2->size;
}

const bs1770gain_stream_vmt_t *bs1770gain_stream_read_vmt(void)
{
  static bs1770gain_stream_vmt_t vmt;

  if (NULL==vmt.type) {
    vmt=*bs1770gain_stream_vmt();
    vmt.type="bs1770gain_stream_read";
    vmt.tag=BS1770GAIN_STREAM_READ;
    vmt.cleanup=bs1770gain_stream_read_cleanup;
    vmt.decode=bs1770gain_stream_read_decode;
  }

  return &vmt;
}

////////
int bs1770gain_stream_read_process(s_t *sr, AVPacket *pkt2, int *got_frame)
{
  s_t *sw;
  int size;

  for (;;) {
    // try to (continue to) decode the secondary packet or to flush
    // the decoder (with a NULL secondary packet), respectively.
    BS1770GAIN_GOTO((size=sr->vmt->decode(sr,pkt2,got_frame))<0,
        "decoding",decode);

    if (0!=*got_frame&&NULL!=(sw=sr->link))
      BS1770GAIN_GOTO(sw->vmt->encode(sw),"encoding",encode);

    if (0<pkt2->size) {
      size=FFMIN(size,pkt2->size);
      pkt2->size-=size;
      pkt2->data+=size;

      if (0==pkt2->size)
        break;
    }
    else if (0==*got_frame)
      break;
  }

  return 0;
encode:
decode:
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
// read/decode
s_t *bs1770gain_stream_decode_create(s_t *sr, AVFormatContext *fc,
    int stream_index, const bs1770gain_options_t *options)
{
  AVDictionary *opts=NULL;
  char drc[32];

  BS1770GAIN_GOTO(NULL==bs1770gain_stream_read_create(sr,fc,stream_index),
      "creating stream",stream);
  sr->vmt=bs1770gain_stream_decode_vmt();

  ////////
  sr->codec=avcodec_find_decoder(sr->cc->codec_id);
  BS1770GAIN_GOTO(NULL==sr->codec,"finding decoder",find);

#if 1 // {
  switch (sr->cc->codec_type) {
  case AVMEDIA_TYPE_AUDIO:
    // we want to have stereo.
    // TODO: should be an option.
    sr->cc->request_channel_layout=AV_CH_LAYOUT_STEREO;
    sr->cc->request_sample_fmt=AV_SAMPLE_FMT_FLT;

    if (AV_CODEC_ID_AC3==sr->cc->codec_id&&NULL!=options) {
      // avoid dynamic range compression.
      sprintf(drc,"%0.2f",options->drc);
      BS1770GAIN_GOTO(av_dict_set(&opts,"drc_scale",drc,0)!=0,
            "switching off dynamic range compression",opts);
    }

    break;
  default:
    break;
  }
#endif // }

  BS1770GAIN_GOTO(avcodec_open2(sr->cc,sr->codec,&opts)<0,
      "opening stream",open);
  ////////

  ////////
  sr->frame=av_frame_alloc();
  BS1770GAIN_GOTO(NULL==sr->frame,"allocating frame",frame);
  ////////

  return sr;
// cleanup:
  av_frame_free(&sr->frame);
frame:
  avcodec_close(sr->cc);
open:
  av_dict_free(&opts);
opts:
find:
  bs1770gain_stream_read_cleanup(sr);
stream:
  return NULL;
}

////////
static void bs1770gain_stream_decode_cleanup(s_t *sr)
{
  av_frame_free(&sr->frame);
  avcodec_close(sr->cc);
  bs1770gain_stream_read_cleanup(sr);
}

static int bs1770gain_stream_decode_decode(s_t *rs, AVPacket *pkt2,
    int *got_frame)
{
  return avcodec_decode_audio4(rs->cc,rs->frame,got_frame,pkt2);
}

const bs1770gain_stream_vmt_t *bs1770gain_stream_decode_vmt(void)
{
  static bs1770gain_stream_vmt_t vmt;

  if (NULL==vmt.type) {
    vmt=*bs1770gain_stream_vmt();
    vmt.type="bs1770gain_stream_decode";
    vmt.tag=BS1770GAIN_STREAM_DECODE;
    vmt.cleanup=bs1770gain_stream_decode_cleanup;
    vmt.decode=bs1770gain_stream_decode_decode;
  }

  return &vmt;
}

///////////////////////////////////////////////////////////////////////////////
// write
s_t *bs1770gain_stream_write_create(s_t *sw, AVFormatContext *fc,
    const s_t *sr)
{
  AVRational ar;

  BS1770GAIN_GOTO(NULL==bs1770gain_stream_create(sw,fc),
      "creating stream",stream);
  sw->vmt=bs1770gain_stream_write_vmt();

  //sw->stream_index=fc->nb_streams;
  sw->st=avformat_new_stream(sw->fc,sr->codec);
  BS1770GAIN_GOTO(NULL==sw->st,"allocating stream",alloc);
  sw->stream_index=sw->st->id;
  sw->cc=sw->st->codec;
  sw->codec=sw->cc->codec;
  BS1770GAIN_GOTO(avcodec_copy_context(sw->cc,sr->cc)<0,
      "copying stream",copy);

  switch (sr->cc->codec_type) {
  case AVMEDIA_TYPE_VIDEO:
    ar=sr->st->sample_aspect_ratio.num
        ?sr->st->sample_aspect_ratio
        :sr->cc->sample_aspect_ratio;
    sw->st->sample_aspect_ratio=ar;
    sw->cc->sample_aspect_ratio=ar;
    sw->st->avg_frame_rate=sr->st->avg_frame_rate;
    break;
  case AVMEDIA_TYPE_AUDIO:
    break;
  default:
    break;
  }

  //sw->cc->codec_tag=0;
  sw->st->time_base=sr->st->time_base;

  return sw;
// cleanup:
copy:
alloc:
  bs1770gain_stream_cleanup(sw);
stream:
  return NULL;
}

////////
static void bs1770gain_stream_write_cleanup(s_t *sw)
{
  bs1770gain_stream_cleanup(sw);
}

static int bs1770gain_stream_write_encode(s_t *sw)
{

  //s_t *sr=sw->link;
  AVPacket *pkt=sw->write.copy.pkt;

  pkt->stream_index=sw->stream_index;

  return av_interleaved_write_frame(sw->fc,pkt);
}

static int bs1770gain_stream_write_flush(s_t *s)
{
  return 0;
}

const bs1770gain_stream_vmt_t *bs1770gain_stream_write_vmt(void)
{
  static bs1770gain_stream_vmt_t vmt;

  if (NULL==vmt.type) {
    vmt=*bs1770gain_stream_vmt();
    vmt.type="bs1770gain_stream_write";
    vmt.tag=BS1770GAIN_STREAM_WRITE;
    vmt.cleanup=bs1770gain_stream_write_cleanup;
    vmt.encode=bs1770gain_stream_write_encode;
    vmt.flush=bs1770gain_stream_write_flush;
  }

  return &vmt;
}

///////////////////////////////////////////////////////////////////////////////
// encode/write
s_t *bs1770gain_stream_encode_create(s_t *sw, AVFormatContext *fc,
    const s_t *sr, int codec_type, int sample_fmt)
{
  AVCodec *codec;
  int nb_samples;

  BS1770GAIN_GOTO(NULL==bs1770gain_stream_create(sw,fc),
      "creating stream",base);
  sw->vmt=bs1770gain_stream_encode_vmt();

  ////////
  codec=avcodec_find_encoder(codec_type);
  BS1770GAIN_GOTO(NULL==codec,"finding encoder",codec);
  sw->st=avformat_new_stream(sw->fc,codec);
  BS1770GAIN_GOTO(NULL==sw->st,"allocation output stream",stream);
  sw->stream_index=sw->st->id;
  sw->cc=sw->st->codec;
  sw->cc->sample_fmt=sample_fmt;
  sw->cc->sample_rate=sr->cc->sample_rate;
  sw->cc->channel_layout=sr->cc->channel_layout;
  sw->cc->channels=sr->cc->channels;
  sw->st->time_base=(AVRational){1,sw->cc->sample_rate};
  BS1770GAIN_GOTO(avcodec_open2(sw->cc,codec,NULL)<0,
      "opening output stream",open);
  sw->codec=codec;
  ////////

  ////////
  sw->frame=av_frame_alloc();
  BS1770GAIN_GOTO(NULL==sw->frame,"allocating frame",frame);

  if (sw->codec->capabilities&CODEC_CAP_VARIABLE_FRAME_SIZE)
    nb_samples=10000;
  else
    nb_samples=sw->cc->frame_size;

  sw->frame->format=sw->cc->sample_fmt;
  sw->frame->channel_layout=sw->cc->channel_layout;
  sw->frame->channels=sw->cc->channels;
  sw->frame->sample_rate=sw->cc->sample_rate;
  sw->frame->nb_samples=nb_samples;

  BS1770GAIN_GOTO(0<nb_samples&&av_frame_get_buffer(sw->frame,0)<0,
      "allocating frame buffer",buffer);
  ////////

  av_init_packet(&sw->write.encode.pkt);

  return sw;
// cleanup:
buffer:
  av_frame_free(&sw->frame);
frame:
  avcodec_close(sw->cc);
open:
stream:
codec:
  bs1770gain_stream_cleanup(sw);
base:
  return NULL;
}

static void bs1770gain_stream_encode_cleanup(s_t *sw)
{
  av_frame_free(&sw->frame);
  avcodec_close(sw->cc);
  bs1770gain_stream_cleanup(sw);
}

////////
// frame may be NULL.
static int bs1770gain_stream_encode_write(s_t *sw, AVFrame *frame,
    int *got_packet)
{
  AVCodecContext *cc=sw->cc;
  AVRational q={1,cc->sample_rate};
  AVPacket *pkt=&sw->write.encode.pkt;

  pkt->size=0;
  pkt->data=NULL;

  // set the presentation time stamp calculated from the sample frequency
  // and the samples processed so far.
  if (NULL!=frame)
    frame->pts=av_rescale_q(sw->nb_samples.stream,q,cc->time_base);

  // encode the frame (may be NULL for flushing the encoder).
  if (avcodec_encode_audio2(sw->cc,pkt,frame,got_packet)<0)
    goto encode;

  // if we fot a packet ...
  if (0!=*got_packet) {
    pkt->stream_index=sw->stream_index;
    av_packet_rescale_ts(pkt,sw->cc->time_base,sw->st->time_base);

    // ... write it to the format.
    if (av_interleaved_write_frame(sw->fc,pkt)<0)
      goto write;
  }

  return 0;
encode:
write:
  return -1;
}

static int bs1770gain_stream_encode_encode(s_t *sw)
{
  s_t *sr=sw->link;
  int got_packet;

  for (;;) {
    // (continue to partially) convert the input frame to the output frame.
    // note: input frame and output frame are moving relatively to each other,
    // we convert the overlapping region.
    BS1770GAIN_GOTO(bs1770gain_stream_frame_convert(sr,sw)<0,
        "converting frame",frame);

    // if the output frame is completely filled ...
    if (bs1770gain_stream_frame_complete(sw)) {
      // ... encode it and write it to the output format.
      BS1770GAIN_GOTO(bs1770gain_stream_encode_write(sw,sw->frame,
          &got_packet)<0,"encoding/writing frame",encode);

      // increase the overall count, and
      sw->nb_samples.stream+=sw->nb_samples.frame;
      // reset the frame count.
      sw->nb_samples.frame=0;
    }

    // if all samples fome the input frame are processed ...
    if (bs1770gain_stream_frame_complete(sr)) {
      // increase the overall count,
      sr->nb_samples.stream+=sr->nb_samples.frame;
      // reset the frame count, and
      sr->nb_samples.frame=0;
      // break out from the loop.
      break;
    }
  }

  return 0;
encode:
frame:
  return -1;
}

static int bs1770gain_stream_encode_flush(s_t *sw)
{
  int got_packet;

  // if there are samples left in the output frame ...
  if (0<sw->nb_samples.frame) {
    // ... set the frame nb_count accordingly,and
    sw->frame->nb_samples=sw->nb_samples.frame;
    sw->nb_samples.frame=0;

    // encode and write these samples.
    if (bs1770gain_stream_encode_write(sw,sw->frame,&got_packet))
      goto encode;
  }

  // flush the encoder.
  do {
    if (bs1770gain_stream_encode_write(sw,NULL,&got_packet))
      goto encode;
  } while (got_packet);

  return 0;
encode:
  return -1;
}

const bs1770gain_stream_vmt_t *bs1770gain_stream_encode_vmt(void)
{
  static bs1770gain_stream_vmt_t vmt;

  if (NULL==vmt.type) {
    vmt=*bs1770gain_stream_vmt();
    vmt.type="bs1770gain_stream_encode";
    vmt.tag=BS1770GAIN_STREAM_ENCODE;
    vmt.cleanup=bs1770gain_stream_encode_cleanup;
    vmt.encode=bs1770gain_stream_encode_encode;
    vmt.flush=bs1770gain_stream_encode_flush;
  }

  return &vmt;
}
