/*
 * ffmpeg1770.c
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
#include <lib1770.h>
#include <libavformat/avformat.h>

#define FFMPEG_BASS 3

#define FFMPEG_CHANNEL_IN_RANGE(ch,channels) ( \
  (channels)<=LIB1770_MAX_CHANNELS \
  ?(ch)<LIB1770_MAX_CHANNELS \
  :((ch)<FFMPEG_BASS||(FFMPEG_BASS<(ch)&&(ch)<=LIB1770_MAX_CHANNELS)) \
)

#define normalize_s8(x) \
  ((double)(x)/INT8_MAX)
#define normalize_s16(x) \
  ((double)(x)/INT16_MAX)
#define normalize_s32(x) \
  ((double)(x)/INT32_MAX)
#define normalize_flt(x) \
  (x)
#define normalize_dbl(x) \
  (x)

#define DECODE_STRUCT_I(T,t) \
struct { \
  T *rp,*mp; \
} t##i

#define DECODE_STRUCT_P(T,t) \
struct { \
  T *rp[LIB1770_MAX_CHANNELS],*mp,**pp; \
} t##p

#define DECODE_CODE_I(T,t) do { \
  u.t##i.rp=(T *)frame->data[0]; \
  u.t##i.mp=u.t##i.rp+channels*frame->nb_samples; \
 \
  while (u.t##i.rp<u.t##i.mp) { \
    for (ch=0,sp=sample;ch<channels;++ch) { \
      if (FFMPEG_CHANNEL_IN_RANGE(ch,channels)) \
        *sp++=normalize_##t(*u.t##i.rp++); \
      else \
        ++u.t##i.rp; \
    } \
 \
    lib1770_pre_add_sample(pre,sample); \
  } \
} while (0)

#define DECODE_CODE_P(T,t) do { \
  for (ch=0,u.t##p.pp=u.t##p.rp,dpp=frame->data;ch<channels;++ch) \
    if (FFMPEG_CHANNEL_IN_RANGE(ch,channels)) \
      *u.t##p.pp++=*(T **)dpp++; \
 \
  u.t##p.mp=u.t##p.rp[0]+frame->nb_samples; \
 \
  while (u.t##p.rp[0]<u.t##p.mp) { \
    for (ch=0,sp=sample,u.t##p.pp=u.t##p.rp;ch<channels;++ch) { \
      if (FFMPEG_CHANNEL_IN_RANGE(ch,channels)) { \
        *sp++=normalize_##t(**u.t##p.pp); \
        ++*u.t##p.pp; \
        ++u.t##p.pp; \
      } \
    } \
 \
    lib1770_pre_add_sample(pre,sample); \
  } \
} while (0)

int decode(AVCodecContext *codec, AVFrame *frame, int *got_frame, AVPacket *p,
    lib1770_pre_t *pre)
{
  union {
    DECODE_STRUCT_I(int8_t,s8);
    DECODE_STRUCT_I(int16_t,s16);
    DECODE_STRUCT_I(int32_t,s32);
    DECODE_STRUCT_I(float,flt);
    DECODE_STRUCT_I(double,dbl);

    DECODE_STRUCT_P(int8_t,s8);
    DECODE_STRUCT_P(int16_t,s16);
    DECODE_STRUCT_P(int32_t,s32);
    DECODE_STRUCT_P(float,flt);
    DECODE_STRUCT_P(double,dbl);
  } u;

  lib1770_sample_t sample;
  double *sp;
  uint8_t **dpp;
  int channels,ch;
  int decoded;

  *got_frame=0;

  switch (codec->codec_type) {
  case AVMEDIA_TYPE_AUDIO:
    if ((decoded=avcodec_decode_audio4(codec,frame,got_frame,p))<0)
      return decoded;

    decoded=FFMIN(decoded,p->size);

    if (*got_frame) {
      channels=frame->channels;

      switch (frame->format) {
      /// interleaved /////////////////////////////////////////////////////////
      case AV_SAMPLE_FMT_U8:
        DECODE_CODE_I(int8_t,s8);
        break;
      case AV_SAMPLE_FMT_S16:
        DECODE_CODE_I(int16_t,s16);
        break;
      case AV_SAMPLE_FMT_S32:
        DECODE_CODE_I(int32_t,s32);
        break;
      case AV_SAMPLE_FMT_FLT:
        DECODE_CODE_I(float,flt);
        break;
      case AV_SAMPLE_FMT_DBL:
        DECODE_CODE_I(double,dbl);
        break;
      /// planar //////////////////////////////////////////////////////////////
      case AV_SAMPLE_FMT_U8P:
        DECODE_CODE_P(int8_t,s8);
        break;
      case AV_SAMPLE_FMT_S16P:
        DECODE_CODE_P(int16_t,s16);
        break;
      case AV_SAMPLE_FMT_S32P:
        DECODE_CODE_P(int32_t,s32);
        break;
      case AV_SAMPLE_FMT_FLTP:
        DECODE_CODE_P(float,flt);
        break;
      case AV_SAMPLE_FMT_DBLP:
        DECODE_CODE_P(double,dbl);
        break;
      /////////////////////////////////////////////////////////////////////////
      default:
        break;
      }
    }

    break;
  case AVMEDIA_TYPE_VIDEO:
    if ((decoded=avcodec_decode_video2(codec,frame,got_frame,p))<0)
      return decoded;

    break;
  default:
    decoded=p->size;
    break;
  }

  p->data+=decoded;
  p->size-=decoded;

  return decoded;
}

char *basename(char *path)
{
  char *p=path+strlen(path);

  while (path<p&&'/'!=p[-1]&&'\\'!=p[-1])
    --p;

  return p;
}

void usage(char **argv)
{
  fprintf(stderr,"usage: %s <wav> [<wav> ...]\n",basename(argv[0]));
  exit(1);
}

int main(int argc, char **argv)
{
  AVFormatContext *ic;
  AVStream *st;
  AVCodecContext *adc;
  AVCodec *ad;
  AVFrame *frame;
  AVPacket p1,p2;
  int i,j,ai,got_frame;
  lib1770_stats_t *stats_im,*stats_rs;
  lib1770_block_t *block_04,*block_30;
  lib1770_pre_t *pre;
  clock_t t1,t2;

  // if there are not suficcient arguments exit.
  if (argc<2)
    usage(argv);

  av_register_all();
  t1=clock();

  // for each file ...
  for (i=1;i<argc;++i) {
    ///////////////////////////////////////////////////////////////////////////
    ic=NULL;
    ai=-1;

    // print out the file name.
    fprintf(stdout,"\"%s\" ...",basename(argv[i]));
    fflush(stdout);

    ///////////////////////////////////////////////////////////////////////////
    // open ffmpeg.
    if (avformat_open_input(&ic,argv[i],NULL,NULL)<0)
      goto ic;

    if (avformat_find_stream_info(ic,NULL)<0)
      goto find;

    for (j=0;j<ic->nb_streams;++j) {
      switch ((st=ic->streams[j])->codec->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
          if (ai<0||2==st->codec->channels)
            ai=j;
          break;
        default:
          break;
      }
    };

    if (ai<0)
      goto find;

    adc=ic->streams[ai]->codec;
    //adc->request_channel_layout=AV_CH_LAYOUT_NATIVE;
    //adc->request_channel_layout=AV_CH_LAYOUT_STEREO;
    adc->request_sample_fmt=AV_SAMPLE_FMT_FLT;

    if (NULL==(ad=avcodec_find_decoder(adc->codec_id)))
      goto find;

    if (0!=avcodec_open2(adc,ad,NULL))
      goto ast;

    if (NULL==(frame=av_frame_alloc()))
      goto frame;

    av_init_packet(&p1);
    p1.data=NULL;
    p1.size=0;
    p2.stream_index=-1;

    ///////////////////////////////////////////////////////////////////////////
    // open a history statistics for integrated loudness.
    // open a maximum statistics for momentary loudness.
    if (NULL==(stats_im=lib1770_stats_new()))
      goto stats_im;

    // open a 0.4 s / 0.75 overlap block for integrated and momentary loudness.
    if (NULL==(block_04=lib1770_block_new(adc->sample_rate,400.0,4)))
      goto block_04;

    // add the integrated and the momentary statistics to the respective block.
    lib1770_block_add_stats(block_04,stats_im);

    ///////////////////////////////////////////////////////////////////////////
    // open a history statistics for loudness range.
    // open a maximum statistics for short term loudness.
    if (NULL==(stats_rs=lib1770_stats_new()))
      goto stats_rs;

    // open a 3 ms / 0.66 overlap block for loudness range and short term
    // loudness.
    if (NULL==(block_30=lib1770_block_new(adc->sample_rate,3000.0,3)))
      goto block_30;

    // add the loudness range and the sort term statistics to the respective
    // block.
    lib1770_block_add_stats(block_30,stats_rs);

    ///////////////////////////////////////////////////////////////////////////
    // open a pre-filter.
    if (NULL==(pre=lib1770_pre_new(adc->sample_rate,adc->channels)))
      goto pre;

    // add the 0.4 s / 0.75 overlap and the 3 ms / 0.66 overlap block to the
    // pre-filter.
    lib1770_pre_add_block(pre,block_04);
    lib1770_pre_add_block(pre,block_30);

    ///////////////////////////////////////////////////////////////////////////
    // decoding.
    while (0<=av_read_frame(ic,&p1)) {
      if (ai==p1.stream_index) {
        p2=p1;

        do {
          if (decode(adc,frame,&got_frame,&p2,pre)<0)
            goto decode;
        } while (0<p2.size);
      }

      av_free_packet(&p1);
    }

    p2.data=NULL;
    p2.size=0;

    if (ai==p1.stream_index) {
      do {
        if (decode(adc,frame,&got_frame,&p2,pre)<0)
          goto decode;
      } while (got_frame);
    }

    lib1770_pre_flush(pre);

    ///////////////////////////////////////////////////////////////////////////
    // print out the results.
    fprintf(stdout,"\b\b\b   \n");
    fprintf(stdout,"  M %.1f, S %.1f, I %.1f, R %.1f\n",
        lib1770_stats_get_max(stats_im),
        lib1770_stats_get_max(stats_rs),
        lib1770_stats_get_mean(stats_im,-10.0),
        lib1770_stats_get_range(stats_rs,-20.0,0.1,0.95));

    ///////////////////////////////////////////////////////////////////////////
    // cleanup.
  decode:
    lib1770_pre_close(pre);
  pre:
    lib1770_block_close(block_30);
  block_30:
    lib1770_stats_close(stats_rs);
  stats_rs:
    lib1770_block_close(block_04);
  block_04:
    lib1770_stats_close(stats_im);
  stats_im:
    av_frame_free(&frame);
  frame:
    avcodec_close(adc);
  ast:
  find:
    avformat_close_input(&ic);
  ic:
    continue;
  }

  t2=clock();
  fprintf(stderr, "Duration: %.0f ms.\n", (double)(t2-t1));

  return 0;
}
