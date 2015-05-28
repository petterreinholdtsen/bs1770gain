/*
 * ffsox_sox_pull_handler.c
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

  priv->cb=1<argc?(pull_cb_t)argv[1]:NULL;
  priv->data=2<argc?(void *)argv[2]:NULL;

  return SOX_SUCCESS;
}

static int flow(sox_effect_t *e, sox_sample_t const *ibuf, sox_sample_t *obuf,
    size_t * isamp, size_t * osamp)
{
  priv_t *priv=e->priv;
  pull_cb_t cb=priv->cb;
  void *data;
  double scale;
  sox_sample_t const *rp,*mp;

  if (NULL!=cb) {
    data=priv->data;
    scale=1.0/MAXOF(*rp);
    rp=ibuf;
    mp=rp+*isamp;

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
  static sox_effect_handler_t handler={
    .name="priv",
    .usage=NULL,
    .flags=SOX_EFF_MCHAN,
    .getopts=getopts,
    .start=NULL,
    .flow=flow,
    .drain=NULL,
    .stop=NULL,
    .kill=NULL,
    .priv_size=sizeof (priv_t)
  };

  return &handler;
}

