/*
 * bs1770gain_stats.c
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
bs1770gain_stats_t *bs1770gain_stats_new(const bs1770gain_options_t *options)
{
  bs1770gain_stats_t *stats;

  /////////////////////////////////////////////////////////////////////////////
  BS1770GAIN_GOTO(NULL==(stats=malloc(sizeof *stats)),
      "allocating stats",stats);

  if (BS1770GAIN_BLOCK_OPTIONS_EMPTY(&options->momentary))
    stats->momentary=NULL;
  else {
    // open a history statistics for integrated loudness.
    BS1770GAIN_GOTO(NULL==(stats->momentary=lib1770_stats_new()),
        "allocating bs.1770 intergrated loudness statistics",momentary);
  }

  if (BS1770GAIN_BLOCK_OPTIONS_EMPTY(&options->shortterm))
    stats->shortterm=NULL;
  else {
    // open a history statistics for integrated loudness.
    BS1770GAIN_GOTO(NULL==(stats->shortterm=lib1770_stats_new()),
        "allocating bs.1770 intergrated loudness statistics",shortterm);
  }

  stats->peak_s=0==options->samplepeak?-1.0:0.0;
  stats->peak_t=0==options->truepeak?-1.0:0.0;

  return stats;
// cleanup:
  if (NULL!=stats->shortterm)
    lib1770_stats_close(stats->shortterm);
shortterm:
  if (NULL!=stats->momentary)
    lib1770_stats_close(stats->momentary);
momentary:
  free(stats);
stats:
  return NULL;
}

void bs1770gain_stats_close(bs1770gain_stats_t *stats)
{
  if (NULL!=stats->shortterm)
    lib1770_stats_close(stats->shortterm);

  if (NULL!=stats->momentary)
    lib1770_stats_close(stats->momentary);

  free(stats);
}

double bs1770gain_stats_get_loudness(const bs1770gain_stats_t *stats,
    const bs1770gain_options_t *options)
{
  switch (options->method) {
  case BS1770GAIN_METHOD_MOMENTARY_MEAN:
    return stats->momentary==NULL
        ?LIB1770_SILENCE
        :lib1770_stats_get_mean(stats->momentary,options->momentary.mean_gate);
  case BS1770GAIN_METHOD_MOMENTARY_MAXIMUM:
    return stats->momentary==NULL
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(stats->momentary);
  case BS1770GAIN_METHOD_SHORTTERM_MEAN:
    return stats->shortterm==NULL
        ?LIB1770_SILENCE
        :lib1770_stats_get_mean(stats->shortterm,options->shortterm.mean_gate);
  case BS1770GAIN_METHOD_SHORTTERM_MAXIMUM:
    return stats->shortterm==NULL
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(stats->shortterm);
  default:
    return LIB1770_SILENCE;
  }
}

void bs1770gain_stats_merge(bs1770gain_stats_t *lhs, bs1770gain_stats_t *rhs)
{
  if (NULL!=lhs->momentary&&NULL!=rhs->momentary)
    lib1770_stats_merge(lhs->momentary,rhs->momentary);

  if (NULL!=lhs->shortterm&&NULL!=rhs->shortterm)
    lib1770_stats_merge(lhs->shortterm,rhs->shortterm);

  if (0.0<=lhs->peak_s&&0.0<=rhs->peak_s&&lhs->peak_s<rhs->peak_s)
    lhs->peak_s=rhs->peak_s;

  if (0.0<=lhs->peak_t&&0.0<=rhs->peak_t&&lhs->peak_t<rhs->peak_t)
    lhs->peak_t=rhs->peak_t;
}

static int bs1770gain_stats_width(bs1770gain_stats_t *stats,
    const bs1770gain_options_t *options)
{
  int width=0;
  int len;

  ////////
  if (0!=options->momentary.mean) {
    if ((len=strlen("integrated")))
      width=len;
  }

  if (0!=options->momentary.maximum) {
    if (width<(len=strlen("momentary maximum")))
      width=len;
  }

  if (0!=options->momentary.range) {
    if (width<(len=strlen("momentary range")))
      width=len;
  }

  ////////
  if (0!=options->shortterm.mean) {
    if (width<(len=strlen("shortterm mean")))
      width=len;
  }

  if (0!=options->shortterm.maximum) {
    if (width<(len=strlen("shortterm maximum")))
      width=len;
  }

  if (0!=options->shortterm.range) {
    if (width<(len=strlen("range")))
      width=len;
  }

  ////////
  if (0<=stats->peak_s) {
    if (width<(len=strlen("sample peak")))
      width=len;
  }

  if (0<=stats->peak_t) {
    if (width<(len=strlen("true peak")))
      width=len;
  }

  return width;
}

static void bs1770gain_stats_label(const char *label, int width, FILE *f)
{
  width+=6;

  for (width-=strlen(label);0<width;--width)
    fputc(' ',f);

  fprintf(f,"%s:  ",label);
}

void bs1770gain_stats_print(bs1770gain_stats_t *stats,
    const bs1770gain_options_t *options)
{
  FILE *f=options->f;
  double level=options->preamp+options->level;
  int width=bs1770gain_stats_width(stats,options);
  double q,db;

  ////////
  if (0!=options->momentary.mean) {
    db=lib1770_stats_get_mean(stats->momentary,options->momentary.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_stats_label("integrated",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->momentary.maximum) {
    db=lib1770_stats_get_max(stats->momentary);
    //fprintf(f,"        momentary:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_stats_label("momentary maximum",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->momentary.range) {
    db=lib1770_stats_get_range(stats->momentary,
        options->momentary.range_gate,
        options->momentary.range_lower_bound,
        options->momentary.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_stats_label("momentary range",width,f);
    fprintf(f,"%.1f LUFS\n",db);
  }

  ////////
  if (0!=options->shortterm.mean) {
    db=lib1770_stats_get_mean(stats->shortterm,options->shortterm.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_stats_label("shortterm mean",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->shortterm.maximum) {
    db=lib1770_stats_get_max(stats->shortterm);
    //fprintf(f,"       shortterm:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_stats_label("shortterm maximum",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->shortterm.range) {
    db=lib1770_stats_get_range(stats->shortterm,
        options->shortterm.range_gate,
        options->shortterm.range_lower_bound,
        options->shortterm.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_stats_label("range",width,f);
    fprintf(f,"%.1f LUFS\n",db);
  }

  ////////
  if (0<=stats->peak_s) {
    q=stats->peak_s;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"      sample peak:  %.1f SPFS / %f\n",db,q);
    bs1770gain_stats_label("sample peak",width,f);
    fprintf(f,"%.1f SPFS / %f\n",db,q);
  }

  if (0<=stats->peak_t) {
    q=stats->peak_t;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"        true peak:  %.1f TPFS / %f\n",db,q);
    bs1770gain_stats_label("true peak",width,f);
    fprintf(f,"%.1f TPFS / %f\n",db,q);
  }
}

///////////////////////////////////////////////////////////////////////////////
bs1770gain_head_t *bs1770gain_head_new(bs1770gain_stats_t *stats,
    double sample_rate, int channels,
    const bs1770gain_options_t *options)
{
  bs1770gain_head_t *head;

  BS1770GAIN_GOTO(NULL==(head=malloc(sizeof *head)),
      "allocating pre-filter",head);

  head->stats=stats;

  if (NULL==stats->momentary)
    head->momentary=NULL;
  else {
    // open a 0.4 s / 0.75 overlap block for integrated and momentary
    // loudness.
    BS1770GAIN_GOTO(NULL==(head->momentary=lib1770_block_new(sample_rate,
        options->momentary.length,options->momentary.partition)),
        "allocation bs.1770 block",momentary);
    // add the integrated and the momentary statistics to the respective
    // block.
    lib1770_block_add_stats(head->momentary,stats->momentary);
  }

  if (NULL==stats->shortterm)
    head->shortterm=NULL;
  else {
    // open a 3 ms / 0.66 overlap block for loudness range and shortterm
    // loudness.
    BS1770GAIN_GOTO(NULL==(head->shortterm=lib1770_block_new(sample_rate,
        options->shortterm.length,options->shortterm.partition)),
        "allocation bs.1770 block",shortterm);
    // add the loudness range and the sort term statistics to the respective
    // block.
    lib1770_block_add_stats(head->shortterm,stats->shortterm);
  }

  /////////////////////////////////////////////////////////////////////////////
  if (NULL==head->momentary&&NULL==head->shortterm)
    head->pre=NULL;
  else {
    // open a pre-filter.
    if (options->mono2stereo&&1==channels)
      channels=2;

    BS1770GAIN_GOTO(NULL==(head->pre=lib1770_pre_new(sample_rate,channels)),
        "allocating the bs.1770 pre-filder",pre);

    // add the 0.4 s / 0.75 overlap and the 3 ms / 0.66 overlap block to the
    // pre-filter.
    if (NULL!=head->momentary)
      lib1770_pre_add_block(head->pre,head->momentary);

    if (NULL!=head->shortterm)
      lib1770_pre_add_block(head->pre,head->shortterm);
  }

  return head;
// cleanup:
  if (NULL!=head->pre)
    lib1770_pre_close(head->pre);
pre:
  if (NULL!=head->momentary)
    lib1770_block_close(head->momentary);
momentary:
  if (NULL!=head->shortterm)
    lib1770_block_close(head->shortterm);
shortterm:
  free(head);
head:
  return NULL;
}

void bs1770gain_head_close(bs1770gain_head_t *head)
{
  if (NULL!=head->pre) {
    lib1770_pre_flush(head->pre);
    lib1770_pre_close(head->pre);
  }

  if (NULL!=head->momentary)
    lib1770_block_close(head->momentary);

  if (NULL!=head->shortterm)
    lib1770_block_close(head->shortterm);

  free(head);
}

void bs1770gain_head_flush(bs1770gain_head_t *head)
{
  if (NULL!=head->pre)
    lib1770_pre_flush(head->pre);
}
