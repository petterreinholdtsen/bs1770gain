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

static int source_link_copy(source_t *si, sink_t *so, int stream_index)
{
  read_copy_t *rc=NULL;
  write_copy_t *wc=NULL;
  ffsox_read_list_t reads;

  if (NULL==(rc=ffsox_read_copy_new(si,stream_index))) {
    MESSAGE("creating read copy node");
    goto rc;
  }

  if (NULL==(wc=ffsox_write_copy_new(so,rc))) {
    MESSAGE("creating write copy node");
    goto wc;
  }

  rc->write=&wc->write;
  rc->prev=si;
  rc->next=wc;
  wc->prev=rc;
  reads.read=&rc->read;

  if (FFSOX_LIST_APPEND(si->reads.h,reads)<0) {
    MESSAGE("appending chain");
    goto link;
  }

  return 0;
link:
wc:
  ffsox_node_destroy(&rc->node);
rc:
  return -1;
}

static int source_link_codec(source_t *si, sink_t *so, int i, double drc,
    int codec_id, int sample_fmt, double q)
{
  read_decode_t *rd=NULL;
  write_encode_t *we=NULL;
  filter_t *f=NULL;
  ffsox_read_list_t reads;

  if (NULL==(rd=ffsox_read_decode_new(si,i,drc))) {
    MESSAGE("creating read decode node");
    goto read;
  }

  if (NULL==(we=ffsox_write_encode_new(so,rd,codec_id,sample_fmt))) {
    MESSAGE("creating write encode node");
    goto write;
  }

  if (NULL==(f=ffsox_filter_new(we,q))) {
    MESSAGE("creating write filter node");
    goto filter;
  }

  rd->write=&we->write;
  rd->prev=si;
  rd->next=f;
  f->prev=rd;
  f->next=we;
  we->prev=f;
  reads.read=&rd->read;

  if (FFSOX_LIST_APPEND(si->reads.h,reads)<0) {
    MESSAGE("appending chain");
    goto link;
  }

  return 0;
link:
  ffsox_node_destroy(&f->node);
filter:
  ffsox_node_destroy(&we->node);
write:
  ffsox_node_destroy(&rd->node);
read:
  return -1;
}

int ffsox_source_link_create(source_t *si, sink_t *so, double drc,
    int codec_id, int sample_fmt, double q, int ai, int vi)
{
  int i;

  if (ffsox_audiostream(si->f.fc,&ai,&vi)<0) {
    MESSAGE("no audio");
    goto audio;
  }

  for (i=0;i<si->f.fc->nb_streams;++i) {
    if (ai==i||vi==i) {
      if (vi==i||q<0.0) {
        if (source_link_copy(si,so,i)<0) {
          MESSAGE("copy linking");
          goto link;
        }
      }
      else {
        if (source_link_codec(si,so,i,drc,codec_id,sample_fmt,q)<0) {
          MESSAGE("codec linking");
          goto link;
        }
      }
    }
  }

  return 0;
link:
  ffsox_source_link_cleanup(si);
audio:
  return -1;
}

void ffsox_source_link_cleanup(source_t *n)
{
  ffsox_list_free_full(n->reads.h,ffsox_read_list_free);
  n->reads.h=NULL;
  n->reads.n=NULL;
}
