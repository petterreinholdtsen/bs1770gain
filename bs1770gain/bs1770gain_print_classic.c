/*
 * bs1770gain_print_classic.c
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

///////////////////////////////////////////////////////////////////////////////
static const bs1770gain_print_vmt_t *get_vmt(void);

///////////////////////////////////////////////////////////////////////////////
void bs1770gain_print_classic(bs1770gain_print_t *p, FILE *f)
{
  p->vmt=get_vmt();
  p->f=f;
  p->a=NULL;
  p->t=NULL;
}

static int bs1770gain_print_width(int flags)
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

static void bs1770gain_print_label(const char *label, int width, FILE *f)
{
  width+=6;

  for (width-=strlen(label);0<width;--width)
    fputc(' ',f);

  fprintf(f,"%s:  ",label);
}

///////////////////////////////////////////////////////////////////////////////
static void session_head(bs1770gain_print_t *p)
{
}

static FILE *session_file(bs1770gain_print_t *p)
{
  return stdout==p->f?p->f:NULL;
}

static void session_tail(bs1770gain_print_t *p)
{
}

////////
static void album_head(bs1770gain_print_t *p, bs1770gain_album_t *a,
    const char *ibasename)
{
  p->a=a;

  if (NULL==ibasename||0==*ibasename)
    fprintf(p->f,"analyzing ...\n");
  else
    fprintf(p->f,"analyzing \"%s\" ...\n",ibasename);
}

static void album_tail(bs1770gain_print_t *p)
{
  fprintf(p->f,"done.\n");
  p->a=NULL;
}

////////
static void track_head(bs1770gain_print_t *p, bs1770gain_track_t *t)
{
  p->t=t;

  if (NULL==t)
    fprintf(p->f,"  [ALBUM]:");
  else {
    fprintf(p->f,"  [%d/%d] \"%s\"",t->n,p->a->n,pbu_basename(t->ipath));
    fprintf(p->f,": ");
    fflush(p->f);
  }
}

static void track_body(bs1770gain_print_t *p, aggregate_t *aggregate,
    const options_t *options)
{
  int flags=aggregate->flags;
  FILE *f=p->f;
  double level=options->preamp+options->level;
  int width=bs1770gain_print_width(flags);
  double q,db;

  fprintf(p->f,p->t&&p->vmt->session.file(p)?"        \n":"\n");

  ////////
  if (0!=(flags&AGGREGATE_MOMENTARY_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->momentary,
        options->momentary.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_print_label("integrated",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->momentary);
    //fprintf(f,"        momentary:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_print_label("momentary maximum",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_RANGE)) {
    db=lib1770_stats_get_range(aggregate->momentary,
        options->momentary.range_gate,
        options->momentary.range_lower_bound,
        options->momentary.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_print_label("momentary range",width,f);
    fprintf(f,"%.1f LUFS\n",db);
  }

  ////////
  if (0!=(flags&AGGREGATE_SHORTTERM_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->shortterm,
        options->shortterm.mean_gate);
    //fprintf(f,"       integrated:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_print_label("shortterm mean",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->shortterm);
    //fprintf(f,"       shortterm:  %.1f LUFS / %.1f LU\n",db,level-db);
    bs1770gain_print_label("shortterm maximum",width,f);
    fprintf(f,"%.1f LUFS / %.1f LU\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_RANGE)) {
    db=lib1770_stats_get_range(aggregate->shortterm,
        options->shortterm.range_gate,
        options->shortterm.range_lower_bound,
        options->shortterm.range_upper_bound);
    //fprintf(f,"            range:  %.1f LUFS\n",db);
    bs1770gain_print_label("range",width,f);
    fprintf(f,"%.1f LUFS\n",db);
  }

  ////////
  if (0!=(flags&AGGREGATE_SAMPLEPEAK)) {
    q=aggregate->samplepeak;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"      sample peak:  %.1f SPFS / %f\n",db,q);
    bs1770gain_print_label("sample peak",width,f);
    fprintf(f,"%.1f SPFS / %f\n",db,q);
  }

  if (0!=(flags&AGGREGATE_TRUEPEAK)) {
    q=aggregate->truepeak;
    db=LIB1770_Q2DB(q);
    //fprintf(f,"        true peak:  %.1f TPFS / %f\n",db,q);
    bs1770gain_print_label("true peak",width,f);
    fprintf(f,"%.1f TPFS / %f\n",db,q);
  }
}

static void track_tail(bs1770gain_print_t *p)
{
  p->t=NULL;
}

////////
static const bs1770gain_print_vmt_t *get_vmt(void)
{
  static bs1770gain_print_vmt_t vmt;

  if (NULL==vmt.name) {
    vmt.name="bs1770gain_print_classic";
    vmt.session.head=session_head;
    vmt.session.file=session_file;
    vmt.session.tail=session_tail;
    vmt.album.head=album_head;
    vmt.album.tail=album_tail;
    vmt.track.head=track_head;
    vmt.track.body=track_body;
    vmt.track.tail=track_tail;
  }

  return &vmt;
}
