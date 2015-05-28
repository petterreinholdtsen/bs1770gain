/*
 * ffsox_source_link.c
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
#include <ffsox_priv.h>

static int ffsox_source_link_copy(source_t *si, sink_t *so, int stream_index)
{
  packet_writer_t *pw=NULL;

  if (NULL==(pw=ffsox_packet_writer_new(si,stream_index,so))) {
    MESSAGE("creating packet writer");
    goto pw;
  }

  return 0;
// cleanup:
  ffsox_node_destroy(&pw->node);
pw:
  return -1;
}

static int ffsox_source_link_codec(source_t *si, sink_t *so, int stream_index,
    double drc, int codec_id, int sample_fmt, double q)
{
  frame_reader_t *fr=NULL;
  frame_writer_t *fw=NULL;

  if (NULL==(fr=ffsox_frame_reader_new(si,stream_index,drc))) {
    MESSAGE("creating frame reader");
    goto read;
  }

  if (NULL==(fw=ffsox_frame_writer_new(so,fr,codec_id,sample_fmt,q))) {
    MESSAGE("creating frwiter");
    goto write;
  }

  fr->next=&fw->frame_consumer;
  fw->prev=&fr->node;

  return 0;
// cleanup:
  ffsox_node_destroy(&fw->node);
write:
  ffsox_node_destroy(&fr->node);
read:
  return -1;
}

int ffsox_source_link(source_t *si, sink_t *so, double drc, int codec_id,
    int sample_fmt, double q)
{
  int i;

  for (i=0;i<si->f.fc->nb_streams;++i) {
    if (si->ai==i||si->vi==i) {
      if (si->vi==i||q<0.0) {
        if (ffsox_source_link_copy(si,so,i)<0) {
          MESSAGE("copy linking");
          goto link;
        }
      }
      else {
        if (ffsox_source_link_codec(si,so,i,drc,codec_id,sample_fmt,q)<0) {
          MESSAGE("codec linking");
          goto link;
        }
      }
    }
  }

  return 0;
link:
  return -1;
}
