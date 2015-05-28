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
#include <bs1770gain_priv.h>

static void bs1770gain_tags_rg(tag_t *tags, const stats_t *track,
    const stats_t *album, const options_t *options)
{
  double db,level;

  level=options->preamp+options->level;

  while (NULL!=tags->key) {
    ///////////////////////////////////////////////////////////////////////////
    if (0==strcasecmp("REPLAYGAIN_ALGORITHM",tags->key))
      strcpy(tags->val,"ITU-R BS.1770");
    else if (0==strcasecmp("REPLAYGAIN_REFERENCE_LOUDNESS",tags->key))
      sprintf(tags->val,"%.2f",level);
    ///////////////////////////////////////////////////////////////////////////
    else if (0==strcasecmp("REPLAYGAIN_TRACK_GAIN",tags->key)) {
      db=bs1770gain_stats_get_loudness(track,options);
      sprintf(tags->val,"%.2f LU",level-db);
    }
    else if (0==strcasecmp("REPLAYGAIN_TRACK_PEAK",tags->key)) {
      if (0!=options->truepeak)
        sprintf(tags->val,"%.f",track->peak_t);
      else if (0!=options->samplepeak)
        sprintf(tags->val,"%.f",track->peak_s);
    }
    else if (0==strcasecmp("REPLAYGAIN_TRACK_RANGE",tags->key)) {
      if (0!=options->shortterm.range) {
        db=lib1770_stats_get_range(track->shortterm,
            options->shortterm.range_gate,
            options->shortterm.range_lower_bound,
            options->shortterm.range_upper_bound);
        sprintf(tags->val,"%.2f LU",db);
      }
    }
    ///////////////////////////////////////////////////////////////////////////
    else if (0==strcasecmp("REPLAYGAIN_ALBUM_GAIN",tags->key)) {
      db=bs1770gain_stats_get_loudness(album,options);
      sprintf(tags->val,"%.2f LU",level-db);
    }
    else if (0==strcasecmp("REPLAYGAIN_ALBUM_PEAK",tags->key)) {
      if (0!=options->truepeak)
        sprintf(tags->val,"%.f",album->peak_t);
      else if (0!=options->samplepeak)
        sprintf(tags->val,"%.f",album->peak_s);
    }
    else if (0==strcasecmp("REPLAYGAIN_ALBUM_RANGE",tags->key)) {
      if (0!=options->shortterm.range) {
        db=lib1770_stats_get_range(album->shortterm,
            options->shortterm.range_gate,
            options->shortterm.range_lower_bound,
            options->shortterm.range_upper_bound);
        sprintf(tags->val,"%.2f LU",db);
      }
    }

    ++tags;
  }
}

static void bs1770gain_tags_bwf(tag_t *tags, const stats_t *stats,
    const options_t *options)
{
  double db,level;

  while (NULL!=tags->key) { 
    if (0==strcasecmp("LoudnessValue",tags->key)) {
      level=options->preamp+options->level;
      db=bs1770gain_stats_get_loudness(stats,options);
      sprintf(tags->val,"%d",(int)floor(100.0*(level-db)+0.5));
    }
    else if (0==strcasecmp("MaxTruePeakLevel",tags->key)) {
      if (0!=options->truepeak)
        sprintf(tags->val,"%.f",stats->peak_t);
      else if (0!=options->samplepeak)
        sprintf(tags->val,"%.f",stats->peak_s);
    }
    else if (0==strcasecmp("LoudnessRange",tags->key)) {
      if (0!=options->shortterm.range) {
        db=lib1770_stats_get_range(stats->shortterm,
            options->shortterm.range_gate,
            options->shortterm.range_lower_bound,
            options->shortterm.range_upper_bound);
        sprintf(tags->val,"%d",(int)floor(100.0*db+0.5));
      }
    }

    ++tags;
  }
}

static void bs1770gain_clone_dict(AVDictionary **ometadata,
    AVDictionary *imetadata, tag_t *tags)
{
  AVDictionaryEntry *de=NULL;
  tag_t *t;

  // for each tag ...
  while (NULL!=(de=av_dict_get(imetadata,"",de,AV_DICT_IGNORE_SUFFIX))) {
    // ... filter out RG/BWF tags
    for (t=tags;NULL!=t->key;++t) {
      if (0==strcasecmp(t->key,de->key))
        goto next_de;
    }

    // ... copy it into the fresh dictionary.
    av_dict_set(ometadata,de->key,de->value,0);
  next_de:
    continue;
  }
}

static void bs1770gain_write_dict(AVDictionary **ometadata,
    AVDictionary *imetadata, tag_t *tags)
{
  tag_t *t;

  // for each RG/BWF tag ...
  for (t=tags;NULL!=t->key;++t) {
    // ... if set, copy it into the fresh dictionanry.
    if (0!=*t->val)
      av_dict_set(ometadata,t->key,t->val,0);
  }
}

