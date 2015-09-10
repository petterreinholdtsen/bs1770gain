/*
 * bs1770gain_aggregate.c
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
#include <bs1770gain_priv.h>

double bs1770gain_aggregate_get_loudness(const aggregate_t *aggregate,
    const options_t *options)
{
  switch (options->method) {
  case BS1770GAIN_METHOD_MOMENTARY_MEAN:
    return NULL==aggregate->momentary
        ?LIB1770_SILENCE
        :lib1770_stats_get_mean(aggregate->momentary,
            options->momentary.mean_gate);
  case BS1770GAIN_METHOD_MOMENTARY_MAXIMUM:
    return NULL==aggregate->momentary
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(aggregate->momentary);
  case BS1770GAIN_METHOD_SHORTTERM_MEAN:
    return NULL==aggregate->shortterm
        ?LIB1770_SILENCE
        :lib1770_stats_get_mean(aggregate->shortterm,
            options->shortterm.mean_gate);
  case BS1770GAIN_METHOD_SHORTTERM_MAXIMUM:
    return NULL==aggregate->shortterm
        ?LIB1770_SILENCE
        :lib1770_stats_get_max(aggregate->shortterm);
  default:
    return LIB1770_SILENCE;
  }
}
