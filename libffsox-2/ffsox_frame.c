/*
 * ffsox_frame.c
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

int ffsox_frame_create(frame_t *f)
{
  f->nb_samples.frame=0;
  f->nb_samples.stream=0;

  if (NULL==(f->frame=av_frame_alloc()))
    goto frame;

  return 0;
// cleanup:
  av_frame_free(&f->frame);
frame:
  return -1;
}

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

int ffsox_frame_convert(frame_t *fr, frame_t *fw, double q)
{
  // we write from an arbitray input format to interleaved signed 32.
  //
  // read pointers:
  union {
    // interleaved.
    struct {
      const int16_t *rp;
    } s16i;
    // planar.
    struct {
      const float *rp[AV_NUM_DATA_POINTERS];
    } fltp;
  } u;

  int nb_samples1,nb_samples2,nb_samples;
  int channels,ch;
  // write pointers:
  int32_t *wp,*mp;

  // get the number of channels
  channels=av_frame_get_channels(fr->frame);

  // we can write the minimum from input and output nb_samples
  nb_samples1=fr->frame->nb_samples-fr->nb_samples.frame;
  nb_samples2=fw->frame->nb_samples-fw->nb_samples.frame;
  nb_samples=nb_samples1<nb_samples2?nb_samples1:nb_samples2;

  // seek to the output offset
  wp=(int32_t *)fw->frame->data[0];
  wp+=channels*fw->nb_samples.frame;
  // the maximum is another nb_samples away from the write pointer
  mp=wp+channels*nb_samples;

  switch (fr->frame->format) {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_S16:
    // seek to the input offset
    u.s16i.rp=(const int16_t *)fr->frame->data[0];
    u.s16i.rp+=channels*fr->nb_samples.frame;

    // write the nb_samples samples
    if (1.0==q) {
      while (wp<mp) {
        *wp=*u.s16i.rp++;
        *wp++<<=16;
      }
    }
    else {
      q*=1<<16;

      while (wp<mp)
        *wp++=floor(q*(*u.s16i.rp++)+0.5);
    }

    break;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_FLTP:
    // seek to the input offsets
    for (ch=0;ch<channels;++ch) {
      u.fltp.rp[ch]=(float *)fr->frame->data[0];
      u.fltp.rp[ch]+=channels*fr->nb_samples.frame;
    }

    if (1.0==q)
      q=INT32_MAX;
    else
      q*=INT32_MAX;

    // write the nb_samples samples
    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor(q*(*u.fltp.rp[ch]++)+0.5);
    }

    break;
  /////////////////////////////////////////////////////////////////////////////
  default:
    MESSAGE("unsupported sample format");
    return -1;
  }

  // advance the read and write offsets
  fr->nb_samples.frame+=nb_samples;
  fw->nb_samples.frame+=nb_samples;

  return nb_samples;
}
