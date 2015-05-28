/*
 * bs1770gain_read.c
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
bs1770gain_read_t *bs1770gain_read_new(const bs1770gain_options_t *options,
    const char *path, bs1770gain_stats_t *stats)
{
  bs1770gain_read_t *read;
  AVFormatContext *ifc;
  AVCodecContext *adc;
  AVDictionary *opts;
  AVCodec *ad;
  //int ai,vi;
  int ai=options->audio;
  sox_signalinfo_t *signal;
  char drc[32];

  /////////////////////////////////////////////////////////////////////////////
  opts=NULL;

  /////////////////////////////////////////////////////////////////////////////
  read=calloc(1,sizeof *read);
  BS1770GAIN_GOTO(NULL==read,"allocation reader",read);

  /////////////////////////////////////////////////////////////////////////////
  read->options=options;

  /////////////////////////////////////////////////////////////////////////////
  read->state=BS1770GAIN_READ_READ;
  read->ai=-1;
  read->clips=0;

  BS1770GAIN_GOTO(avformat_open_input(&read->ifc,path,NULL,NULL)<0,
      "opening format",ifc);
  ifc=read->ifc;
  ifc->flags|=AVFMT_FLAG_GENPTS;

  BS1770GAIN_GOTO(avformat_find_stream_info(ifc,NULL)<0,
      "finding stream info",find);
#if 0 // {
  BS1770GAIN_GOTO(bs1770gain_audiostream(ifc,&ai,&vi,options)<0,
      "finding audio",find);
#else // } {
  BS1770GAIN_GOTO(ffsox_audiostream(ifc,&ai,NULL)<0,"finding audio",find);
#endif // }
////
  read->ai=ai;

  read->adc=adc=ifc->streams[ai]->codec;
  //adc->request_channel_layout=AV_CH_LAYOUT_NATIVE;
  adc->request_channel_layout=AV_CH_LAYOUT_STEREO;
  adc->request_sample_fmt=AV_SAMPLE_FMT_FLT;

  BS1770GAIN_GOTO(NULL==(ad=ffsox_find_decoder(adc->codec_id)),
      "finding audio decoder",find);

  if (AV_CODEC_ID_AC3==adc->codec_id) {
    sprintf(drc,"%0.2f",options->drc);
    BS1770GAIN_GOTO(av_dict_set(&opts,"drc_scale",drc,0)!=0,
        "switching off dynamic range compression",opts);
  }

  BS1770GAIN_GOTO(0!=avcodec_open2(adc,ad,&opts),
      "opening audio",adc);
////

  BS1770GAIN_GOTO(NULL==(read->frame=av_frame_alloc()),
      "allocation frame",frame);

  av_init_packet(&read->p1);
  read->p1.data=NULL;
  read->p1.size=0;
  read->p2.stream_index=-1;

  /////////////////////////////////////////////////////////////////////////////
  read->head=bs1770gain_head_new(stats,adc->sample_rate,
      adc->channels,options);
  BS1770GAIN_GOTO(NULL==read->head,"allocating head",head);

  /////////////////////////////////////////////////////////////////////////////
  signal=&read->signal;
  // samples per second, 0 if unknown
  signal->rate=adc->sample_rate;
  // number of sound channels, 0 if unknown
  signal->channels=adc->channels;
  // bits per sample, 0 if unknown
  signal->precision=32;
  // samples * chans in file, 0 if unknown, -1 if unspecified
  signal->length=round(0.000001*ifc->duration*adc->sample_rate);
  signal->length*=adc->channels;
  // Effects headroom multiplier; may be null
  signal->mult=NULL;

  /////////////////////////////////////////////////////////////////////////////
  read->ts=bs1770gain_seek(ifc,options);
  BS1770GAIN_GOTO(read->ts<0,"seeking",seek);

  return read;
// cleanup:
seek:
  bs1770gain_head_close(read->head);
head:
  av_frame_free(&read->frame);
frame:
  avcodec_close(read->adc);
adc:
  av_dict_free(&opts);
opts:
find:
  avformat_close_input(&read->ifc);
ifc:
  free(read);
read:
  return NULL;
}

void bs1770gain_read_close(bs1770gain_read_t *read)
{
  bs1770gain_head_close(read->head);
  av_frame_free(&read->frame);
  avcodec_close(read->adc);
  avformat_close_input(&read->ifc);
  free(read);
}

size_t bs1770gain_read_decode(bs1770gain_read_t *read,
    sox_sample_t *obuf, size_t size)
{
  sox_sample_t *wp=obuf;
  sox_sample_t *mp=wp+size;
  size_t converted;

  while (wp<mp) {
    if (bs1770gain_read_eob(read)) {
      if (bs1770gain_read_decode_frame(read)<0)
        goto decode;
    }

    if (BS1770GAIN_READ_EOF==read->state)
      break;

    converted=bs1770gain_read_convert_frame(read,wp,size);
    wp+=converted;
    size-=converted;
  }
decode:
  return wp-obuf;
}

int bs1770gain_read_decode_frame(bs1770gain_read_t *read)
{
  AVFormatContext *ifc=read->ifc;
  AVCodecContext *adc=read->adc;
  AVFrame *frame=read->frame;
  AVPacket *p1=&read->p1;
  AVPacket *p2=&read->p2;
  int *ai=&read->ai;
  int *got_frame=&read->got_frame;
  AVStream *st;
  int64_t ts;
  int size;

  switch (read->state) {
  case BS1770GAIN_READ_READ:
    while (0==*got_frame) {
      if (0==p2->size) {
        if (0<p1->size) {
          av_free_packet(p1);
          p1->size=0;
        }

        if (av_read_frame(ifc,p1)<0) {
          read->state=BS1770GAIN_READ_FLUSH;
          goto flush;
        }

        if (read->ts>0) {
          st=ifc->streams[p1->stream_index];
          ts=av_rescale_q(read->ts,AV_TIME_BASE_Q,st->time_base);

          if (p1->dts!=AV_NOPTS_VALUE)
            p1->dts-=ts;

          if (p1->pts!=AV_NOPTS_VALUE)
            p1->pts-=ts;
        }

        if (bs1770gain_oor(p1,ifc,read->options)) {
          av_free_packet(p1);
          read->state=BS1770GAIN_READ_FLUSH;
          goto flush;
        }

        if (*ai==p1->stream_index)
          *p2=*p1;
      }
      else {
        if ((size=avcodec_decode_audio4(adc,frame,got_frame,p2))<0) {
          av_free_packet(p1);
          p1->size=0;
          goto error;
        }

        p2->size-=size;
        p2->data+=size;
      }
    }

    read->got_frame=0;
    bs1770gain_read_reset(read);
    break;
  case BS1770GAIN_READ_FLUSH:
  flush:
    p2->stream_index=*ai;
    p2->size=0;
    p2->data=NULL;

    if (avcodec_decode_audio4(adc,frame,got_frame,p2)<0) {
      read->state=BS1770GAIN_READ_EOF;
      goto error;
    }

    if (0==*got_frame) {
      read->state=BS1770GAIN_READ_EOF;
      goto done;
    }

    bs1770gain_read_reset(read);
    break;
  case BS1770GAIN_READ_EOF:
    goto error;
  default:
    goto error;
  }
done:
  return read->state;
error:
  return -1;
}

///////////////////////////////////////////////////////////////////////////////
int bs1770gain_read_eob(bs1770gain_read_t *read)
{
  switch (read->frame->format) {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    return read->s8i.mp==read->s8i.rp;
  case AV_SAMPLE_FMT_S16:
    return read->s16i.mp==read->s16i.rp;
  case AV_SAMPLE_FMT_S32:
    return read->s32i.mp==read->s32i.rp;
  case AV_SAMPLE_FMT_FLT:
    return read->flti.mp==read->flti.rp;
  case AV_SAMPLE_FMT_DBL:
    return read->dbli.mp==read->dbli.rp;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    return read->s8p.mp==read->s8p.rp[0];
  case AV_SAMPLE_FMT_S16P:
    return read->s16p.mp==read->s16p.rp[0];
  case AV_SAMPLE_FMT_S32P:
    return read->s32p.mp==read->s32p.rp[0];
  case AV_SAMPLE_FMT_FLTP:
    return read->fltp.mp==read->fltp.rp[0];
  case AV_SAMPLE_FMT_DBLP:
    return read->dblp.mp==read->dblp.rp[0];
  /////////////////////////////////////////////////////////////////////////////
  default:
    return 1;
  }
}

///////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_READ_RESET_I(t,read) do { \
  AVFrame *frame=(read)->frame; \
  int nb_samples=frame->nb_samples; \
  int channels=frame->channels; \
  (read)->t##i.rp=(bs1770gain_##t##_t *)frame->data[0]; \
  (read)->t##i.mp=(read)->t##i.rp+channels*nb_samples; \
} while (0)

#define BS1770GAIN_READ_RESET_P(t,read) do { \
  AVFrame *frame=(read)->frame; \
  int nb_samples=frame->nb_samples; \
  int channels=frame->channels; \
  bs1770gain_##t##_t **rpp=(read)->t##p.rp; \
  bs1770gain_##t##_t **dpp=(bs1770gain_##t##_t **)frame->data; \
  int ch; \
 \
  for (ch=0;ch<channels;++ch) \
    *rpp++=*dpp++; \
 \
  (read)->t##p.mp=(read)->t##p.rp[0]+nb_samples; \
} while (0)

void bs1770gain_read_reset(bs1770gain_read_t *read)
{
  switch (read->frame->format) {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    BS1770GAIN_READ_RESET_I(s8,read);
    break;
  case AV_SAMPLE_FMT_S16:
    BS1770GAIN_READ_RESET_I(s16,read);
    break;
  case AV_SAMPLE_FMT_S32:
    BS1770GAIN_READ_RESET_I(s32,read);
    break;
  case AV_SAMPLE_FMT_FLT:
    BS1770GAIN_READ_RESET_I(flt,read);
    break;
  case AV_SAMPLE_FMT_DBL:
    BS1770GAIN_READ_RESET_I(dbl,read);
    break;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    BS1770GAIN_READ_RESET_P(s8,read);
    break;
  case AV_SAMPLE_FMT_S16P:
    BS1770GAIN_READ_RESET_P(s16,read);
    break;
  case AV_SAMPLE_FMT_S32P:
    BS1770GAIN_READ_RESET_P(s32,read);
    break;
  case AV_SAMPLE_FMT_FLTP:
    BS1770GAIN_READ_RESET_P(flt,read);
    break;
  case AV_SAMPLE_FMT_DBLP:
    BS1770GAIN_READ_RESET_P(dbl,read);
    break;
  /////////////////////////////////////////////////////////////////////////////
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_BASS 3

#define BS1770GAIN_CHANNEL_IN_RANGE(ch,channels) ( \
  (channels)<=LIB1770_MAX_CHANNELS \
  ?(ch)<LIB1770_MAX_CHANNELS \
  :((ch)<BS1770GAIN_BASS||(BS1770GAIN_BASS<(ch)&&(ch)<=LIB1770_MAX_CHANNELS)) \
)

#define bs1770_normalize_dbl_s8(x) \
  ((double)(x)/INT8_MAX)
#define bs1770_normalize_dbl_s16(x) \
  ((double)(x)/INT16_MAX)
#define bs1770_normalize_dbl_s32(x) \
  ((double)(x)/INT32_MAX)
#define bs1770_normalize_dbl_flt(x) \
  (x)
#define bs1770_normalize_dbl_dbl(x) \
  (x)

#define bs1770_normalize_max_s8(x) \
  ((double)((x)<0?-((x)+1):(x))/INT8_MAX)
#define bs1770_normalize_max_s16(x) \
  ((double)((x)<0?-((x)+1):(x))/INT16_MAX)
#define bs1770_normalize_max_s32(x) \
  ((double)((x)<0?-((x)+1):(x))/INT32_MAX)
#define bs1770_normalize_max_flt(x) \
  ((x)<0?-(x):(x))
#define bs1770_normalize_max_dbl(x) \
  ((x)<0?-(x):(x))

#define bs1770_normalize_tp_pre_s8(x)
#define bs1770_normalize_tp_pre_s16(x)
#define bs1770_normalize_tp_pre_s32(x)
#define bs1770_normalize_tp_pre_flt(x) \
  ((x)*=0.25)
#define bs1770_normalize_tp_pre_dbl(x) \
  ((x)*=0.25)

#define bs1770_normalize_tp_post_s8(wp) \
  (*(wp)>>=2)
#define bs1770_normalize_tp_post_s16(wp) \
  (*(wp)>>=2)
#define bs1770_normalize_tp_post_s32(wp) \
  (*(wp)>>=2)
#define bs1770_normalize_tp_post_flt(wp)
#define bs1770_normalize_tp_post_dbl(wp)

#define bs1770_normalize_sox_s8(x,clips) \
  (SOX_SIGNED_8BIT_TO_SAMPLE(x,clips))
#define bs1770_normalize_sox_s16(x,clips) \
  (SOX_SIGNED_16BIT_TO_SAMPLE(x,clips))
#define bs1770_normalize_sox_s32(x,clips) \
  (SOX_SIGNED_32BIT_TO_SAMPLE(x,clips))
#define bs1770_normalize_sox_flt(x,clips) \
  (SOX_FLOAT_32BIT_TO_SAMPLE(x,clips))
#define bs1770_normalize_sox_dbl(x,clips) \
  (SOX_FLOAT_64BIT_TO_SAMPLE(x,clips))

#define BS1770GAIN_READ_RESET_I(t,read) do { \
  AVFrame *frame=(read)->frame; \
  int nb_samples=frame->nb_samples; \
  int channels=frame->channels; \
  (read)->t##i.rp=(bs1770gain_##t##_t *)frame->data[0]; \
  (read)->t##i.mp=(read)->t##i.rp+channels*nb_samples; \
} while (0)

#define BS1770GAIN_READ_RESET_P(t,read) do { \
  AVFrame *frame=(read)->frame; \
  int nb_samples=frame->nb_samples; \
  int channels=frame->channels; \
  bs1770gain_##t##_t **rpp=(read)->t##p.rp; \
  bs1770gain_##t##_t **dpp=(bs1770gain_##t##_t **)frame->data; \
  int ch; \
 \
  for (ch=0;ch<channels;++ch) \
    *rpp++=*dpp++; \
 \
  (read)->t##p.mp=(read)->t##p.rp[0]+nb_samples; \
} while (0)

#define BS1770GAIN_READ_CONVERT_I(t,read,wp,wmp) do { \
  int mono2stereo=(read)->options->mono2stereo; \
  bs1770gain_head_t *head=(read)->head; \
  lib1770_pre_t *pre=NULL==head?NULL:head->pre; \
  bs1770gain_stats_t *stats=NULL==head?NULL:head->stats; \
  double peak_s=NULL==stats?-1.0:stats->peak_s; \
  int channels=(read)->frame->channels; \
  bs1770gain_##t##_t *rp=(read)->t##i.rp; \
  bs1770gain_##t##_t *rmp=(read)->t##i.mp; \
  bs1770gain_##t##_t x; \
  double y; \
  sox_uint64_t clips=(read)->clips; \
  lib1770_sample_t sample; \
  double *sp; \
  int ch; \
  SOX_SAMPLE_LOCALS; \
 \
  while (rp<rmp&&(wp)<(wmp)) { \
    sp=sample; \
 \
    for (ch=0;ch<channels;++ch) { \
      x=*rp; \
 \
      if (0<=peak_s&&peak_s<(y=bs1770_normalize_max_##t(x))) \
        peak_s=y; \
 \
      if (BS1770GAIN_CHANNEL_IN_RANGE(ch,channels)) { \
        *sp++=bs1770_normalize_dbl_##t(x); \
 \
        if (1==channels&&mono2stereo) \
          *sp++=bs1770_normalize_dbl_##t(x); \
      } \
 \
      bs1770_normalize_tp_pre_##t(x); \
      *wp=bs1770_normalize_sox_##t(x,clips); \
      bs1770_normalize_tp_post_##t(wp); \
      ++wp; \
      ++rp; \
    } \
 \
    if (NULL!=pre) \
      lib1770_pre_add_sample(pre,sample); \
  } \
 \
  (read)->clips=clips; \
  (read)->t##i.rp=rp; \
 \
  if (0.0<=peak_s) \
    stats->peak_s=peak_s; \
} while (0)

#define BS1770GAIN_READ_CONVERT_P(t,read,wp,wmp) do { \
  int mono2stereo=(read)->options->mono2stereo; \
  bs1770gain_head_t *head=(read)->head; \
  lib1770_pre_t *pre=NULL==head?NULL:head->pre; \
  bs1770gain_stats_t *stats=NULL==head?NULL:head->stats; \
  double peak_s=NULL==stats?-1.0:stats->peak_s; \
  int channels=(read)->frame->channels; \
  bs1770gain_##t##_t **rp=(read)->t##p.rp; \
  bs1770gain_##t##_t **pp; \
  bs1770gain_##t##_t *rmp=(read)->t##p.mp; \
  bs1770gain_##t##_t x; \
  double y; \
  sox_uint64_t clips=(read)->clips; \
  int ch; \
  lib1770_sample_t sample; \
  double *sp; \
  SOX_SAMPLE_LOCALS; \
 \
  while (rp[0]<rmp&&(wp)<(wmp)) { \
    pp=rp; \
    sp=sample; \
 \
    for (ch=0;ch<channels;++ch) { \
      x=**pp; \
 \
      if (0<=peak_s&&peak_s<(y=bs1770_normalize_max_##t(x))) \
        peak_s=y; \
 \
      if (BS1770GAIN_CHANNEL_IN_RANGE(ch,channels)) { \
        *sp++=bs1770_normalize_dbl_##t(x); \
  \
        if (1==channels&&mono2stereo) \
          *sp++=bs1770_normalize_dbl_##t(x); \
      } \
 \
      bs1770_normalize_tp_pre_##t(x); \
      *wp=bs1770_normalize_sox_##t(x,clips); \
      bs1770_normalize_tp_post_##t(wp); \
      ++wp; \
      ++*pp; \
      ++pp; \
    } \
 \
    if (NULL!=pre) \
      lib1770_pre_add_sample(pre,sample); \
  } \
 \
  (read)->clips=clips; \
 \
  if (0.0<=peak_s) \
    stats->peak_s=peak_s; \
} while (0)

size_t bs1770gain_read_convert_frame(bs1770gain_read_t *read,
    sox_sample_t *obuf, size_t size)
{
  sox_sample_t *wp=obuf;
  sox_sample_t *mp=wp+size;

  switch (read->frame->format) {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    BS1770GAIN_READ_CONVERT_I(s8,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_S16:
    BS1770GAIN_READ_CONVERT_I(s16,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_S32:
    BS1770GAIN_READ_CONVERT_I(s32,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_FLT:
    BS1770GAIN_READ_CONVERT_I(flt,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_DBL:
    BS1770GAIN_READ_CONVERT_I(dbl,read,wp,mp);
    break;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    BS1770GAIN_READ_CONVERT_P(s8,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_S16P:
    BS1770GAIN_READ_CONVERT_P(s16,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_S32P:
    BS1770GAIN_READ_CONVERT_P(s32,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_FLTP:
    BS1770GAIN_READ_CONVERT_P(flt,read,wp,mp);
    break;
  case AV_SAMPLE_FMT_DBLP:
    BS1770GAIN_READ_CONVERT_P(dbl,read,wp,mp);
    break;
  /////////////////////////////////////////////////////////////////////////////
  default:
    break;
  }

  return wp-obuf;
}

///////////////////////////////////////////////////////////////////////////////
static int bs1770gain_read_getopts(sox_effect_t *effp, int argc, char *argv[])
{
  bs1770gain_read_t **read=(bs1770gain_read_t **)effp->priv;

  BS1770GAIN_GOTO(argc<2,"missing argument",argc);
  *read=(bs1770gain_read_t *)argv[1];

  return SOX_SUCCESS;
argc:
  return SOX_EOF;
}

static int bs1770gain_read_drain(sox_effect_t *effp, sox_sample_t *obuf,
    size_t *osamp)
{
  bs1770gain_read_t *read=*(bs1770gain_read_t **)effp->priv;

  // ensure that *osamp is a multiple of the number of channels.
  *osamp-=*osamp%effp->out_signal.channels;

  // Read up to *osamp samples into obuf; store the actual number read
  // back to *osamp.
  *osamp=bs1770gain_read_decode(read,obuf,*osamp);

  // if 0 samples is returned it indicates that end-of-file has been
  // reached or an error has occurred.
  return 0<*osamp?SOX_SUCCESS:SOX_EOF;
}

sox_effect_handler_t const *bs1770gain_read_handler(void)
{
  static sox_effect_handler_t handler={
    // Effect name
    .name="bs1770gain FFmpeg input",
    // Short explanation of parameters accepted by effect
    .usage=NULL,
    // Combination of SOX_EFF_* flags
    .flags=SOX_EFF_MCHAN,
    // Called to parse command-line arguments (called once per effect).
    .getopts=bs1770gain_read_getopts,
    // Called to initialize effect (called once per flow).
    .start=NULL,
    // Called to process samples.
    .flow=NULL,
    // Called to finish getting output after input is complete.
    .drain=bs1770gain_read_drain,
    // Called to shut down effect (called once per flow).
    .stop=NULL,
    // Called to shut down effect (called once per effect).
    .kill=NULL,
    // Size of private data SoX should pre-allocate for effect
    .priv_size=sizeof (bs1770gain_read_t *)
  };

  return &handler;
}
