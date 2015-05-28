/*
 * ffsox_collect.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@snafu.de>
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
#include <ffsox_priv.h>

///////////////////////////////////////////////////////////////////////////////
#define FFMPEG_BASS 3

#define FFMPEG_CHANNEL_IN_RANGE(ch,channels) ( \
  (channels)<=LIB1770_MAX_CHANNELS \
  ?(ch)<LIB1770_MAX_CHANNELS \
  :((ch)<FFMPEG_BASS||(FFMPEG_BASS<(ch)&&(ch)<=LIB1770_MAX_CHANNELS)) \
)

///////////////////////////////////////////////////////////////////////////////
int ffsox_collect_create(collect_t *collect, collect_config_t *cc)
{
  collect->scale=cc->scale;
  collect->invscale=1.0/cc->scale;
  collect->aggregate=cc->aggregate;
  collect->channels=cc->channels;
  collect->sp=collect->sample;

  if (NULL==(collect->pre=lib1770_pre_new(cc->samplerate,cc->channels))) {
    MESSAGE("creating pre-filter");
    goto pre;
  }

  if (NULL==cc->aggregate->momentary)
    collect->momentary=NULL;
  else {
    collect->momentary=lib1770_block_new(cc->samplerate,cc->momentary.ms,
        cc->momentary.partition);

    if (NULL==collect->momentary) {
      MESSAGE("creating momentary block");
      goto momentary;
    }

    lib1770_block_add_stats(collect->momentary,cc->aggregate->momentary);
    lib1770_pre_add_block(collect->pre,collect->momentary);
  }
    
  if (NULL==cc->aggregate->shortterm)
    collect->shortterm=NULL;
  else {
    collect->shortterm=lib1770_block_new(cc->samplerate,cc->shortterm.ms,
        cc->shortterm.partition);

    if (NULL==collect->shortterm) {
      MESSAGE("creating shortterm block");
      goto shortterm;
    }

    lib1770_block_add_stats(collect->shortterm,cc->aggregate->shortterm);
    lib1770_pre_add_block(collect->pre,collect->shortterm);
  }

  return 0;
// cleanup:
  if (NULL!=collect->shortterm)
    lib1770_block_close(collect->shortterm);
shortterm:
  if (NULL!=collect->momentary)
    lib1770_block_close(collect->momentary);
momentary:
  lib1770_pre_close(collect->pre);
pre:
  return -1;
}

void ffsox_collect_cleanup(collect_t *collect)
{
  if (NULL!=collect->shortterm)
    lib1770_block_close(collect->shortterm);

  if (NULL!=collect->momentary)
    lib1770_block_close(collect->momentary);

  lib1770_pre_close(collect->pre);
}

///////////////////////////////////////////////////////////////////////////////
void ffsox_collect_flush(collect_t *collect)
{
  lib1770_pre_flush(collect->pre);
}

void ffsox_collect_channel(void *data, int ch, double x)
{
  collect_t *collect=data;
  aggregate_t *aggregate=collect->aggregate;

  if (FFMPEG_CHANNEL_IN_RANGE(ch,collect->channels))
    *collect->sp++=x;

  if (0!=(AGGREGATE_SAMPLEPEAK&aggregate->flags)) {
    if (x<0)
      x=-x;

    if (aggregate->samplepeak<x)
      aggregate->samplepeak=x;
  }
}

void ffsox_collect_sample(void *data)
{
  collect_t *collect=data;

  lib1770_pre_add_sample(collect->pre,collect->sample);
  collect->sp=collect->sample;
}

void ffsox_collect_truepeak(void *data, double x)
{
  collect_t *collect=data;
  aggregate_t *aggregate=collect->aggregate;

  if (0!=(AGGREGATE_TRUEPEAK&aggregate->flags)) {
    x*=collect->invscale;

    if (aggregate->truepeak<x)
      aggregate->truepeak=x;
  }
}
