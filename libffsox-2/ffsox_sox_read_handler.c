/*
 * ffsox_sox_read_handler.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@users.sf.net>
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
#include <ffsox_priv.h>

///////////////////////////////////////////////////////////////////////////////
typedef struct priv priv_t;

struct priv {
  sox_reader_t *read;
};

///////////////////////////////////////////////////////////////////////////////
static int getopts(sox_effect_t *e, int argc, char *argv[])
{
  priv_t *priv=e->priv;

  if (argc<2) {
    DMESSAGE("missing argument");
    goto argc;
  }

  priv->read=(void *)argv[1];

  return SOX_SUCCESS;
argc:
  return SOX_EOF;
}

static int drain(sox_effect_t *e, sox_sample_t *obuf, size_t *osamp)
{
  priv_t *priv=e->priv;
  sox_reader_t *read=priv->read;

  /* ensure that *osamp is a multiple of the number of channels. */
  *osamp-=*osamp%e->out_signal.channels;

  /* Read up to *osamp samples into obuf; store the actual number read
   * back to *osamp */
  *osamp=ffsox_sox_reader_read(read,obuf,*osamp);

#if defined (PBU_DEBUG) // {
  /* sox_read may return a number that is less than was requested; only if
   * 0 samples is returned does it indicate that end-of-file has been reached
   * or an error has occurred */
  if (0!=read->sox_errno)
    DMESSAGE("reading");
#endif // }

  return 0==*osamp?SOX_EOF:SOX_SUCCESS;
}

sox_effect_handler_t const *ffsox_sox_read_handler(void)
{
  static sox_effect_handler_t handler;

  if (NULL==handler.name) {
    handler.name="ffsox_sox_read";
    handler.usage=NULL;
    handler.flags=SOX_EFF_MCHAN;
    handler.getopts=getopts;
    handler.start=NULL;
    handler.flow=NULL;
    handler.drain=drain;
    handler.stop=NULL;
    handler.kill=NULL;
    handler.priv_size=sizeof (priv_t);
  }

  return &handler;
}
