/*
 * bs1770gain_sink.c
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

#define BS1770GAIN_SINK
#if defined (BS1770GAIN_SINK) // {
///////////////////////////////////////////////////////////////////////////////
typedef struct bs1770gain_sink bs1770gain_sink_t;

struct bs1770gain_sink {
  bs1770gain_read_t *read;
  sox_uint64_t delta;
  sox_uint64_t count;
  int percent;
};
#endif // }

///////////////////////////////////////////////////////////////////////////////
static void bs1770gain_print(bs1770gain_sink_t *sink, FILE *f)
{
  char buf[32];
  int i;

  sprintf(buf,"%d%%",sink->percent);
  fputs(buf,f);
  fflush(f);
  i=strlen(buf);

  while (0<i) {
    fputc('\b',f);
    --i;
  }
}

static int bs1770gain_sink_getopts(sox_effect_t *effp, int argc,
    char *argv[])
{
#if defined (BS1770GAIN_SINK) // {
  bs1770gain_sink_t *sink=(bs1770gain_sink_t *)effp->priv;
#else // } {
  bs1770gain_read_t **read=(bs1770gain_read_t **)effp->priv;
#endif // }

  BS1770GAIN_GOTO(argc<2,"missing argument",argc);
#if defined (BS1770GAIN_SINK) // {
  sink->read=(bs1770gain_read_t *)argv[1];
  sink->delta=0;
  sink->count=0ull;
  sink->percent=0;
#else // } {
  *read=(bs1770gain_read_t *)argv[1];
#endif // }

  return SOX_SUCCESS;
argc:
  return SOX_EOF;
}

#if defined (BS1770GAIN_SINK) // {
static int bs1770gain_sink_start(sox_effect_t * effp)
{
  bs1770gain_sink_t *sink=(bs1770gain_sink_t *)effp->priv;
  FILE *f;

  sink->delta=effp->in_signal.length/100;
  sink->count=0ull;
  sink->percent=0;

  if (stdout==(f=sink->read->options->f))
    bs1770gain_print(sink,f);

  return SOX_SUCCESS;
}
#endif // }

static int bs1770gain_sink_flow(sox_effect_t *effp, sox_sample_t const *ibuf,
    sox_sample_t *obuf, size_t *isamp, size_t *osamp)
{
#if defined (BS1770GAIN_SINK) // {
  bs1770gain_sink_t *sink=(bs1770gain_sink_t *)effp->priv;
  bs1770gain_read_t *read=sink->read;
  sox_uint64_t delta=sink->delta;
  sox_uint64_t count=sink->count;
  FILE *f=sink->read->options->f;
#else // } {
  bs1770gain_read_t *read=*(bs1770gain_read_t **)effp->priv;
#endif // }
  bs1770gain_head_t *head=read->head;
  bs1770gain_stats_t *stats=NULL==head?NULL:head->stats;
  const sox_sample_t *rp,*mp;
  double peak_t;
  sox_sample_t x;
  double y;

  rp=ibuf;
  mp=rp+*isamp;

  if (NULL!=stats&&0<=stats->peak_t) {
    peak_t=stats->peak_t;

    while (rp<mp) {
      x=*rp;

      if (x<0)
        x=-(x+1);

      if (0<x&&peak_t<(y=4.0*x/INT32_MAX))
        peak_t=y;

      ++rp;
#if defined (BS1770GAIN_SINK) // {
      if (stdout==f&&(delta==++count)) {
        count=0;
        ++sink->percent;
        bs1770gain_print(sink,f);
      }
#endif // }
    }

    stats->peak_t=peak_t;
  }
  else {
    while (rp<mp) {
      ++rp;

      if (stdout==f&&(delta==++count)) {
        count=0;
        ++sink->percent;
        bs1770gain_print(sink,f);
      }
    }
  }

  sink->count=count;

  // Outputting is the last `effect' in the effect chain so always passes
  // 0 samples on to the next effect (as there isn't one!)
  *osamp=0;

  // All samples output successfully
  return SOX_SUCCESS;
}

sox_effect_handler_t const *bs1770gain_sink_handler(void)
{
  static sox_effect_handler_t handler = {
    // Effect name
    .name="bs1770gain sink",
    // Short explanation of parameters accepted by effect
    .usage=NULL,
    // Combination of SOX_EFF_* flags
    .flags=SOX_EFF_MCHAN,
    // Called to parse command-line arguments (called once per effect).
    .getopts=bs1770gain_sink_getopts,
    // Called to initialize effect (called once per flow).
#if defined (BS1770GAIN_SINK) // {
    .start=bs1770gain_sink_start,
#else // } {
    .start=NULL,
#endif // }
    // Called to process samples.
    .flow=bs1770gain_sink_flow,
    // Called to finish getting output after input is complete.
    .drain=NULL,
    // Called to shut down effect (called once per flow).
    .stop=NULL,
    // Called to shut down effect (called once per effect).
    .kill=NULL,
    // Size of private data SoX should pre-allocate for effect
#if defined (BS1770GAIN_SINK) // {
    .priv_size=sizeof (bs1770gain_sink_t)
#else // } {
    .priv_size=sizeof (bs1770gain_read_t *)
#endif // }
  };

  return &handler;
}
