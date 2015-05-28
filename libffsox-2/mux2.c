/*
 * mux2.c
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

#define MUX2_SOX_READER
///////////////////////////////////////////////////////////////////////////////
typedef struct input input_t;
typedef struct output output_t;

///////////////////////////////////////////////////////////////////////////////
struct input {
  sox_reader_t *in;
};

static int input_getopts(sox_effect_t *e, int argc, char *argv[])
{
  input_t *priv=e->priv;

  if (argc<2) {
    MESSAGE("missing argument");
    goto argc;
  }

  priv->in=(sox_reader_t *)argv[1];

  return SOX_SUCCESS;
argc:
  return SOX_EOF;
}

static int input_drain(sox_effect_t *e, sox_sample_t *obuf, size_t *osamp)
{
  input_t *priv=e->priv;
  sox_reader_t *in=priv->in;

  /* ensure that *osamp is a multiple of the number of channels. */
  *osamp-=*osamp%e->out_signal.channels;

  /* Read up to *osamp samples into obuf; store the actual number read
   * back to *osamp */
  *osamp=ffsox_sox_reader_read(in,obuf,*osamp);

  /* sox_read may return a number that is less than was requested; only if
   * 0 samples is returned does it indicate that end-of-file has been reached
   * or an error has occurred */
  if (0==*osamp&&0!=in->sox_errno)
    MESSAGE("reading");

  return 0==*osamp?SOX_EOF:SOX_SUCCESS;
}

static sox_effect_handler_t const *input_handler(void)
{
  static sox_effect_handler_t handler={
    .name="input",
    .usage=NULL,
    .flags=SOX_EFF_MCHAN,
    .getopts=input_getopts,
    .start=NULL,
    .flow=NULL,
    .drain=input_drain,
    .stop=NULL,
    .kill=NULL,
    .priv_size=sizeof (input_t)
  };

  return &handler;
}

///////////////////////////////////////////////////////////////////////////////
struct output {
  sox_format_t *out;
};

static int output_getopts(sox_effect_t *e, int argc, char *argv[])
{
  output_t *priv=e->priv;

  if (argc<2) {
    MESSAGE("missing argument");
    goto argc;
  }

  priv->out=(sox_format_t *)argv[1];

  return SOX_SUCCESS;
argc:
  return SOX_EOF;
}

static int output_flow(sox_effect_t *e, sox_sample_t const *ibuf,
    sox_sample_t *obuf, size_t * isamp, size_t * osamp)
{
  output_t *priv=e->priv;
  sox_format_t *out=priv->out;
  size_t len;

  /* Write out *isamp samples */
  len=sox_write(out, ibuf, *isamp);

  /* len is the number of samples that were actually written out; if this is
   * different to *isamp, then something has gone wrong--most often, it's
   * out of disc space */
  if (len != *isamp) {
    fprintf(stderr, "%s: %s\n", out->filename, out->sox_errstr);
    return SOX_EOF;
  }

  /* Outputting is the last `effect' in the effect chain so always passes
   * 0 samples on to the next effect (as there isn't one!) */
  *osamp = 0;

  return SOX_SUCCESS; /* All samples output successfully */
}

static sox_effect_handler_t const *output_handler(void)
{
  static sox_effect_handler_t handler={
    .name="output",
    .usage=NULL,
    .flags=SOX_EFF_MCHAN,
    .getopts=output_getopts,
    .start=NULL,
    .flow=output_flow,
    .drain=NULL,
    .stop=NULL,
    .kill=NULL,
    .priv_size=sizeof (output_t)
  };

  return &handler;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  source_t si;
  frame_reader_t *fr;
  sox_reader_t *in;
  sox_format_t *out;
  sox_effects_chain_t *chain;
  sox_effect_t *e;
  char *opts[1];

  if (argc<3) {
    MESSAGE("usage");
    goto usage;
  }

  if (ffsox_dynload(NULL)<0) {
    MESSAGE("loading shared libraries");
    goto load;
  }

  if (SOX_SUCCESS!=sox_init()) {
    MESSAGE("initializing SoX");
    goto sox;
  }

  av_register_all();

  // create the chain.
  if (ffsox_source_create(&si,argv[1],-1,-1,NULL,NULL)<0) {
    MESSAGE("creating source");
    goto si;
  }

  av_dump_format(si.f.fc,0,si.f.path,0);

  if (NULL==(fr=ffsox_frame_reader_new(&si,si.ai,0.0))) {
    MESSAGE("creating frame reader");
    goto fr;
  }

  if (NULL==(in=ffsox_sox_reader_new(fr,1.0,NULL))) {
    MESSAGE("creating sox addapter");
    goto sa;
  }

  if (NULL==(out=sox_open_write(argv[2],&in->signal,NULL,NULL,NULL,NULL))) {
    MESSAGE("opening output");
    goto open_out;
  }

  if (NULL==(chain=sox_create_effects_chain(&in->encoding,&out->encoding))) {
    MESSAGE("creating chain");
    goto chain;
  }

  // add the input effect.
  if (NULL==(e=sox_create_effect(input_handler()))) {
    MESSAGE("creating input effect");
    goto effect;
  }

  opts[0]=(char *)in;

  if (SOX_SUCCESS!=sox_effect_options(e,1,opts)) {
    MESSAGE("setting effect options");
    free(e);
    goto effect;
  }

  if (SOX_SUCCESS!=sox_add_effect(chain,e,&in->signal,&in->signal)) {
    MESSAGE("adding effect to chain");
    free(e);
    goto effect;
  }

  free(e);

  // add the output effect.
  if (NULL==(e=sox_create_effect(output_handler()))) {
    MESSAGE("creating output effect");
    goto effect;
  }

  opts[0]=(char *)out;

  if (SOX_SUCCESS!=sox_effect_options(e,1,opts)) {
    MESSAGE("setting effect options");
    free(e);
    goto effect;
  }

  if (SOX_SUCCESS!=sox_add_effect(chain,e,&in->signal,&in->signal)) {
    MESSAGE("adding effect to chain");
    free(e);
    goto effect;
  }

  free(e);

  // run the chain.
  if (SOX_SUCCESS!=sox_flow_effects(chain,NULL,NULL)) {
    MESSAGE("running effects chain");
    goto flow;
  }
// cleanup:
flow:
effect:
  sox_delete_effects_chain(chain);
chain:
  sox_close(out);
open_out:
sa:
fr:
  si.vmt->cleanup(&si);
si:
  sox_quit();
sox:
load:
usage:
  return 0;
}
