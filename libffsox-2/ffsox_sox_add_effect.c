/*
 * ffsox_sox_add_effect.c
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
#define FFSOX_ANALYZE_RATE 192000.0

int ffsox_sox_add_effect(sox_effect_t *e, sox_effects_chain_t *chain,
    sox_signalinfo_t *signal_in, sox_signalinfo_t const *signal_out,
    int n, char *opts[])
{
  if (SOX_SUCCESS!=sox_effect_options(e,n,opts)) {
    MESSAGE("setting options to SoX effect");
    return SOX_EOF;
  }

  if (SOX_SUCCESS!=sox_add_effect(chain,e,signal_in,signal_out)) {
    MESSAGE("adding SoX effect to SoX chain");
    return SOX_EOF;
  }

  return SOX_SUCCESS;
}

int ffsox_sox_add_effect_fn(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal_in, sox_signalinfo_t const *signal_out,
    int n, char *opts[], sox_effect_fn_t fn)
{
  int code=SOX_EOF;
  sox_effect_t *e;

  if (NULL==(e=sox_create_effect(fn()))) {
    MESSAGE("creating effect");
    goto create;
  }

  code=ffsox_sox_add_effect(e,chain,signal_in,signal_out,n,opts);
//cleanup:
  free(e);
create:
  return code;
}

int ffsox_sox_add_effect_name(sox_effects_chain_t *chain,
    sox_signalinfo_t *signal_in, sox_signalinfo_t const *signal_out,
    int n, char *opts[], const char *name)
{
  int code=SOX_EOF;
  sox_effect_t *e;

  if (NULL==(e=sox_create_effect(sox_find_effect(name)))) {
    MESSAGE("creating effect");
    goto create;
  }

  code=ffsox_sox_add_effect(e,chain,signal_in,signal_out,n,opts);
//cleanup:
  free(e);
create:
  return code;
}
