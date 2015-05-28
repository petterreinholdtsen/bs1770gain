/*
 * ffsox_frame_convert.c
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

/// interleaved ///////////////////////////////////////////////////////////////
#define CONVERT_I(sfx,T,SOX_CONVERT) \
static void convert_##sfx##i(convert_t *convert, sox_uint64_t *clipsp) \
{ \
  sox_uint64_t clips=*clipsp; \
  int channels=convert->channels; \
  T *rp; \
  sox_sample_t *wp,*mp; \
  SOX_SAMPLE_LOCALS; \
 \
  rp=(T *)convert->fr->frame->data[0]; \
  rp+=channels*convert->fr->nb_samples.frame; \
 \
  wp=(sox_sample_t *)convert->fw->frame->data[0]; \
  wp+=channels*convert->fw->nb_samples.frame; \
  mp=wp+channels*convert->nb_samples; \
 \
  while (wp<mp) \
    *wp++=SOX_CONVERT(*rp++,clips); \
 \
  *clipsp=clips; \
}

CONVERT_I(u8,uint8_t,SOX_UNSIGNED_8BIT_TO_SAMPLE)
CONVERT_I(s16,int16_t,SOX_SIGNED_16BIT_TO_SAMPLE)
CONVERT_I(s32,int32_t,SOX_SIGNED_32BIT_TO_SAMPLE)
CONVERT_I(flt,float,SOX_FLOAT_32BIT_TO_SAMPLE)
CONVERT_I(dbl,double,SOX_FLOAT_64BIT_TO_SAMPLE)

/// planar ////////////////////////////////////////////////////////////////////
#define CONVERT_P(sfx,T,SOX_CONVERT) \
static void convert_##sfx##p(convert_t *convert, sox_uint64_t *clipsp) \
{ \
  sox_uint64_t clips=*clipsp; \
  int channels=convert->channels; \
  T *rp[AV_NUM_DATA_POINTERS]; \
  sox_sample_t *wp,*mp; \
  int ch; \
  SOX_SAMPLE_LOCALS; \
 \
  for (ch=0;ch<channels;++ch) { \
    rp[ch]=(T *)convert->fr->frame->data[ch]; \
    rp[ch]+=convert->fr->nb_samples.frame; \
  } \
 \
  wp=(sox_sample_t *)convert->fw->frame->data[0]; \
  wp+=channels*convert->fw->nb_samples.frame; \
  mp=wp+channels*convert->nb_samples; \
 \
  while (wp<mp) { \
    for (ch=0;ch<channels;++ch) \
      *wp++=SOX_CONVERT(*rp[ch]++,clips); \
  } \
 \
  *clipsp=clips; \
}

CONVERT_P(u8,uint8_t,SOX_UNSIGNED_8BIT_TO_SAMPLE)
CONVERT_P(s16,int32_t,SOX_SIGNED_16BIT_TO_SAMPLE)
CONVERT_P(s32,int32_t,SOX_SIGNED_32BIT_TO_SAMPLE)
CONVERT_P(flt,float,SOX_FLOAT_32BIT_TO_SAMPLE)
CONVERT_P(dbl,double,SOX_FLOAT_64BIT_TO_SAMPLE)

///////////////////////////////////////////////////////////////////////////////
int ffsox_frame_convert_sox(frame_t *fr, frame_t *fw, double q,
    sox_uint64_t *clipsp)
{
  convert_t convert;

  ffsox_convert_setup(&convert,fr,fw);

  switch (fr->frame->format) {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    convert_u8i(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_S16:
    convert_s16i(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_S32:
    convert_s32i(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_FLT:
    convert_flti(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_DBL:
    convert_dbli(&convert,clipsp);
    break;
  /// planar //////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    convert_u8p(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_S16P:
    convert_s16p(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_S32P:
    convert_s32p(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_FLTP:
    convert_fltp(&convert,clipsp);
    break;
  case AV_SAMPLE_FMT_DBLP:
    convert_dblp(&convert,clipsp);
    break;
  /////////////////////////////////////////////////////////////////////////////
  default:
    MESSAGE("unsupported sample format");
    return -1;
  }

  // advance the read and write offsets
  fr->nb_samples.frame+=convert.nb_samples;
  fw->nb_samples.frame+=convert.nb_samples;

  return convert.nb_samples;
}
