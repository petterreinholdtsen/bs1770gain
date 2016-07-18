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
//#include <endian.h>

#define DBFMT "%.2f"

///////////////////////////////////////////////////////////////////////////////
static const bs1770gain_print_vmt_t *get_vmt(void);
static void bs1770gain_xml_puts(const char *rp, FILE *f);

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
  else {
#if 0 // {
    fprintf(p->f,"  <album folder=\"%s\">\n",ibasename);
#else // } {
    fputs("  <album folder=\"",p->f);
    //fputs(ibasename,p->f);
    bs1770gain_xml_puts(ibasename,p->f);
    fputs("\">\n",p->f);
#endif // }
  }

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
#if 0 // {
    fprintf(p->f,"    <track total=\"%d\" number=\"%d\" file=\"%s\">\n",
        p->a->n,t->n,pbu_basename(t->ipath));
#else // } {
    fprintf(p->f,"    <track total=\"%d\" number=\"%d\" file=\"",
        p->a->n,t->n);
    //fputs(pbu_basename(t->ipath),p->f);
    bs1770gain_xml_puts(pbu_basename(t->ipath),p->f);
    fputs("\">\n",p->f);
#endif // }
  }

  fflush(p->f);
}

static void track_body(bs1770gain_print_t *p, aggregate_t *aggregate,
    const options_t *options)
{
  int flags=aggregate->flags;
  FILE *f=p->f;
  double norm=options->preamp+options->norm;
  double q,db;

  ////////
  if (0!=(flags&AGGREGATE_MOMENTARY_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->momentary,
        options->momentary.mean_gate);
#if defined (DBFMT) // {
    fprintf(f,"      <integrated lufs=\"" DBFMT "\" lu=\"" DBFMT "\" />\n",
        db,norm-db);
#else // } {
    fprintf(f,"      <integrated lufs=\"%.1f\" lu=\"%.1f\" />\n",db,norm-db);
#endif // }
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->momentary);
#if defined (DBFMT) // {
    fprintf(f,"      <momentary lufs=\"" DBFMT "\" lu=\"" DBFMT "\" />\n",
        db,norm-db);
#else // } {
    fprintf(f,"      <momentary lufs=\"%.1f\" lu=\"%.1f\" />\n",db,norm-db);
#endif // }
  }

  if (0!=(flags&AGGREGATE_MOMENTARY_RANGE)) {
    db=lib1770_stats_get_range(aggregate->momentary,
        options->momentary.range_gate,
        options->momentary.range_lower_bound,
        options->momentary.range_upper_bound);
#if defined (DBFMT) // {
    fprintf(f,"      <momentary-range lufs=\"" DBFMT "\" />\n",db);
#else // } {
    fprintf(f,"      <momentary-range lufs=\"%.1f\" />\n",db);
#endif // }
  }

  ////////
  if (0!=(flags&AGGREGATE_SHORTTERM_MEAN)) {
    db=lib1770_stats_get_mean(aggregate->shortterm,
        options->shortterm.mean_gate);
#if defined (DBFMT) // {
    fprintf(f,"      <shortterm-mean lufs=\"" DBFMT "\" lu=\"" DBFMT "\" />\n",
        db,norm-db);
#else // } {
    fprintf(f,"      <shortterm-mean lufs=\"%.1f\" lu=\"%.1f\" />\n",
        db,norm-db);
#endif // }
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_MAXIMUM)) {
    db=lib1770_stats_get_max(aggregate->shortterm);
#if defined (DBFMT) // {
    fprintf(f,"      <shortterm-maximum lufs=\"" DBFMT "\""
        " lu=\"" DBFMT "\" />\n",db,norm-db);
#else // } {
    fprintf(f,"      <shortterm-maximum lufs=\"%.1f\" lu=\"%.1f\" />\n",
        db,norm-db);
#endif // }
  }

  if (0!=(flags&AGGREGATE_SHORTTERM_RANGE)) {
    db=lib1770_stats_get_range(aggregate->shortterm,
        options->shortterm.range_gate,
        options->shortterm.range_lower_bound,
        options->shortterm.range_upper_bound);
#if defined (DBFMT) // {
    fprintf(f,"      <range lufs=\"" DBFMT "\" />\n",db);
#else // } {
    fprintf(f,"      <range lufs=\"%.1f\" />\n",db);
#endif // }
  }

  ////////
  if (0!=(flags&AGGREGATE_SAMPLEPEAK)) {
    q=aggregate->samplepeak;
    db=LIB1770_Q2DB(q);
#if defined (DBFMT) // {
    fprintf(f,"      <sample-peak spfs=\"" DBFMT "\" factor=\"%f\" />\n",db,q);
#else // } {
    fprintf(f,"      <sample-peak spfs=\"%.1f\" factor=\"%f\" />\n",db,q);
#endif // }
  }

  if (0!=(flags&AGGREGATE_TRUEPEAK)) {
    q=aggregate->truepeak;
    db=LIB1770_Q2DB(q);
#if defined (DBFMT) // {
    fprintf(f,"      <true-peak tpfs=\"" DBFMT "\" factor=\"%f\" />\n",db,q);
#else // } {
    fprintf(f,"      <true-peak tpfs=\"%.1f\" factor=\"%f\" />\n",db,q);
#endif // }
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

// http://zaemis.blogspot.de/2011/06/reading-unicode-utf-8-by-hand-in-c.html
#if 1 // {
static uint32_t bs1770gain_octet(const char **rpp)
{
  enum {
    MASK1=(1u<<7)|(1u<<6),
    MASK2=MASK1|(1u<<5),
    MASK3=MASK2|(1u<<4),
    MASK4=MASK3|(1u<<3),
  };

  const uint8_t *rp=(const uint8_t *)*rpp;
  uint32_t octet=0;
  int n;

  if (MASK1==(MASK2&*rp)) {
    n=1;
    octet|=((uint8_t)~MASK2)&*rp++;
  }
  else if (MASK2==(MASK3&*rp)) {
    n=2;
    octet|=((uint8_t)~MASK3)&*rp++;
  }
  else if (MASK3==(MASK4&*rp)) {
    n=3;
    octet|=((uint8_t)~MASK4)&*rp++;
  }
  else {
    n=0;
    octet|=*rp++;
  }

  while (0<n) {
    octet<<=6;
    octet|=((uint8_t)~MASK1)&*rp++;
    --n;
  }

  *rpp=(const char *)rp;

  return octet;
}
#else // } {
static uint32_t bs1770gain_octet(const char **rpp)
{
  const uint8_t *rp=(const uint8_t *)*rpp;
  uint32_t octet=0;
  uint32_t mask;
  int n;

  if ((1u<<7)&*rp) {
    if ((1u<<6)&*rp) {
      if ((1u<<5)&*rp) {
        if ((1u<<4)&*rp) {
          if ((1u<<3)&*rp)
            goto error;
          else {
            mask=~((1u<<7)|(1u<<6)|(1u<<5)|(1u<<4)|(1u<<3));
            octet|=mask&*rp++;
            n=3;
          }
        }
        else {
          mask=~((1u<<7)|(1u<<6)|(1u<<5)|(1u<<4));
          octet|=mask&*rp++;
          n=2;
        }
      }
      else {
        mask=~((1u<<7)|(1u<<6)|(1u<<5));
        octet|=mask&*rp++;
        n=1;
      }
    }
    else
      goto error;
  }
  else {
    n=0;
    octet|=*rp++;
    goto end;
  }

  mask=~((1u<<7)|(1u<<6)|(1u<<5));

  while (0<n) {
    octet<<=6;
    octet|=mask&*rp++;
    --n;
  }
end:
  *rpp=(const char *)rp;
error:

  return octet;
}
#endif // }

static void bs1770gain_xml_puts(const char *rp, FILE *f)
{
  uint32_t octet;

  while (*rp) {
    octet=bs1770gain_octet(&rp);

    if ('0'<=octet&&octet<='9')
      fputc(octet,f);
    else if ('A'<=octet&&octet<='Z')
      fputc(octet,f);
    else if ('a'<=octet&&octet<='z')
      fputc(octet,f);
    else
      fprintf(f,"&#x%X;",octet);
  }
}
