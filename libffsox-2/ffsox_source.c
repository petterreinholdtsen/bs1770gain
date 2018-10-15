/*
 * ffsox_basename.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
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

static source_vmt_t vmt;

int ffsox_source_create(source_t *n, const char *path, int ai, int vi,
    source_cb_t cb, void *data)
{
  if (ffsox_node_create(&n->node)<0) {
    DMESSAGE("creating node");
    goto base;
  }

  n->vmt=ffsox_source_get_vmt();
  n->f.path=path;
  n->f.fc=NULL;

  if (avformat_open_input(&n->f.fc,path,0,0)<0) {
    DMESSAGE("opening input file");
    goto fc;
  }

  n->f.fc->flags|=AVFMT_FLAG_GENPTS;

  if (avformat_find_stream_info(n->f.fc,0)<0) {
    DMESSAGE("finding stream info");
    goto find;
  }

  ////
  n->ai=ai;
  n->vi=vi;

  if (ffsox_audiostream(n->f.fc,&n->ai,&n->vi)<0) {
    DMESSAGE("missing audio");
    goto audio;
  }
  ////

  ////
  n->cb=cb;
  n->data=data;
  ////

  n->consumer.h=NULL;
  n->consumer.n=NULL;

  n->next=NULL;
  n->ts=0ll;
  memset(&n->pkt,0,sizeof n->pkt);
  av_init_packet(&n->pkt);

  return 0;
audio:
find:
  if (NULL!=n->f.fc->pb) {
    avio_close(n->f.fc->pb);
    n->f.fc->pb=NULL;
  }

  avformat_close_input(&n->f.fc);
fc:
  vmt.parent->cleanup(&n->node);
base:
  return -1;
}

int ffsox_source_seek(source_t *n, int64_t ts)
{
  AVStream *st;
  int si;

  if (0ll<ts) {
    si=av_find_default_stream_index(n->f.fc);
    st=n->f.fc->streams[si];
    ts=av_rescale_q(ts,AV_TIME_BASE_Q,st->time_base);

    if (avformat_seek_file(n->f.fc,si,INT64_MIN,ts,INT64_MAX,0)<0) {
      DMESSAGE("seeking");
      goto seek;
    }

    n->ts=av_rescale_q(st->cur_dts,st->time_base,AV_TIME_BASE_Q);
  }

  return 0;
seek:
  return -1;
}

int ffsox_source_append(source_t *si, packet_consumer_t *pc)
{
  packet_consumer_list_t consumer;
#if defined (PBU_DEBUG) // [
  ffsox_packet_consumer_list_t *n;
  packet_consumer_t *consumer_cur;
#endif // ]

  consumer.consumer=pc;

//DVWRITELN("pc->vmt->name: \"%s\"",pc->vmt->name);
  if (LIST_APPEND(si->consumer.h,consumer)<0) {
    DMESSAGE("appending chain");
    goto append;
  }

  pc->prev=si;

#if defined (PBU_DEBUG) // [
  for (n=si->consumer.h;n;LIST_NEXT(&n,si->consumer.h)) {
    consumer_cur=n->consumer;
//DVWRITELN("consumer_cur->vmt->name: \"%s\", consumer_cur->si.stream_index: %d",consumer_cur->vmt->name,consumer_cur->si.stream_index);
    (void)consumer_cur;
  }
#endif // ]

  return 0;
append:
  return -1;
}

////////
static void source_cleanup(source_t *n)
{
  if (NULL!=n->consumer.h)
    ffsox_source_link_cleanup(n);

  if (NULL!=n->f.fc->pb) {
    avio_close(n->f.fc->pb);
    n->f.fc->pb=NULL;
  }

  avformat_close_input(&n->f.fc);
  vmt.parent->cleanup(&n->node);
}

static node_t *source_next(source_t *n)
{
  return NULL==n->next?NULL:&n->next->node;
}

static int source_run(source_t *n)
{
  packet_consumer_t *consumer;
  int64_t ts;
  AVPacket *pkt=&n->pkt;

  switch (n->state) {
  case STATE_RUN:
    for (;;) {
      n->consumer.n=n->consumer.h;
#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // {
      av_free_packet(pkt);
#else // } {
      av_packet_unref(pkt);
#endif // }

      if (av_read_frame(n->f.fc,pkt)<0) {
        n->state=STATE_FLUSH;
        goto flush;
      }

      if (NULL!=n->cb)
        n->cb(n,n->data);

//DVWRITELN("pkt->stream_index: %d",pkt->stream_index);
      while (NULL!=n->consumer.n) {
        consumer=n->consumer.n->consumer;
//DVWRITELN("consumer->vmt->name: \"%s\", consumer->si.stream_index: %d",consumer->vmt->name,consumer->si.stream_index);

        if (pkt->stream_index==consumer->si.stream_index) {
          if (0ll<n->ts) {
            ts=av_rescale_q(n->ts,AV_TIME_BASE_Q,consumer->si.st->time_base);

            if (pkt->dts!=AV_NOPTS_VALUE)
              pkt->dts-=ts;

            if (pkt->pts!=AV_NOPTS_VALUE)
              pkt->pts-=ts;
          }

          consumer->vmt->set_packet(consumer,pkt);
          n->next=consumer;

          return MACHINE_PUSH;
        }

        LIST_NEXT(&n->consumer.n,n->consumer.h);
//DVWRITELN("n->consumer.n: %p",n->consumer.n);
      }
    }
  case STATE_FLUSH:
  flush:
    if (NULL==n->consumer.n) {
      n->next=NULL;
      n->state=STATE_END;

      return MACHINE_POP;
    }
    else {
      consumer=n->consumer.n->consumer;
      consumer->state=STATE_FLUSH;
//DVWRITELN("consumer->vmt->name: \"%s\"",consumer->vmt->name);
      consumer->vmt->set_packet(consumer,NULL);
      n->next=consumer;
      LIST_NEXT(&n->consumer.n,n->consumer.h);

      return MACHINE_PUSH;
    }
  case STATE_END:
    return MACHINE_POP;
  default:
    DMESSAGE("illegal source state");
    return -1;
  }
}

const source_vmt_t *ffsox_source_get_vmt(void)
{
  const node_vmt_t *parent;

  if (NULL==vmt.parent) {
    parent=ffsox_node_get_vmt();
    vmt.node=*parent;
    vmt.parent=parent;
    vmt.name="source";
    vmt.cleanup=source_cleanup;
    vmt.next=source_next;
    vmt.run=source_run;
  }

  return &vmt;
}
