/*
 * bs1770gain_transcode.c
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
typedef struct bs1770gain_streams bs1770gain_streams_t;

struct bs1770gain_streams {
  double q;

  struct {
    int ai;
    int vi;
  } i;

  struct {
    int ai;
    int vi;
  } o;
};

///////////////////////////////////////////////////////////////////////////////
static int bs1770gain_new_stream(AVStream *ist, AVFormatContext *ofc,
    int codec_type)
{
  AVCodecContext *icc=ist->codec;
  AVCodec *oc;
  AVStream *ost;
  AVCodecContext *occ;
  AVRational sar;

  if (codec_type<0) {
    // copy stream (video and audio).
    if (NULL==(ost=avformat_new_stream(ofc,icc->codec)))
      goto ost;

    occ=ost->codec;

    if (avcodec_copy_context(occ,icc)<0)
      goto ost;

    switch (icc->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
      sar=ist->sample_aspect_ratio.num
          ?ist->sample_aspect_ratio
          :icc->sample_aspect_ratio;
      ost->sample_aspect_ratio=sar;
      occ->sample_aspect_ratio=sar;
      ost->avg_frame_rate=ist->avg_frame_rate;
      break;
    case AVMEDIA_TYPE_AUDIO:
      break;
    default:
      break;
    }

    ost->time_base=ist->time_base;
  }
  else {
    // decode stream (audio only).
    BS1770GAIN_GOTO(NULL==(oc=avcodec_find_encoder(codec_type)),
        "finding encoder",ost);
    BS1770GAIN_GOTO(NULL==(ost=avformat_new_stream(ofc,NULL)),
        "allocation output stream",ost);
    occ=ost->codec;
    occ->sample_fmt=BS1770GAIN_SAMPLE_FMT;
    occ->sample_rate=icc->sample_rate;
    occ->channel_layout=icc->channel_layout;
    occ->channels=icc->channels;
#if 0 // {
    occ->time_base=icc->time_base;
#else // } {
    occ->time_base=(AVRational){1,occ->sample_rate};
#endif // }
    BS1770GAIN_GOTO(avcodec_open2(occ,oc,NULL)<0,
        "allocation encoder",ost);
    ost->time_base=occ->time_base;
  }

  occ->codec_tag=0;

  if (ofc->oformat->flags&AVFMT_GLOBALHEADER)
    occ->flags|=CODEC_FLAG_GLOBAL_HEADER;

  return 0;
ost:
  return -1;
}

static AVFormatContext *bs1770gain_open_output(const AVFormatContext *ifc,
    const char *opath, bs1770gain_streams_t *s, int codec_type)
{
  AVFormatContext *ofc=NULL;

  BS1770GAIN_GOTO(avformat_alloc_output_context2(&ofc,NULL,NULL,opath)<0,
      "opening output format",ofc);

  if (0<=s->i.vi) {
    s->o.vi=ofc->nb_streams;
    BS1770GAIN_GOTO(bs1770gain_new_stream(ifc->streams[s->i.vi],ofc,-1)<0,
        "opening output stream",ost);
  }
  else
    s->o.vi=-1;

  s->o.ai=ofc->nb_streams;
  BS1770GAIN_GOTO(bs1770gain_new_stream(ifc->streams[s->i.ai],ofc,
      codec_type)<0,"opening output stream",ost);

  return ofc;
ost:
  avformat_free_context(ofc);
ofc:
  return NULL;
}

static void bs1770gain_tags_rg(bs1770gain_tag_t *tags,
    const bs1770gain_stats_t *track,
    const bs1770gain_stats_t *album,
    const bs1770gain_options_t *options)
{
  double db,level;

  level=options->preamp+options->level;

  while (NULL!=tags->key) {
    ///////////////////////////////////////////////////////////////////////////
    if (0==strcasecmp("REPLAYGAIN_ALGORITHM",tags->key)) {
      if (0!=options->integrated)
        strcpy(tags->val,"ITU BS.1770");
    }
    else if (0==strcasecmp("REPLAYGAIN_REFERENCE_LOUDNESS",tags->key)) {
      if (0!=options->integrated)
        sprintf(tags->val,"%.2f",level);
    }
    ///////////////////////////////////////////////////////////////////////////
    else if (0==strcasecmp("REPLAYGAIN_TRACK_GAIN",tags->key)) {
#if 0 // {
      if (0!=options->integrated) {
        lib1770_stats_get_mean(track->stats_im,-10.0);
        sprintf(tags->val,"%.2f LU",level-db);
      }
#else // } {
      db=bs1770gain_stats_get_loudness(track,options);
      sprintf(tags->val,"%.2f LU",level-db);
#endif // }
    }
    else if (0==strcasecmp("REPLAYGAIN_TRACK_PEAK",tags->key)) {
      if (0!=options->truepeak)
        sprintf(tags->val,"%.f",track->peak_t);
      else if (0!=options->samplepeak)
        sprintf(tags->val,"%.f",track->peak_s);
    }
    else if (0==strcasecmp("REPLAYGAIN_TRACK_RANGE",tags->key)) {
      if (0!=options->range) {
        db=lib1770_stats_get_range(track->stats_rs,-20.0,0.1,0.95);
        sprintf(tags->val,"%.2f LU",db);
      }
    }
    ///////////////////////////////////////////////////////////////////////////
    else if (0==strcasecmp("REPLAYGAIN_ALBUM_GAIN",tags->key)) {
#if 0 // {
      if (0!=options->integrated) {
        db=lib1770_stats_get_mean(album->stats_im,-10.0);
        sprintf(tags->val,"%.2f LU",level-db);
      }
#else // } {
      db=bs1770gain_stats_get_loudness(album,options);
      sprintf(tags->val,"%.2f LU",level-db);
#endif // }
    }
    else if (0==strcasecmp("REPLAYGAIN_ALBUM_PEAK",tags->key)) {
      if (0!=options->truepeak)
        sprintf(tags->val,"%.f",album->peak_t);
      else if (0!=options->samplepeak)
        sprintf(tags->val,"%.f",album->peak_s);
    }
    else if (0==strcasecmp("REPLAYGAIN_ALBUM_RANGE",tags->key)) {
      if (0!=options->range) {
        db=lib1770_stats_get_range(album->stats_rs,-20.0,0.1,0.95);
        sprintf(tags->val,"%.2f LU",db);
      }
    }

    ++tags;
  }
}

static void bs1770gain_tags_bwf(bs1770gain_tag_t *tags,
    const bs1770gain_stats_t *stats,
    const bs1770gain_options_t *options)
{
  double db,level;

  while (NULL!=tags->key) { 
    if (0==strcasecmp("LoudnessValue",tags->key)) {
#if 0 // {
      if (0!=options->integrated) {
        level=options->preamp+options->level;
        db=lib1770_stats_get_mean(stats->stats_im,-10.0);
        sprintf(tags->val,"%d",(int)floor(100.0*(level-db)+0.5));
      }
#else // } {
      level=options->preamp+options->level;
      db=bs1770gain_stats_get_loudness(stats,options);
      sprintf(tags->val,"%d",(int)floor(100.0*(level-db)+0.5));
#endif // }
    }
    else if (0==strcasecmp("MaxTruePeakLevel",tags->key)) {
      if (0!=options->truepeak)
        sprintf(tags->val,"%.f",stats->peak_t);
      else if (0!=options->samplepeak)
        sprintf(tags->val,"%.f",stats->peak_s);
    }
    else if (0==strcasecmp("LoudnessRange",tags->key)) {
      if (0!=options->range) {
        db=lib1770_stats_get_range(stats->stats_rs,-20.0,0.1,0.95);
        sprintf(tags->val,"%d",(int)floor(100.0*db+0.5));
      }
    }

    ++tags;
  }
}

static void bs1770gain_clone_dict(AVDictionary **ometadata,
    AVDictionary *imetadata, bs1770gain_tag_t *tags)
{
  AVDictionaryEntry *mtag;
  bs1770gain_tag_t *t;

  mtag=NULL;

  // for each tag ...
  while (NULL!=(mtag=av_dict_get(imetadata,"",mtag,AV_DICT_IGNORE_SUFFIX))) {
    // ... filter out RG/BWF tags
    for (t=tags;NULL!=t->key;++t) {
      if (0==strcasecmp(t->key,mtag->key))
        goto next_mtag;
    }

    // ... copy it into the fresh dictionary.
    av_dict_set(ometadata,mtag->key,mtag->value,0);
  next_mtag:
    continue;
  }
}

static void bs1770gain_write_dict(AVDictionary **ometadata,
    AVDictionary *imetadata, bs1770gain_tag_t *tags)
{
  bs1770gain_tag_t *t;

  // for each RG/BWF tag ...
  for (t=tags;NULL!=t->key;++t) {
    // ... if set, copy it into the fresh dictionanry.
    if (0!=*t->val)
      av_dict_set(ometadata,t->key,t->val,0);
  }
}

// copy all tags into the fresh dictionary except the RG/BWF ones.
static void bs1770gain_clone_tags(bs1770gain_tag_t *tags,
    AVFormatContext *ofc, AVFormatContext *ifc,
    bs1770gain_stats_t *track, bs1770gain_stats_t *album,
    const bs1770gain_options_t *options)
{
  AVStream *ist,*ost;
  int i;

  // initialize the RG/BWF tags.
  switch (options->mode) {
  case BS1770GAIN_MODE_RG_TAGS:
    bs1770gain_tags_rg(tags,track,album,options);
    break;
  case BS1770GAIN_MODE_BWF_TAGS:
    bs1770gain_tags_bwf(tags,track,options);
    break;
  default:
    break;
  }

  // copy the format dictionary.
  bs1770gain_clone_dict(&ofc->metadata,ifc->metadata,tags);

  for (i=0;i<ifc->nb_streams;++i) {
    ost=ofc->streams[i];
    ist=ifc->streams[i];
    // copy the stream dictionary.
    bs1770gain_clone_dict(&ost->metadata,ist->metadata,tags);
#if 0 // {
    if (ai==i)
      bs1770gain_write_dict(&ost->metadata,ist->metadata,tags);
#endif // }
  }
}

int bs1770gain_transcode(bs1770gain_stats_t *track, bs1770gain_stats_t *album,
    const char *ipath, const char *opath, const bs1770gain_options_t *options)
{
  bs1770gain_tag_t tags[]={
    ///////////////////////////////////////////////////////////////////////////
    { .key="REPLAYGAIN_ALGORITHM",          .val="" },
    { .key="REPLAYGAIN_REFERENCE_LOUDNESS", .val="" },
    { .key="REPLAYGAIN_TRACK_GAIN",         .val="" },
    { .key="REPLAYGAIN_TRACK_PEAK",         .val="" },
    { .key="REPLAYGAIN_TRACK_RANGE",        .val="" },
    { .key="REPLAYGAIN_ALBUM_GAIN",         .val="" },
    { .key="REPLAYGAIN_ALBUM_RANGE",        .val="" },
    { .key="REPLAYGAIN_ALBUM_PEAK",         .val="" },
    ///////////////////////////////////////////////////////////////////////////
    { .key="LoudnessValue" ,                .val=""},
    { .key="MaxTruePeakLevel",              .val="" },
    { .key="LoudnessRange",                 .val="" },
    ///////////////////////////////////////////////////////////////////////////
    { .key=NULL,                            .val="" }
  };

  int code;
  AVFormatContext *ifc,*ofc;
  bs1770gain_streams_t s;
  int size,got_frame;
  AVCodecContext *adc;
  AVDictionary *opts;
  AVCodec *ad;
  bs1770gain_convert_t *convert;
  AVStream *st;
  AVPacket p1,p2;
  char drc[32];
  int64_t ts1,ts2;

  code=-1;
  ifc=NULL;
  opts=NULL;
  ofc=NULL;

  BS1770GAIN_GOTO(avformat_open_input(&ifc,ipath,0,0)<0,
      "open input file",ifc);
  ifc->flags|=AVFMT_FLAG_GENPTS;
  BS1770GAIN_GOTO(avformat_find_stream_info(ifc,0)<0,
      "finding stream info",find);
  BS1770GAIN_GOTO(bs1770gain_audiostream(ifc,&s.i.ai,&s.i.vi,options)<0,
      "finding audio",find);

  if (0!=options->extensions)
    bs1770gain_csv2avdict(ipath,'\t',&ifc->metadata);

  if (BS1770GAIN_MODE_APPLY==options->mode) {
    adc=ifc->streams[s.i.ai]->codec;
    // we want to have stereo.
    // TODO: should be an option.
    adc->request_channel_layout=AV_CH_LAYOUT_STEREO;

    // find a decoder.
    BS1770GAIN_GOTO(NULL==(ad=bs1770gain_find_decoder(adc->codec_id)),
        "finding audio decoder",find);

    if (AV_CODEC_ID_AC3==adc->codec_id) {
      // avoid dynamic range compression.
      sprintf(drc,"%0.2f",options->drc);
      BS1770GAIN_GOTO(av_dict_set(&opts,"drc_scale",drc,0)!=0,
          "switching off dynamic range compression",opts);
    }

    // open the audio decoder.
    BS1770GAIN_GOTO(0!=avcodec_open2(adc,ad,&opts),"opening decoder",adc);

    // open the output context.
    ofc=bs1770gain_open_output(ifc,opath,&s,AV_CODEC_ID_FLAC);
    BS1770GAIN_GOTO(NULL==ofc,"opening output format",ofc);
    // copy all tags except the RG/BWF ones.
    bs1770gain_clone_tags(tags,ofc,ifc,track,album,options);

    // open a converter.
    convert=bs1770gain_convert_new(ifc,s.i.ai,ofc,s.o.ai,options,track,album);
    BS1770GAIN_GOTO(NULL==convert,"allocation convert",convert);
  }
  else {
    adc=NULL;
    ad=NULL;
    convert=NULL;

    // open the output context.
    BS1770GAIN_GOTO(NULL==(ofc=bs1770gain_open_output(ifc,opath,&s,-1)),
        "opening output format",ofc);
    // copy all tags except the RG/BWF ones.
    bs1770gain_clone_tags(tags,ofc,ifc,track,album,options);
    // set the RG/BWF tags.
    bs1770gain_write_dict(&ofc->metadata,ifc->metadata,tags);
  }

  if (0==(ofc->oformat->flags&AVFMT_NOFILE)) {
    BS1770GAIN_GOTO(avio_open(&ofc->pb,opath,AVIO_FLAG_WRITE)<0,
        "opening output file",pb);
  }

  av_init_packet(&p1);
  p1.data=NULL;
  p1.size=0;

  /////////////////////////////////////////////////////////////////////////////
  BS1770GAIN_GOTO((ts1=bs1770gain_seek(ifc,options))<0,"seeking",seek);
  BS1770GAIN_GOTO(avformat_write_header(ofc,NULL)<0,"writing header",header);

  while (0<=av_read_frame(ifc,&p1)) {
    // if stream is in range ...
    if (p1.stream_index<ifc->nb_streams) {
      // in case of seeking adjust the timestamps ...
      if (ts1>0) {
        st=ifc->streams[p1.stream_index];
        ts2=av_rescale_q(ts1,AV_TIME_BASE_Q,st->time_base);

        if (p1.dts!=AV_NOPTS_VALUE)
          p1.dts-=ts2;

        if (p1.pts!=AV_NOPTS_VALUE)
          p1.pts-=ts2;
      }

      if (bs1770gain_oor(&p1,ifc,options)) {
        av_free_packet(&p1);
        goto flush;
      }

      if (BS1770GAIN_MODE_APPLY==options->mode&&s.i.ai==p1.stream_index) {
        p2=p1;

        // transcode the selected audio stream.
        do {
          if ((size=bs1770gain_convert_packet(convert,&got_frame,&p2))<0) {
            BS1770GAIN_MESSAGE("transcoding audio");
            av_free_packet(&p1);
            goto write;
          }

          size=FFMIN(size,p2.size);
          p2.data+=size;
          p2.size-=size;
        } while (0<p2.size);
      }
      else if (s.i.ai==p1.stream_index||s.i.vi==p1.stream_index) {
        // pass through only selected streams, i.e. just
        // one audio and just one video.
        p1.stream_index=p1.stream_index==s.i.ai?s.o.ai:s.o.vi;

        if (av_interleaved_write_frame(ofc,&p1)<0) {
          BS1770GAIN_MESSAGE("writing frame");
          av_free_packet(&p1);
          goto write;
        }
      }
    }

    av_free_packet(&p1);
  }
flush:
  if (BS1770GAIN_MODE_APPLY==options->mode) {
    // flush the decoder.
    p1.stream_index=s.i.ai;
    p1.data=NULL;
    p1.size=0;

    do {
      if (bs1770gain_convert_packet(convert,&got_frame,&p1)<0) {
        BS1770GAIN_MESSAGE("transcoding audio");
        goto write;
      }
    } while (got_frame);

    // signal to the encoder eof by sending a NULL frame.
    // use a properly initialized packet, i.e. p1.
    avcodec_encode_audio2(ofc->streams[s.i.ai]->codec,&p1,NULL,&got_frame);
  }

  BS1770GAIN_GOTO(av_write_trailer(ofc),"writing trailer",trailer);
  code=0;
// cleanup:
trailer:
write:
header:
seek:
  if (0==(ofc->flags&AVFMT_NOFILE))
    avio_close(ofc->pb);
pb:
  if (NULL!=convert)
    bs1770gain_convert_close(convert);
convert:
  avformat_free_context(ofc);
ofc:
  if (NULL!=adc)
    avcodec_close(adc);
adc:
  av_dict_free(&opts);
opts:
find:
  avformat_close_input(&ifc);
ifc:
  return code;
}