// copy all tags into the fresh dictionary except the RG/BWF ones.
static void bs1770gain_clone_tags(tag_t *tags, sink_t *so, source_t *si,
    stats_t *track, stats_t *album, const options_t *options)
{
  stream_list_t *stream;
  AVStream *sti,*sto;

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
  bs1770gain_clone_dict(&so->f.fc->metadata,si->f.fc->metadata,tags);

  LIST_FOREACH(&stream,so->streams) {
    sti=stream->si->st;
    sto=stream->so->st;
    bs1770gain_clone_dict(&sto->metadata,sti->metadata,tags);
#if 0 // {
    if (ai==i)
      bs1770gain_write_dict(&ost->metadata,ist->metadata,tags);
#endif // }
  }
}

static void bs1170gain_transcode_progress(const source_t *si, void *data)
{
  FILE *f=data;
  const AVPacket *pkt;
  const AVStream *st;
  int64_t duration;
  double percent;
  char buf[32];
  int i;

  pkt=&si->pkt;
  st=si->f.fc->streams[pkt->stream_index];
  duration=av_rescale_q(si->f.fc->duration,AV_TIME_BASE_Q,st->time_base);
  percent=0ll<pkt->dts&&0ll<duration?100.0*pkt->dts/duration:0.0;
  sprintf(buf,"%.0f%%",percent);
  fputs(buf,f);
  fflush(f);
  i=strlen(buf);

  while (0<i) {
    fputc('\b',f);
    --i;
  }
}

int bs1770gain_transcode(track_t *t, const options_t *options)
{
  enum { CODEC_ID=AV_CODEC_ID_FLAC,SAMPLE_FMT=AV_SAMPLE_FMT_S32 };

  tag_t tags[]={
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

  int code=-1;
  FILE *f=options->f;
  source_cb_t progress=stdout==f?bs1170gain_transcode_progress:NULL;
  double drc=options->drc;
  int ai=options->audio;
  int vi=options->video;
  album_t *a=t->album;
  source_t si;
  sink_t so;
  machine_t m;
  double q;

  if (ffsox_source_create(&si,t->ipath,ai,vi,progress,f)<0) {
    MESSAGE("creating source");
    goto si;
  }

  if (0!=options->extensions)
    ffsox_csv2avdict(si.f.path,'\t',&si.f.fc->metadata);

  if (bs1770gain_track_alloc_output(t,&si,options)<0) {
    MESSAGE("allocating output path");
    goto output;
  }

  if (0==pbu_same_file(t->ipath,t->opath)) {
    MESSAGE("overwriting of input not supported");
    goto output;
  }

  if (ffsox_sink_create(&so,t->opath)<0) {
    MESSAGE("creating sink");
    goto so;
  }

  if (BS1770GAIN_MODE_APPLY==options->mode) {
    q=options->preamp+options->level;
    q-=(1.0-options->apply)*bs1770gain_stats_get_loudness(a->stats,
        options);
    q-=options->apply*bs1770gain_stats_get_loudness(t->stats,options);
    q=LIB1770_DB2Q(q);
  }
  else
    q=-1.0;

  if (ffsox_source_link(&si,&so,drc,CODEC_ID,SAMPLE_FMT,q)<0) {
    MESSAGE("creating link");
    goto link;
  }

  // copy all tags except the RG/BWF ones.
  bs1770gain_clone_tags(tags,&so,&si,t->stats,a->stats,options);

  if (BS1770GAIN_MODE_APPLY!=options->mode) {
    // set the RG/BWF tags.
    bs1770gain_write_dict(&so.f.fc->metadata,si.f.fc->metadata,tags);
  }

  if (ffsox_source_seek(&si,options->begin)<0) {
    MESSAGE("seeking");
    goto seek;
  }

  if (ffsox_sink_open(&so)<0) {
    MESSAGE("opening sink");
    goto open;
  }

  // print a start massage.
  if (stdout==f) {
    fprintf(f,"  [%d/%d] \"%s\" ",t->n,a->n,pbu_basename(t->opath));
    fflush(f);
  }

  if (ffsox_machine_run(&m,&si.node)<0) {
    MESSAGE("running machine");
    goto machine;
  }

  if (stdout==f)
    fprintf(f,"    \n");

  code=0;
// cleanup:
machine:
  ffsox_sink_close(&so);
open:
seek:
link:
  ffsox_sink_cleanup(&so);
so:
output:
  si.vmt->cleanup(&si);
si:
  code=0;
//cleanup:
  return code;
}
