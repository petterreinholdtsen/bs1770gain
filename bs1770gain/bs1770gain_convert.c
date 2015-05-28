/*
 * bs1770gain_convert.c
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
#include <bs1770gain.h>

///////////////////////////////////////////////////////////////////////////////
bs1770gain_convert_t *bs1770gain_convert_new(AVFormatContext *ifc, int iai,
    AVFormatContext *ofc, int oai, const bs1770gain_options_t *options,
    const bs1770gain_stats_t *track, const bs1770gain_stats_t *album)
{
  bs1770gain_convert_t *convert;
  double q;

  convert=malloc(sizeof *convert);
  BS1770GAIN_GOTO(NULL==convert,"allocation converter",convert);

  // compute the amplification/attenuation factor.
  q=options->preamp+options->level;
#if 0 // {
  q-=(1.0-options->apply)*lib1770_stats_get_mean(album->stats_im,-10.0);
  q-=options->apply*lib1770_stats_get_mean(track->stats_im,-10.0);
#else // } {
  q-=(1.0-options->apply)*bs1770gain_stats_get_loudness(album,options);
  q-=options->apply*bs1770gain_stats_get_loudness(track,options);
#endif // }
  q=LIB1770_DB2Q(q);
  convert->q=q;

  // from where we are fed.
  convert->i.fc=ifc;
  convert->i.ai=iai;
  convert->i.frame=av_frame_alloc();
  BS1770GAIN_GOTO(NULL==convert->i.frame,"allocation input frame",iframe);

  // where we write to.
  convert->o.fc=ofc;
  convert->o.ai=oai;
  convert->o.frame=av_frame_alloc();
  BS1770GAIN_GOTO(NULL==convert->o.frame,"allocation output frame",oframe);

  return convert;
// cleanup:
  av_frame_free(&convert->o.frame);
oframe:
  av_frame_free(&convert->i.frame);
iframe:
convert:
  return NULL;
}

void bs1770gain_convert_close(bs1770gain_convert_t *convert)
{
  av_frame_free(&convert->o.frame);
  av_frame_free(&convert->i.frame);
  free(convert);
}

///////////////////////////////////////////////////////////////////////////////
static int bs1770gain_convert_encode_write(bs1770gain_convert_t *convert)
{
  int code=-1;
  AVFormatContext *ofc=convert->o.fc;
  AVFrame *oframe=convert->o.frame;
  int stream_index=convert->o.ai;
  AVCodecContext *occ;
  AVPacket p;
  int got_packet;

  occ=ofc->streams[stream_index]->codec;
  oframe->pts=av_frame_get_best_effort_timestamp(oframe);
  got_packet=0;

  av_init_packet(&p);
  p.data=NULL;
  p.size=0;

  if (avcodec_encode_audio2(occ,&p,oframe,&got_packet)<0)
    goto encode;

  if (got_packet) {
    p.stream_index=stream_index;

    if (av_interleaved_write_frame(ofc,&p)<0)
      goto write;
  }

  code=0;
// cleanup.
write:
encode:
  av_free_packet(&p);

  return code;
}

static int bs1770gain_convert_frame(bs1770gain_convert_t *convert)
{
  AVFrame *iframe=convert->i.frame;
  AVFrame *oframe=convert->o.frame;
  double q=convert->q;
  int code,ch;
  int format,nb_samples,channels,channel_layout,sample_rate;
  int64_t best_effort_timestamp;
  int32_t *wp,*mp;

  union {
    // interleaved.
    struct {
      int8_t const *rp;
    } u8i;
    struct {
      int16_t const *rp;
    } s16i;
    struct {
      int32_t const *rp;
    } s32i;
    struct {
      float const *rp;
    } flti;
    struct {
      double const *rp;
    } dbli;
    // planar.
    struct {
      int8_t const *rp[AV_NUM_DATA_POINTERS];
    } u8p;
    struct {
      int16_t const *rp[AV_NUM_DATA_POINTERS];
    } s16p;
    struct {
      int32_t const *rp[AV_NUM_DATA_POINTERS];
    } s32p;
    struct {
      float const *rp[AV_NUM_DATA_POINTERS];
    } fltp;
    struct {
      double const *rp[AV_NUM_DATA_POINTERS];
    } dblp;
  } u;

  code=-1;

  format=BS1770GAIN_SAMPLE_FMT;
  nb_samples=iframe->nb_samples;
  channels=av_frame_get_channels(iframe);
  channel_layout=av_frame_get_channel_layout(iframe);
  sample_rate=av_frame_get_sample_rate(iframe);
  best_effort_timestamp=av_frame_get_best_effort_timestamp(iframe);

  if (format!=oframe->format
      ||nb_samples!=oframe->nb_samples
      ||channels!=oframe->channels) {

    if (NULL!=*oframe->data)
      av_freep(oframe->data);

    BS1770GAIN_GOTO(av_samples_alloc(oframe->data,NULL,
        channels,nb_samples,format,0)<0,"allocating samples",alloc);
    oframe->format=format;
    oframe->nb_samples=nb_samples;
    av_frame_set_channels(oframe,channels);
  }

  av_frame_set_channel_layout(oframe,channel_layout);
  av_frame_set_sample_rate(oframe,sample_rate);
  av_frame_set_best_effort_timestamp(oframe,best_effort_timestamp);

  wp=(int32_t *)oframe->data[0];
  mp=wp+channels*nb_samples;

  switch (iframe->format) {
    /// interleaved ///////////////////////////////////////////////////////////
    case AV_SAMPLE_FMT_U8:
      u.u8i.rp=(int8_t *)iframe->data[0];

      while (wp<mp)
        *wp++=floor(q*(((int32_t)*u.u8i.rp++)<<24)+0.5);

      break;
    case AV_SAMPLE_FMT_S16:
      u.s16i.rp=(int16_t *)iframe->data[0];

      while (wp<mp)
        *wp++=floor(q*(((int32_t)*u.s16i.rp++)<<16)+0.5);

      break;
    case AV_SAMPLE_FMT_S32:
      u.s32i.rp=(int32_t *)iframe->data[0];

      while (wp<mp)
        *wp++=floor(q*(*u.s32i.rp++)+0.5);

      break;
    case AV_SAMPLE_FMT_FLT:
      u.flti.rp=(float *)iframe->data[0];

      while (wp<mp)
        //*wp++=floor((*u.flti.rp++)*INT32_MAX*q+0.5);
        *wp++=floor(q*((double)INT32_MAX)*(*u.flti.rp++)+0.5);

      break;
    case AV_SAMPLE_FMT_DBL:
      u.dbli.rp=(double *)iframe->data[0];

      while (wp<mp)
        //*wp++=floor((*u.dbli.rp++)*INT32_MAX*q+0.5);
        *wp++=floor(q*((double)INT32_MAX)*(*u.dbli.rp++)+0.5);

      break;
    /// planar ////////////////////////////////////////////////////////////////
    case AV_SAMPLE_FMT_U8P:
      for (ch=0;ch<channels;++ch)
        u.u8p.rp[ch]=(int8_t *)iframe->data[ch];

      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor(q*(((int32_t)*u.u8p.rp[ch]++)<<24)+0.5);
      }

      break;
    case AV_SAMPLE_FMT_S16P:
      for (ch=0;ch<channels;++ch)
        u.s16p.rp[ch]=(int16_t *)iframe->data[ch];

      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor(q*(((int32_t)*u.s16p.rp[ch]++)<<16)+0.5);
      }

      break;
    case AV_SAMPLE_FMT_S32P:
      for (ch=0;ch<channels;++ch)
        u.s32p.rp[ch]=(int32_t *)iframe->data[ch];

      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor(q*(*u.s32p.rp[ch]++)+0.5);
      }

      break;
    case AV_SAMPLE_FMT_FLTP:
      for (ch=0;ch<channels;++ch)
        u.fltp.rp[ch]=(float *)iframe->data[ch];

      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor((*u.fltp.rp[ch]++)*INT32_MAX*q+0.5);
          //*wp++=floor((*u.fltp.rp[ch]++)*INT32_MAX*q+0.5);
      }

      break;
    case AV_SAMPLE_FMT_DBLP:
      for (ch=0;ch<channels;++ch)
        u.dblp.rp[ch]=(double *)iframe->data[ch];

      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor(q*((double)INT32_MAX)*(*u.dblp.rp[ch]++)+0.5);
      }

      break;
    ///////////////////////////////////////////////////////////////////////////
    default:
      break;
  }

  code=0;
alloc:
  return code;
}

int bs1770gain_convert_packet(bs1770gain_convert_t *convert, int *got_frame,
    AVPacket *p)
{
  int size=0;
  AVFormatContext *ifc=convert->i.fc;
  AVCodecContext *icc=ifc->streams[convert->i.ai]->codec;
  AVFrame *iframe=convert->i.frame;

  *got_frame=0;

  BS1770GAIN_GOTO((size=avcodec_decode_audio4(icc,iframe,got_frame,p))<0,
      "decoding audio",size);

  if (*got_frame) {
    BS1770GAIN_GOTO(bs1770gain_convert_frame(convert)<0,
        "converting audio frame",frame);
    BS1770GAIN_GOTO(bs1770gain_convert_encode_write(convert)<0,
        "writing audio frame",frame);
  }
frame:
size:
  return size;
}
