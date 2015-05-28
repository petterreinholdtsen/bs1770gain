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
    struct { const int8_t  *rp; } s8i;
    struct { const int16_t *rp; } s16i;
    struct { const int32_t *rp; } s32i;
    struct { const float   *rp; } flti;
    struct { const double  *rp; } dbli;
    // planar.
    struct { const int8_t  *rp[AV_NUM_DATA_POINTERS]; } s8p;
    struct { const int16_t *rp[AV_NUM_DATA_POINTERS]; } s16p;
    struct { const int32_t *rp[AV_NUM_DATA_POINTERS]; } s32p;
    struct { const float   *rp[AV_NUM_DATA_POINTERS]; } fltp;
    struct { const double  *rp[AV_NUM_DATA_POINTERS]; } dblp;
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
  case AV_SAMPLE_FMT_U8:
    // seek to the input offset
    u.s8i.rp=(const int8_t *)fr->frame->data[0];
    u.s8i.rp+=channels*fr->nb_samples.frame;

    // write the nb_samples samples
    if (1.0==q) {
      while (wp<mp) {
        *wp=*u.s8i.rp++;
        *wp++<<=24;
      }
    }
    else {
      q*=1<<24;

      while (wp<mp)
        *wp++=floor(q*(*u.s8i.rp++)+0.5);
    }

    break;
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
  case AV_SAMPLE_FMT_S32:
    // seek to the input offset
    u.s32i.rp=(const int32_t *)fr->frame->data[0];
    u.s32i.rp+=channels*fr->nb_samples.frame;

    // write the nb_samples samples
    if (1.0==q) {
      while (wp<mp)
        *wp=*u.s32i.rp++;
    }
    else {
      while (wp<mp)
        *wp++=floor(q*(*u.s32i.rp++)+0.5);
    }

    break;
  case AV_SAMPLE_FMT_FLT:
    // seek to the input offset
    u.flti.rp=(const float *)fr->frame->data[0];
    u.flti.rp+=channels*fr->nb_samples.frame;

    q=1.0==q?INT32_MAX:q*INT32_MAX;

    // write the nb_samples samples
    while (wp<mp)
      *wp++=floor(q*(*u.flti.rp++)+0.5);

    break;
  case AV_SAMPLE_FMT_DBL:
    // seek to the input offset
    u.dbli.rp=(const double *)fr->frame->data[0];
    u.dbli.rp+=channels*fr->nb_samples.frame;

    q=1.0==q?INT32_MAX:q*INT32_MAX;

    // write the nb_samples samples
    while (wp<mp)
      *wp++=floor(q*(*u.dbli.rp++)+0.5);

    break;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    // seek to the input offsets
    for (ch=0;ch<channels;++ch) {
      u.s8p.rp[ch]=(int8_t *)fr->frame->data[0];
      u.s8p.rp[ch]+=channels*fr->nb_samples.frame;
    }

    // write the nb_samples samples
    if (1.0==q) {
      while (wp<mp) {
        for (ch=0;ch<channels;++ch) {
          *wp++=*u.s8p.rp[ch];
          *wp++<<=24;
        }
      }
    }
    else {
      q*=1<<24;

      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor(q*(*u.s8p.rp[ch]++)+0.5);
      }
    }

    break;
  case AV_SAMPLE_FMT_S16P:
    // seek to the input offsets
    for (ch=0;ch<channels;++ch) {
      u.s16p.rp[ch]=(int16_t *)fr->frame->data[0];
      u.s16p.rp[ch]+=channels*fr->nb_samples.frame;
    }

    // write the nb_samples samples
    if (1.0==q) {
      while (wp<mp) {
        for (ch=0;ch<channels;++ch) {
          *wp++=*u.s16p.rp[ch];
          *wp++<<=16;
        }
      }
    }
    else {
      q*=1<<16;

      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor(q*(*u.s16p.rp[ch]++)+0.5);
      }
    }

    break;
  case AV_SAMPLE_FMT_S32P:
    // seek to the input offsets
    for (ch=0;ch<channels;++ch) {
      u.s32p.rp[ch]=(int32_t *)fr->frame->data[0];
      u.s32p.rp[ch]+=channels*fr->nb_samples.frame;
    }

    // write the nb_samples samples
    if (1.0==q) {
      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=*u.s32p.rp[ch]++;
      }
    }
    else {
      while (wp<mp) {
        for (ch=0;ch<channels;++ch)
          *wp++=floor(q*(*u.s32p.rp[ch]++)+0.5);
      }
    }

    break;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_FLTP:
    // seek to the input offsets
    for (ch=0;ch<channels;++ch) {
      u.fltp.rp[ch]=(float *)fr->frame->data[0];
      u.fltp.rp[ch]+=channels*fr->nb_samples.frame;
    }

    q=1.0==q?INT32_MAX:q*INT32_MAX;

    // write the nb_samples samples
    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor(q*(*u.fltp.rp[ch]++)+0.5);
    }

    break;
  case AV_SAMPLE_FMT_DBLP:
    // seek to the input offsets
    for (ch=0;ch<channels;++ch) {
      u.dblp.rp[ch]=(double *)fr->frame->data[0];
      u.dblp.rp[ch]+=channels*fr->nb_samples.frame;
    }

    q=1.0==q?INT32_MAX:q*INT32_MAX;

    // write the nb_samples samples
    while (wp<mp) {
      for (ch=0;ch<channels;++ch)
        *wp++=floor(q*(*u.dblp.rp[ch]++)+0.5);
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
