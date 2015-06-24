/*
 * ffsox_sox_pull_handler.c
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
  pull_cb_t cb;
  void *data;
};

///////////////////////////////////////////////////////////////////////////////
static int getopts(sox_effect_t *e, int argc, char *argv[])
{
  priv_t *priv=e->priv;

  priv->cb=1<argc?(void *)argv[1]:NULL;
  priv->data=2<argc?(void *)argv[2]:NULL;

  return SOX_SUCCESS;
}

static int flow(sox_effect_t *e, sox_sample_t const *ibuf, sox_sample_t *obuf,
    size_t * isamp, size_t * osamp)
{
  priv_t *priv=e->priv;
  pull_cb_t cb=priv->cb;
  void *data;
  sox_sample_t const *rp,*mp;
  double scale;

  (void)obuf;

  if (NULL!=cb) {
    data=priv->data;
    rp=ibuf;
    mp=rp+*isamp;
    scale=1.0/MAXOF(*rp);

    while (rp<mp)
      cb(data,scale*(*rp++));
  }

  /* Outputting is the last `effect' in the effect chain so always passes
   * 0 samples on to the next effect (as there isn't one!) */
  *osamp = 0;

  return SOX_SUCCESS; /* All samples priv successfully */
}

sox_effect_handler_t const *ffsox_sox_pull_handler(void)
{
  static sox_effect_handler_t handler;

  if (NULL==handler.name) {
    handler.name="priv";
    handler.usage=NULL;
    handler.flags=SOX_EFF_MCHAN;
    handler.getopts=getopts;
    handler.start=NULL;
    handler.flow=flow;
    handler.drain=NULL;
    handler.stop=NULL;
    handler.kill=NULL;
    handler.priv_size=sizeof (priv_t);
  }

  return &handler;
}

