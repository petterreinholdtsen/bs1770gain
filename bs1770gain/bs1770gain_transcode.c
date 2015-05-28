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
#include <ffsox_priv.h>

static void bs1770gain_tags_rg(bs1770gain_tag_t *tags,
    const bs1770gain_stats_t *track,
    const bs1770gain_stats_t *album,
    const bs1770gain_options_t *options)
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

static void bs1770gain_tags_bwf(bs1770gain_tag_t *tags,
    const bs1770gain_stats_t *stats,
    const bs1770gain_options_t *options)
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
static void bs1770gain_clone_tags(bs1770gain_tag_t *tags, sink_t *so,
    source_t *si, bs1770gain_stats_t *track, bs1770gain_stats_t *album,
    const bs1770gain_options_t *options)
{
  //int i;
  read_list_t *reads;
  read_t *read;
  write_t *write;
  AVStream *ist,*ost;

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

  LIST_FOREACH(&reads,si->reads.h) {
    read=reads->read;

    if (NULL!=(write=read->write)) {
      ist=read->s.st;
      ost=write->s.st;
      bs1770gain_clone_dict(&ost->metadata,ist->metadata,tags);
#if 0 // {
      if (ai==i)
        bs1770gain_write_dict(&ost->metadata,ist->metadata,tags);
#endif // }
    };
  }
}

int bs1770gain_transcode(bs1770gain_stats_t *track, bs1770gain_stats_t *album,
    const char *ipath, const char *opath, const bs1770gain_options_t *options)
{
  enum { CODEC_ID=AV_CODEC_ID_FLAC,SAMPLE_FMT=AV_SAMPLE_FMT_S32 };

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

  int code=-1;
  double drc=options->drc;
  int ai=options->audio;
  int vi=options->video;
  ffsox_source_t si;
  ffsox_sink_t so;
  ffsox_machine_t m;
  double q;

  if (ffsox_source_create(&si,ipath)<0) {
    FFSOX_MESSAGE("creating source");
    goto si;
  }

  if (0!=options->extensions)
    ffsox_csv2avdict(si.f.path,'\t',&si.f.fc->metadata);

  if (ffsox_sink_create(&so,opath)<0) {
    FFSOX_MESSAGE("creating sink");
    goto so;
  }

  if (BS1770GAIN_MODE_APPLY==options->mode) {
    q=options->preamp+options->level;
    q-=(1.0-options->apply)*bs1770gain_stats_get_loudness(album,options);
    q-=options->apply*bs1770gain_stats_get_loudness(track,options);
    q=LIB1770_DB2Q(q);
  }
  else
    q=-1.0;

  if (ffsox_source_link_create(&si,&so,drc,CODEC_ID,SAMPLE_FMT,q,ai,vi)<0) {
    FFSOX_MESSAGE("creating link");
    goto link;
  }

  // copy all tags except the RG/BWF ones.
  bs1770gain_clone_tags(tags,&so,&si,track,album,options);

  if (BS1770GAIN_MODE_APPLY!=options->mode) {
    // set the RG/BWF tags.
    bs1770gain_write_dict(&so.f.fc->metadata,si.f.fc->metadata,tags);
  }

  if (ffsox_source_seek(&si,options->begin)<0) {
    FFSOX_MESSAGE("seeking");
    goto seek;
  }

  if (ffsox_sink_open(&so)<0) {
    FFSOX_MESSAGE("opening sink");
    goto open;
  }

  if (ffsox_machine_create(&m,&si)<0) {
    FFSOX_MESSAGE("creating machine");
    goto machine;
  }

  if (ffsox_machine_loop(&m)<0) {
    FFSOX_MESSAGE("running machine");
    goto run;
  }

  code=0;
// cleanup:
run:
  ffsox_machine_cleanup(&m);
machine:
  ffsox_sink_close(&so);
open:
seek:
  ffsox_source_link_cleanup(&si);
link:
  ffsox_sink_cleanup(&so);
so:
  si.vmt->cleanup(&si);
si:
  code=0;
//cleanup:
  return code;
}
