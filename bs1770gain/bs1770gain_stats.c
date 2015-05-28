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

  if (0==options->integrated&&0==options->momentary)
    stats->stats_im=NULL;
  else {
    // open a history statistics for integrated loudness.
    BS1770GAIN_GOTO(NULL==(stats->stats_im=lib1770_stats_new()),
        "allocating bs.1770 intergrated loudness statistics",stats_im);
  }

  if (0==options->range&&0==options->shortterm)
    stats->stats_rs=NULL;
  else {
    // open a history statistics for integrated loudness.
    BS1770GAIN_GOTO(NULL==(stats->stats_rs=lib1770_stats_new()),
        "allocating bs.1770 intergrated loudness statistics",stats_rs);
  }

  stats->peak_s=0==options->samplepeak?-1.0:0.0;
  stats->peak_t=0==options->truepeak?-1.0:0.0;

  return stats;
// cleanup:
  if (NULL!=stats->stats_rs)
    lib1770_stats_close(stats->stats_rs);
stats_rs:
  if (NULL!=stats->stats_im)
    lib1770_stats_close(stats->stats_im);
stats_im:
  free(stats);
stats:
  return NULL;
}

void bs1770gain_stats_close(bs1770gain_stats_t *stats)
{
  if (NULL!=stats->stats_rs)
    lib1770_stats_close(stats->stats_rs);

  if (NULL!=stats->stats_im)
    lib1770_stats_close(stats->stats_im);

  free(stats);
}

double bs1770gain_stats_get_loudness(const bs1770gain_stats_t *stats,
    const bs1770gain_options_t *options)
{
  switch (options->method) {
  case BS1770GAIN_METHOD_INTEGRATED:
    return stats->stats_im==NULL
        ?LIB1770_SILENCE
        :lib1770_stats_get_mean(stats->stats_im,-10.0);
  case BS1770GAIN_METHOD_SHORTTERM:
    return stats->stats_rs==NULL
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(stats->stats_rs);
  case BS1770GAIN_METHOD_MOMENTARY:
    return stats->stats_im==NULL
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(stats->stats_im);
  default:
    return LIB1770_SILENCE;
  }
}

void bs1770gain_stats_merge(bs1770gain_stats_t *lhs, bs1770gain_stats_t *rhs)
{
  if (NULL!=lhs->stats_im&&NULL!=rhs->stats_im)
    lib1770_stats_merge(lhs->stats_im,rhs->stats_im);

  if (NULL!=lhs->stats_rs&&NULL!=rhs->stats_rs)
    lib1770_stats_merge(lhs->stats_rs,rhs->stats_rs);

  if (0.0<=lhs->peak_s&&0.0<=rhs->peak_s&&lhs->peak_s<rhs->peak_s)
    lhs->peak_s=rhs->peak_s;

  if (0.0<=lhs->peak_t&&0.0<=rhs->peak_t&&lhs->peak_t<rhs->peak_t)
    lhs->peak_t=rhs->peak_t;
}

