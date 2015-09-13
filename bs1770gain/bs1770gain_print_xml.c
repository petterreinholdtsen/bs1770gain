/*
 * bs1770gain_print_xml.c
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
void bs1770gain_print_xml(bs1770gain_print_t *p, FILE *f)
{
  p->vmt=get_vmt();
  p->f=f;
  p->a=NULL;
  p->t=NULL;
}

///////////////////////////////////////////////////////////////////////////////
static void session_head(bs1770gain_print_t *p)
{
  fprintf(p->f,"<bs1770gain>\n");
  fflush(p->f);
}

static FILE *session_file(bs1770gain_print_t *p)
{
  return NULL;
}

static void session_tail(bs1770gain_print_t *p)
{
  fprintf(p->f,"</bs1770gain>\n");
  fflush(p->f);
}

////////
static void album_head(bs1770gain_print_t *p, bs1770gain_album_t *a,
    const char *ibasename)
{
  p->a=a;

  if (NULL==ibasename||0==*ibasename)
    fprintf(p->f,"  <album>\n");
  else
    fprintf(p->f,"  <album folder=\"%s\">\n",ibasename);

  fflush(p->f);
}

static void album_tail(bs1770gain_print_t *p)
{
  p->a=NULL;
  fprintf(p->f,"  </album>\n");
  fflush(p->f);
}

////////
static void track_head(bs1770gain_print_t *p, bs1770gain_track_t *t)
{
  p->t=t;

  if (NULL==t)
    fprintf(p->f,"    <summary total=\"%d\">\n",p->a->n);
  else {
    fprintf(p->f,"    <track total=\"%d\" number=\"%d\" file=\"%s\">\n",
        p->a->n,t->n,pbu_basename(t->ipath));
  }

  fflush(p->f);
}

static void track_body(bs1770gain_print_t *p, aggregate_t *aggregate,
    const options_t *options)
{
  int flags=aggregate->flags;
  FILE *f=p->f;
  double level=options->preamp+options->level;
  double q,db;

  ////////
  if (0!=(flags&AGGREGATE_MOMENTARY_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->momentary,
        options->momentary.mean_gate);
    fprintf(f,"      <integrated lufs=\"%.1f\" lu=\"%.1f\" />\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->momentary);
    fprintf(f,"      <momentary lufs=\"%.1f\" lu=\"%.1f\" />\n",db,level-db);
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_RANGE)) {
    db=lib1770_stats_get_range(aggregate->momentary,
        options->momentary.range_gate,
        options->momentary.range_lower_bound,
        options->momentary.range_upper_bound);
    fprintf(f,"      <momentary-range lufs=\"%.1f\" />\n",db);
  }

  ////////
  if (0!=(flags&AGGREGATE_SHORTTERM_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->shortterm,
        options->shortterm.mean_gate);
    fprintf(f,"      <shortterm-mean lufs=\"%.1f\" lu=\"%.1f\" />\n",
        db,level-db);
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->shortterm);
    fprintf(f,"      <shortterm-maximum lufs=\"%.1f\" lu=\"%.1f\" />\n",
        db,level-db);
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_RANGE)) {
    db=lib1770_stats_get_range(aggregate->shortterm,
        options->shortterm.range_gate,
        options->shortterm.range_lower_bound,
        options->shortterm.range_upper_bound);
    fprintf(f,"      <range lufs=\"%.1f\" />\n",db);
  }

  ////////
  if (0!=(flags&AGGREGATE_SAMPLEPEAK)) {
    q=aggregate->samplepeak;
    db=LIB1770_Q2DB(q);
    fprintf(f,"      <sample-peak spfs=\"%.1f\" factor=\"%f\" />\n",db,q);
  }

  if (0!=(flags&AGGREGATE_TRUEPEAK)) {
    q=aggregate->truepeak;
    db=LIB1770_Q2DB(q);
    fprintf(f,"      <true-peak tpfs=\"%.1f\" factor=\"%f\" />\n",db,q);
  }

  fflush(p->f);
}

static void track_tail(bs1770gain_print_t *p)
{
  fprintf(p->f,p->t?"    </track>\n":"    </summary>\n");
  fflush(p->f);
  p->t=NULL;
}

////////
static const bs1770gain_print_vmt_t *get_vmt(void)
{
  static bs1770gain_print_vmt_t vmt;

  if (NULL==vmt.name) {
    vmt.name="bs1770gain_print_xml";
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
