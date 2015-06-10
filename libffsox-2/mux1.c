/*
 * mux1.c
 * Copyright (C) 2014, 2015 Peter Belkner <pbelkner@snafu.de>
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
#include <ffsox.h>

int main(int argc, char **argv)
{
  enum { CODEC_ID=AV_CODEC_ID_FLAC,SAMPLE_FMT=AV_SAMPLE_FMT_S32 };
  double q;
  ffsox_source_t si;
  ffsox_sink_t so;
  ffsox_machine_t m;

  if (argc<3) {
    PBU_MESSAGE("usage");
    goto usage;
  }

  if (ffsox_dynload(NULL)<0) {
    PBU_MESSAGE("loading shared libraries");
    goto load;
  }

  av_register_all();
  q=3<argc?1.0:-1.0;

  if (ffsox_source_create(&si,argv[1],-1,-1,NULL,NULL)<0) {
    PBU_MESSAGE("creating source");
    goto si;
  }

  av_dump_format(si.f.fc,0,si.f.path,0);

  if (ffsox_sink_create(&so,argv[2])<0) {
    PBU_MESSAGE("creating sink");
    goto so;
  }

  if (ffsox_source_link_create(&si,&so,0.0,CODEC_ID,SAMPLE_FMT,q)<0) {
    PBU_MESSAGE("creating link");
    goto link;
  }

  av_dump_format(so.f.fc,0,so.f.path,1);

  if (ffsox_sink_open(&so)<0) {
    PBU_MESSAGE("opening sink");
    goto open;
  }

  if (ffsox_machine_run(&m,&si.node)<0) {
    PBU_MESSAGE("running machine");
    goto machine;
  }

  fprintf(stderr,"yep!\n");
// cleanup:
machine:
  ffsox_sink_close(&so);
open:
  ffsox_source_link_cleanup(&si);
link:
  ffsox_sink_cleanup(&so);
so:
  si.vmt->cleanup(&si);
si:
load:
usage:
  return 0;
}
