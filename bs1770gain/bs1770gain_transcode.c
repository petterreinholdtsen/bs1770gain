/*
 * bs1770gain_transcode.c
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
#include <bs1770gain_priv.h>
#include <ctype.h>

static void bs1770gain_tags_rg(tag_t *tags, const aggregate_t *track,
    const aggregate_t *album, const options_t *options)
{
  double db,norm;

  norm=options->preamp+options->norm;

  while (NULL!=tags->key) {
    ///////////////////////////////////////////////////////////////////////////
    if (0==strcasecmp("REPLAYGAIN_ALGORITHM",tags->key)) {
      strcpy(tags->val,"ITU-R BS.1770");
      goto next;
    }
    else if (0==strcasecmp("REPLAYGAIN_REFERENCE_LOUDNESS",tags->key)) {
      sprintf(tags->val,"%.2f",norm);
      goto next;
    }

    ///////////////////////////////////////////////////////////////////////////
    if (BS1770GAIN_IS_MODE_TRACK_TAGS(options->mode)) {
      if (0==strcasecmp("REPLAYGAIN_TRACK_GAIN",tags->key)) {
        db=bs1770gain_aggregate_get_loudness(track,options);
        sprintf(tags->val,"%.2f %s",norm-db,options->unit);
        goto next;
      }
      else if (0==strcasecmp("REPLAYGAIN_TRACK_PEAK",tags->key)) {
        if (0!=(track->flags&AGGREGATE_TRUEPEAK))
          sprintf(tags->val,"%f",track->truepeak);
        else if (0!=(track->flags&AGGREGATE_SAMPLEPEAK))
          sprintf(tags->val,"%f",track->samplepeak);

        goto next;
      }
      else if (0==strcasecmp("REPLAYGAIN_TRACK_RANGE",tags->key)) {
        if (0!=(track->flags&AGGREGATE_SHORTTERM_RANGE)) {
          db=lib1770_stats_get_range(track->shortterm,
              options->shortterm.range_gate,
              options->shortterm.range_lower_bound,
              options->shortterm.range_upper_bound);
          sprintf(tags->val,"%.2f %s",db,options->unit);
        }

        goto next;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    if (BS1770GAIN_IS_MODE_ALBUM_TAGS(options->mode)) {
      if (0==strcasecmp("REPLAYGAIN_ALBUM_GAIN",tags->key)) {
        db=bs1770gain_aggregate_get_loudness(album,options);
        sprintf(tags->val,"%.2f %s",norm-db,options->unit);
        goto next;
      }
      else if (0==strcasecmp("REPLAYGAIN_ALBUM_PEAK",tags->key)) {
        if (0!=(album->flags&AGGREGATE_TRUEPEAK))
          sprintf(tags->val,"%f",album->truepeak);
        else if (0!=(album->flags&AGGREGATE_SAMPLEPEAK))
          sprintf(tags->val,"%f",album->samplepeak);

        goto next;
      }
      else if (0==strcasecmp("REPLAYGAIN_ALBUM_RANGE",tags->key)) {
        if (0!=(album->flags&AGGREGATE_SHORTTERM_RANGE)) {
          db=lib1770_stats_get_range(album->shortterm,
              options->shortterm.range_gate,
              options->shortterm.range_lower_bound,
              options->shortterm.range_upper_bound);
          sprintf(tags->val,"%.2f %s",db,options->unit);
        }

        goto next;
      }
    }

    ///////////////////////////////////////////////////////////////////////////
  next:
    ++tags;
  }
}

static void bs1770gain_tags_bwf(tag_t *tags, const aggregate_t *aggregate,
    const options_t *options)
{
  double db,norm;

  while (NULL!=tags->key) { 
    if (0==strcasecmp("LoudnessValue",tags->key)) {
      norm=options->preamp+options->norm;
      db=bs1770gain_aggregate_get_loudness(aggregate,options);
      sprintf(tags->val,"%d",(int)floor(100.0*(norm-db)+0.5));
    }
    else if (0==strcasecmp("MaxTruePeakLevel",tags->key)) {
      if (0!=(aggregate->flags&AGGREGATE_TRUEPEAK))
        sprintf(tags->val,"%f",aggregate->truepeak);
      else if (0!=(aggregate->flags&AGGREGATE_SAMPLEPEAK))
        sprintf(tags->val,"%f",aggregate->samplepeak);
    }
    else if (0==strcasecmp("LoudnessRange",tags->key)) {
      if (0!=(aggregate->flags&AGGREGATE_SHORTTERM_RANGE)) {
        db=lib1770_stats_get_range(aggregate->shortterm,
            options->shortterm.range_gate,
            options->shortterm.range_lower_bound,
            options->shortterm.range_upper_bound);
        sprintf(tags->val,"%d",(int)floor(100.0*db+0.5));
      }
    }

    ++tags;
  }
}

#if defined (BS1770GAIN_TAG_PREFIX) // {
static const char *bs1770gain_convert_key(tag_t *t, char *key, size_t size,
    const options_t *options)
{
  const char *p1,*p2,*rp;
  char *wp;

    p1=t->key;
    p2="REPLAYGAIN";

    while (*p2) {
      if (*p1++!=*p2++) {
        // no RG tag, copy it into the fresh dictionanry.
        return t->key;
      }
    }

    if (size<strlen(options->tag_prefix)+strlen(p1))
      return NULL;

    wp=key;

    for (rp=options->tag_prefix;*rp;++rp)
      *wp++=toupper(*rp);

    for (rp=p1;*rp;++rp)
      *wp++=*rp;

    *wp=0;

    // RG tag, copy it into the fresh dictionanry.
    return key;
}

static void bs1770gain_clone_dict(track_t *track, AVDictionary **ometadata,
    AVDictionary *imetadata, tag_t *tags, const options_t *options)
{
  enum {
    TRACK=1<<1,
    DISC=1<<2
  };

#if 0 // {
  album_t *album=track->album;
#endif // }
  AVDictionaryEntry *de=NULL;
  tag_t *t;
  int flags=0;
  char value[32];
  char buf[128];
  const char *key;

  // for each tag ...
  while (NULL!=(de=av_dict_get(imetadata,"",de,AV_DICT_IGNORE_SUFFIX))) {
    // ... filter out RG/BWF tags
    for (t=tags;NULL!=t->key;++t) {
      if (NULL==(key=bs1770gain_convert_key(t,buf,sizeof buf,options)))
        goto next_de;
      else if (0==strcasecmp(key,de->key))
        goto next_de;
    }

    // ... copy it into the fresh dictionary.
    if (0==strcasecmp("TRACK",de->key))
      flags|=TRACK;
    else if (0==strcasecmp("DISC",de->key))
      flags|=DISC;

    av_dict_set(ometadata,de->key,de->value,0);
  next_de:
    continue;
  }

  if (0!=(EXTENSION_TAGS&options->extensions)) {
    if (0==(TRACK&flags)) {
#if 0 // {
      sprintf(value,"%d/%d",track->n,album->n);
#else // } {
      sprintf(value,"%d",track->n);
#endif // }
      av_dict_set(ometadata,"TRACK",value,0);
    }

    if (0==(DISC&flags)) {
      sprintf(value,"%d",1);
      av_dict_set(ometadata,"DISC",value,0);
    }
  }
}

static void bs1770gain_write_dict(AVDictionary **ometadata, tag_t *tags,
    const options_t *options)
{
  tag_t *t;
  char buf[128];
  const char *key;

  // for each RG/BWF tag ...
  for (t=tags;NULL!=t->key;++t) {
    if (0==*t->val)
      continue;

    if (NULL!=(key=bs1770gain_convert_key(t,buf,sizeof buf,options)))
      av_dict_set(ometadata,key,t->val,0);
  }
}
#else // } {
static void bs1770gain_clone_dict(track_t *track, AVDictionary **ometadata,
    AVDictionary *imetadata, tag_t *tags, const options_t *options)
{
  enum {
    TRACK=1<<1,
    DISC=1<<2
  };

#if 0 // {
  album_t *album=track->album;
#endif // }
  AVDictionaryEntry *de=NULL;
  tag_t *t;
  int flags=0;
  char value[32];

  // for each tag ...
  while (NULL!=(de=av_dict_get(imetadata,"",de,AV_DICT_IGNORE_SUFFIX))) {
    // ... filter out RG/BWF tags
    for (t=tags;NULL!=t->key;++t) {
      if (0==strcasecmp(t->key,de->key))
        goto next_de;
    }

    // ... copy it into the fresh dictionary.
    if (0==strcasecmp("TRACK",de->key))
      flags|=TRACK;
    else if (0==strcasecmp("DISC",de->key))
      flags|=DISC;

    av_dict_set(ometadata,de->key,de->value,0);
  next_de:
    continue;
  }

  if (0!=(EXTENSION_TAGS&options->extensions)) {
    if (0==(TRACK&flags)) {
#if 0 // {
      sprintf(value,"%d/%d",track->n,album->n);
#else // } {
      sprintf(value,"%d",track->n);
#endif // }
      av_dict_set(ometadata,"TRACK",value,0);
    }

    if (0==(DISC&flags)) {
      sprintf(value,"%d",1);
      av_dict_set(ometadata,"DISC",value,0);
    }
  }
}

static void bs1770gain_write_dict(AVDictionary **ometadata, tag_t *tags)
{
  tag_t *t;

  // for each RG/BWF tag ...
  for (t=tags;NULL!=t->key;++t) {
    // ... if set, copy it into the fresh dictionanry.
    if (0!=*t->val)
      av_dict_set(ometadata,t->key,t->val,0);
  }
}
#endif // }

// copy all tags into the fresh dictionary except the RG/BWF ones.
static void bs1770gain_clone_tags(tag_t *tags, sink_t *so, source_t *si,
    track_t *track, const options_t *options)
{
  album_t *album=track->album;
  stream_list_t *stream;
  AVStream *sti,*sto;

  if (BS1770GAIN_IS_MODE_RG_TAGS(options->mode))
    bs1770gain_tags_rg(tags,&track->aggregate,&album->aggregate,options);

  if (BS1770GAIN_IS_MODE_BWF_TAGS(options->mode))
    bs1770gain_tags_bwf(tags,&track->aggregate,options);

  // copy the format dictionary.
  bs1770gain_clone_dict(track,&so->f.fc->metadata,si->f.fc->metadata,tags,
      options);

  LIST_FOREACH(&stream,so->streams) {
    sti=stream->si->st;
    sto=stream->so->st;
    bs1770gain_clone_dict(track,&sto->metadata,sti->metadata,tags,options);
#if 0 // {
    if (ai==i)
      bs1770gain_write_dict(&ost->metadata,tags);
#endif // }
  }
}

int bs1770gain_transcode(track_t *t, options_t *options)
{
  enum { CODEC_ID=AV_CODEC_ID_FLAC,SAMPLE_FMT=AV_SAMPLE_FMT_S32 };

  tag_t tags[]={
    ///////////////////////////////////////////////////////////////////////////
    { "REPLAYGAIN_ALGORITHM",          {0} },
    { "REPLAYGAIN_REFERENCE_LOUDNESS", {0} },
    { "REPLAYGAIN_TRACK_GAIN",         {0} },
    { "REPLAYGAIN_TRACK_PEAK",         {0} },
    { "REPLAYGAIN_TRACK_RANGE",        {0} },
    { "REPLAYGAIN_ALBUM_GAIN",         {0} },
    { "REPLAYGAIN_ALBUM_RANGE",        {0} },
    { "REPLAYGAIN_ALBUM_PEAK",         {0} },
    ///////////////////////////////////////////////////////////////////////////
    { "LoudnessValue",                 {0} },
    { "MaxTruePeakLevel",              {0} },
    { "LoudnessRange",                 {0} },
    ///////////////////////////////////////////////////////////////////////////
    { NULL,                            {0} }
  };

  int code=-1;
  bs1770gain_print_t *p=&options->p;
  FILE *f=p->vmt->session.file(p);
  source_cb_t progress=f?ffsox_source_progress:NULL;
  int stereo=options->stereo;
  double drc=options->drc;
  int ai=options->audio;
  int vi=options->video;
  album_t *a=t->album;
  source_t si;
  sink_t so;
  machine_t m;
  int sample_fmt;
  double q;

  TRACE_PUSH();

  if (ffsox_source_create(&si,t->ipath,ai,vi,progress,f)<0) {
    DMESSAGE("creating source");
    goto si;
  }

  if (0!=(EXTENSION_CSV&options->extensions))
    ffsox_csv2avdict(si.f.path,'\t',&si.f.fc->metadata);

  if (bs1770gain_track_alloc_output(t,&si,options)<0) {
    DMESSAGE("allocating output path");
    goto output;
  }

  if (0==pbu_same_file(t->ipath,t->opath)) {
    DMESSAGE("overwriting of input not supported");
    goto output;
  }

  if (ffsox_sink_create(&so,t->opath)<0) {
    DMESSAGE("creating sink");
    goto so;
  }

  if (BS1770GAIN_IS_MODE_APPLY(options->mode)) {
    q=options->preamp+options->norm;
    q-=(1.0-options->apply)*bs1770gain_aggregate_get_loudness(&a->aggregate,
        options);
    q-=options->apply*bs1770gain_aggregate_get_loudness(&t->aggregate,
        options);
    q=LIB1770_DB2Q(q);
    sample_fmt=SAMPLE_FMT;
  }
  else {
    q=NULL==options->format?-1.0:1.0;
    sample_fmt=-1;
  }

  if (ffsox_source_link_create(&si,&so,stereo,drc,CODEC_ID,sample_fmt,q)<0) {
    DMESSAGE("creating link");
    goto link;
  }

//code=0;
#if 1 // {
  // copy all tags except the RG/BWF ones.
  bs1770gain_clone_tags(tags,&so,&si,t,options);

  if (!BS1770GAIN_IS_MODE_APPLY(options->mode)) {
    // set the RG/BWF tags.
#if defined (BS1770GAIN_TAG_PREFIX) // {
    bs1770gain_write_dict(&so.f.fc->metadata,tags,options);
#else // } {
    bs1770gain_write_dict(&so.f.fc->metadata,tags);
#endif // }
  }

  if (ffsox_source_seek(&si,options->begin)<0) {
    DMESSAGE("seeking");
    goto seek;
  }

  if (ffsox_sink_open(&so)<0) {
    DMESSAGE("opening sink");
    goto open;
  }

  // print a start massage.
  if (stdout==f) {
    fprintf(f,"  [%d/%d] \"%s\" ",t->n,a->n,pbu_basename(t->opath));
    fflush(f);
  }

  if (ffsox_machine_run(&m,&si.node)<0) {
    DMESSAGE("running machine");
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
#endif // }
  ffsox_source_link_cleanup(&si);
link:
  ffsox_sink_cleanup(&so);
so:
output:
  si.vmt->cleanup(&si);
si:
  //code=0;
//cleanup:
  TRACE_POP();

  return code;
}
