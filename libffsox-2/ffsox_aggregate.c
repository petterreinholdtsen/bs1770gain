/*
 * ffsox_aggregate.c
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

int ffsox_aggregate_create(aggregate_t *aggregate, int flags)
{
  aggregate->flags=flags;
  aggregate->samplepeak=0.0;
  aggregate->truepeak=0.0;
 
  if (0==(AGGREGATE_MOMENTARY&flags))
    aggregate->momentary=NULL;
  else if (NULL==(aggregate->momentary=lib1770_stats_new())) {
    DMESSAGE("creating momentary statistics");
    goto momentary;
  }

  if (0==(AGGREGATE_SHORTTERM&flags))
    aggregate->shortterm=NULL;
  else if (NULL==(aggregate->shortterm=lib1770_stats_new())) {
    DMESSAGE("creating shortterm statistics");
    goto shortterm;
  }

  return 0;
// cleanup:
  if (NULL!=aggregate->shortterm)
    lib1770_stats_close(aggregate->shortterm);
shortterm:
  if (NULL!=aggregate->momentary)
    lib1770_stats_close(aggregate->momentary);
momentary:
  return -1;
}

void ffsox_aggregate_cleanup(aggregate_t *aggregate)
{
  if (NULL!=aggregate->shortterm)
    lib1770_stats_close(aggregate->shortterm);

  if (NULL!=aggregate->momentary)
    lib1770_stats_close(aggregate->momentary);
}

int ffsox_aggregate_merge(aggregate_t *lhs, aggregate_t *rhs)
{
  int flags;

  if ((flags=lhs->flags)!=rhs->flags) {
    DMESSAGE("trying to merge incompatible aggregators");
    goto merge;
  }

  if (0!=(AGGREGATE_MOMENTARY&flags))
    lib1770_stats_merge(lhs->momentary,rhs->momentary);

  if (0!=(AGGREGATE_SHORTTERM&flags))
    lib1770_stats_merge(lhs->shortterm,rhs->shortterm);

  if (0!=(AGGREGATE_SAMPLEPEAK&flags)&&lhs->samplepeak<rhs->samplepeak)
    lhs->samplepeak=rhs->samplepeak;

  if (0!=(AGGREGATE_TRUEPEAK&flags)&&lhs->truepeak<rhs->truepeak)
    lhs->truepeak=rhs->truepeak;

  return 0;
merge:
  return -1;
}