#if 0 // {
void bs1770gain_stats_print(bs1770gain_stats_t *stats,
    bs1770gain_options_t *options)
{
  FILE *f=options->f;
  double level=options->preamp+options->level;
  double q,db;

  if (0!=options->integrated) {
    db=lib1770_stats_get_mean(stats->stats_im,-10.0);
    fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->momentary) {
    db=lib1770_stats_get_max(stats->stats_im);
    fprintf(f,"        momentary:  %.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->shortterm) {
    db=lib1770_stats_get_max(stats->stats_rs);
    fprintf(f,"       shortterm:  %.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->range) {
    db=lib1770_stats_get_range(stats->stats_rs,-20.0,0.1,0.95);
    fprintf(f,"            range:  %.1f LUFS\n",db);
  }

  if (0<=stats->peak_s) {
    q=stats->peak_s;
    db=LIB1770_Q2DB(q);
    fprintf(f,"      sample peak:  %.1f SPFS / %f\n",db,q);
  }

  if (0<=stats->peak_t) {
    q=stats->peak_t;
    db=LIB1770_Q2DB(q);
    fprintf(f,"        true peak:  %.1f TPFS / %f\n",db,q);
  }
}
#else // } {
static int bs1770gain_stats_width(bs1770gain_stats_t *stats,
    bs1770gain_options_t *options)
{
  int width=0;
  int len;

  if (0!=options->integrated) {
    if ((len=strlen("integrated")))
      width=len;
  }

  if (0!=options->momentary) {
    if (width<(len=strlen("momentary")))
      width=len;
  }

  if (0!=options->shortterm) {
    if (width<(len=strlen("shortterm")))
      width=len;
  }

  if (0!=options->range) {
    if (width<(len=strlen("range")))
      width=len;
  }

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
    bs1770gain_options_t *options)
{
  FILE *f=options->f;
  double level=options->preamp+options->level;
  int width=bs1770gain_stats_width(stats,options);
  double q,db;

  if (0!=options->integrated) {
    db=lib1770_stats_get_mean(stats->stats_im,-10.0);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_stats_label("integrated",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->shortterm) {
    db=lib1770_stats_get_max(stats->stats_rs);
    //fprintf(f,"       shortterm:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_stats_label("shortterm",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->momentary) {
    db=lib1770_stats_get_max(stats->stats_im);
    //fprintf(f,"        momentary:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_stats_label("momentary",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=options->range) {
    db=lib1770_stats_get_range(stats->stats_rs,-20.0,0.1,0.95);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_stats_label("range",width,f);
    fprintf(f,"%.1f LUFS\n",db);
  }

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
#endif // }

///////////////////////////////////////////////////////////////////////////////
bs1770gain_head_t *bs1770gain_head_new(bs1770gain_stats_t *stats,
    double sample_rate, int channels,
    const bs1770gain_options_t *options)
{
  bs1770gain_head_t *head;

  BS1770GAIN_GOTO(NULL==(head=malloc(sizeof *head)),
      "allocating pre-filter",head);

  head->stats=stats;

  if (NULL==stats->stats_im)
    head->block_04=NULL;
  else {
    // open a 0.4 s / 0.75 overlap block for integrated and momentary
    // loudness.
    BS1770GAIN_GOTO(NULL==(head->block_04=lib1770_block_new(sample_rate,
        400.0,4)),"allocation bs.1770 block",block_04);
    // add the integrated and the momentary statistics to the respective
    // block.
    lib1770_block_add_stats(head->block_04,stats->stats_im);
  }

  if (NULL==stats->stats_rs)
    head->block_30=NULL;
  else {
    // open a 3 ms / 0.66 overlap block for loudness range and shortterm
    // loudness.
    BS1770GAIN_GOTO(NULL==(head->block_30=lib1770_block_new(sample_rate,
        400.0,4)),"allocation bs.1770 block",block_30);
    // add the loudness range and the sort term statistics to the respective
    // block.
    lib1770_block_add_stats(head->block_30,stats->stats_rs);
  }

  /////////////////////////////////////////////////////////////////////////////
  if (NULL==head->block_04&&NULL==head->block_30)
    head->pre=NULL;
  else {
    // open a pre-filter.
    if (options->mono2stereo&&1==channels)
      channels=2;

    BS1770GAIN_GOTO(NULL==(head->pre=lib1770_pre_new(sample_rate,channels)),
        "allocating the bs.1770 pre-filder",pre);

    // add the 0.4 s / 0.75 overlap and the 3 ms / 0.66 overlap block to the
    // pre-filter.
    if (NULL!=head->block_04)
      lib1770_pre_add_block(head->pre,head->block_04);

    if (NULL!=head->block_30)
      lib1770_pre_add_block(head->pre,head->block_30);
  }

  return head;
// cleanup:
  if (NULL!=head->pre)
    lib1770_pre_close(head->pre);
pre:
  if (NULL!=head->block_04)
    lib1770_block_close(head->block_04);
block_04:
  if (NULL!=head->block_30)
    lib1770_block_close(head->block_30);
block_30:
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

  if (NULL!=head->block_04)
    lib1770_block_close(head->block_04);

  if (NULL!=head->block_30)
    lib1770_block_close(head->block_30);

  free(head);
}

void bs1770gain_head_flush(bs1770gain_head_t *head)
{
  if (NULL!=head->pre)
    lib1770_pre_flush(head->pre);
}
