/*
 * mux3.c
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

int main(int argc, char **argv)
{
  enum { FLAGS=AGGREGATE_ALL&~AGGREGATE_TRUEPEAK };
  //enum { FLAGS=AGGREGATE_ALL };
  //enum { FLAGS=0 };
  aggregate_t aggregate;
  analyze_config_t ac;

  if (argc<2) {
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

  if (ffsox_aggregate_create(&aggregate,FLAGS)<0) {
    MESSAGE("creating aggregator");
    goto aggregate;
  }

  memset(&ac,0,sizeof ac);
  ac.path=argv[1];
  ac.aggregate=&aggregate;
  ac.drc=0.0;
  ac.momentary.ms=400.0;
  ac.momentary.partition=4;
  ac.shortterm.ms=3000.0;
  ac.shortterm.partition=3;
  ac.f=stdout;
  ac.dump=1;

  if (ffsox_analyze(&ac)<0) {
    MESSAGE("analyzing");
    goto analyze;
  }

  if (NULL!=aggregate.momentary&&NULL!=aggregate.shortterm) {
    fprintf(stdout,"M %.1f, S %.1f, I %.1f, R %.1f\n",
        lib1770_stats_get_max(aggregate.momentary),
        lib1770_stats_get_max(aggregate.shortterm),
        lib1770_stats_get_mean(aggregate.momentary,-10.0),
        lib1770_stats_get_range(aggregate.shortterm,-20.0,0.1,0.95));
  }
  else if (NULL!=aggregate.momentary) {
    fprintf(stdout,"M %.1f, I %.1f\n",
        lib1770_stats_get_max(aggregate.momentary),
        lib1770_stats_get_mean(aggregate.momentary,-10.0));
  }
  else if (NULL!=aggregate.shortterm) {
    fprintf(stdout,"S %.1f, R %.1f\n",
        lib1770_stats_get_max(aggregate.shortterm),
        lib1770_stats_get_range(aggregate.shortterm,-20.0,0.1,0.95));
  }
  else
    fprintf(stdout,"Sorry, no statistics available.\n");

// cleanup:
  ffsox_aggregate_cleanup(&aggregate);
analyze:
aggregate:
  sox_quit();
sox:
load:
usage:
  return 0;
}
