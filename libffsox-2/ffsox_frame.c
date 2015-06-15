/*
 * ffsox_frame.c
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

int ffsox_frame_create(frame_t *f)
{
  f->nb_samples.frame=0;
  f->nb_samples.stream=0;

  if (NULL==(f->frame=av_frame_alloc())) {
    DMESSAGE("allocation frame");
    goto frame;
  }

  return 0;
// cleanup:
  av_frame_free(&f->frame);
frame:
  return -1;
}

#if 1 // {
int ffsox_frame_create_cc(frame_t *f, AVCodecContext *cc)
{
  const AVCodec *codec=cc->codec;
  AVFrame *frame;
  int nb_samples;

  if (ffsox_frame_create(f)<0) {
    DMESSAGE("creating frame");
    goto frame;
  }

  if (NULL!=codec&&(codec->capabilities&CODEC_CAP_VARIABLE_FRAME_SIZE))
    nb_samples=10000;
  else
    nb_samples=cc->frame_size;

  frame=f->frame;
  frame->format=cc->sample_fmt;
  frame->channel_layout=cc->channel_layout;
  frame->channels=cc->channels;
  frame->sample_rate=cc->sample_rate;
  frame->nb_samples=nb_samples;

  if (0<nb_samples&&av_frame_get_buffer(frame,0)<0) {
    DMESSAGE("allocatiing frame buffer");
    goto buffer;
  }

  return 0;
// cleanup:
buffer:
  ffsox_frame_cleanup(f);
frame:
  return -1;
}
#else // } {
1nt ffsox_frame_create_cc(frame_t *f, AVCodecContext *cc)
{
  const AVCodec *codec=cc->codec;
  AVFrame *frame;
  int nb_samples;

  if (ffsox_frame_create(f)<0) {
    DMESSAGE("creating frame");
    goto frame;
  }

  frame=f->frame;
  frame->format=cc->sample_fmt;
  frame->channel_layout=cc->channel_layout;
  frame->channels=cc->channels;
  frame->sample_rate=cc->sample_rate;
  frame->nb_samples=cc->frame_size;

  if (av_frame_get_buffer(frame,0)<0) {
    DMESSAGE("allocatiing frame buffer");
    goto buffer;
  }

  return 0;
// cleanup:
buffer:
  ffsox_frame_cleanup(f);
frame:
  return -1;
}
#endif // }

void ffsox_frame_cleanup(frame_t *f)
{
  av_frame_free(&f->frame);
}

int ffsox_frame_complete(frame_t *f)
{
  return f->nb_samples.frame==f->frame->nb_samples;
}

void ffsox_frame_reset(frame_t *f)
{
  f->nb_samples.stream+=f->nb_samples.frame;
  f->nb_samples.frame=0;
}
