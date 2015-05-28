/*
 * bs1770gain_aggregate.c
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
#include <bs1770gain_priv.h>

static int bs1770gain_aggregate_width(int flags)
{
  int width=0;
  int len;

  ////////
  if (0!=(flags&AGGREGATE_MOMENTARY_MEAN)) {
    if (width<(len=strlen("integrated")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_MAXIMUM)) {
    if (width<(len=strlen("momentary maximum")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_RANGE)) {
    if (width<(len=strlen("momentary range")))
      width=len;
  }

  ////////
  if (0!=(flags&AGGREGATE_SHORTTERM_MEAN)) {
    if (width<(len=strlen("shortterm mean")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_MAXIMUM)) {
    if (width<(len=strlen("shortterm maximum")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_RANGE)) {
    if (width<(len=strlen("range")))
      width=len;
  }

  ////////
  if (0!=(flags&AGGREGATE_SAMPLEPEAK)) {
    if (width<(len=strlen("sample peak")))
      width=len;
  }

  if (0!=(flags&AGGREGATE_TRUEPEAK)) {
    if (width<(len=strlen("true peak")))
      width=len;
  }

  return width;
}

static void bs1770gain_aggregate_label(const char *label, int width, FILE *f)
{
  width+=6;

  for (width-=strlen(label);0<width;--width)
    fputc(' ',f);

  fprintf(f,"%s:  ",label);
}

void bs1770gain_aggregate_print(aggregate_t *aggregate,
    const options_t *options)
{
  int flags=aggregate->flags;
  FILE *f=options->f;
  double level=options->preamp+options->level;
  int width=bs1770gain_aggregate_width(flags);
  double q,db;

  ////////
  if (0!=(flags&AGGREGATE_MOMENTARY_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->momentary,
        options->momentary.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_aggregate_label("integrated",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->momentary);
    //fprintf(f,"        momentary:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_aggregate_label("momentary maximum",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_RANGE)) {
    db=lib1770_stats_get_range(aggregate->momentary,
        options->momentary.range_gate,
        options->momentary.range_lower_bound,
        options->momentary.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_aggregate_label("momentary range",width,f);
    fprintf(f,"%.1f LUFS\n",db);
  }

  ////////
  if (0!=(flags&AGGREGATE_SHORTTERM_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->shortterm,
        options->shortterm.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_aggregate_label("shortterm mean",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->shortterm);
    //fprintf(f,"       shortterm:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_aggregate_label("shortterm maximum",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_RANGE)) {
    db=lib1770_stats_get_range(aggregate->shortterm,
        options->shortterm.range_gate,
        options->shortterm.range_lower_bound,
        options->shortterm.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_aggregate_label("range",width,f);
    fprintf(f,"%.1f LUFS\n",db);
  }

  ////////
  if (0!=(flags&AGGREGATE_SAMPLEPEAK)) {
    q=aggregate->samplepeak;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"      sample peak:  %.1f SPFS / %f\n",db,q);
    bs1770gain_aggregate_label("sample peak",width,f);
    fprintf(f,"%.1f SPFS / %f\n",db,q);
  }

  if (0!=(flags&AGGREGATE_TRUEPEAK)) {
    q=aggregate->truepeak;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"        true peak:  %.1f TPFS / %f\n",db,q);
    bs1770gain_aggregate_label("true peak",width,f);
    fprintf(f,"%.1f TPFS / %f\n",db,q);
  }
}

double bs1770gain_aggregate_get_loudness(const aggregate_t *aggregate,
    const options_t *options)
{
  switch (options->method) {
  case BS1770GAIN_METHOD_MOMENTARY_MEAN:
    return NULL==aggregate->momentary
        ?LIB1770_SILENCE
        :lib1770_stats_get_mean(aggregate->momentary,
            options->momentary.mean_gate);
  case BS1770GAIN_METHOD_MOMENTARY_MAXIMUM:
    return NULL==aggregate->momentary
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(aggregate->momentary);
  case BS1770GAIN_METHOD_SHORTTERM_MEAN:
    return NULL==aggregate->shortterm
        ?LIB1770_SILENCE
        :lib1770_stats_get_mean(aggregate->shortterm,
            options->shortterm.mean_gate);
  case BS1770GAIN_METHOD_SHORTTERM_MAXIMUM:
    return NULL==aggregate->shortterm
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(aggregate->shortterm);
  default:
    return LIB1770_SILENCE;
  }
}
