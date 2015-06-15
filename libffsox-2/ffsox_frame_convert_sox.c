/*
 * ffsox_frame_convert.c
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

///////////////////////////////////////////////////////////////////////////////
// operation
#define INTERCEPT_CHANNEL_INT_OP(i) \
  intercept->channel(intercept->data,ch,scale*(i))
#define INTERCEPT_CHANNEL_FLT_OP(x) \
  intercept->channel(intercept->data,ch,x)
#define INTERCEPT_SAMPLE_OP() \
  intercept->sample(intercept->data)

// no operation
#define INTERCEPT_CHANNEL_INT_NOP(i)
#define INTERCEPT_CHANNEL_FLT_NOP(x)
#define INTERCEPT_SAMPLE_NOP()

/// interleaved ///////////////////////////////////////////////////////////////
#define CONVERT_INT_I(SOX_CONVERT,OP) do { \
  double scale=1.0/MAXOF(*rp); \
  double x; \
 \
  if (1.0==q) { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        INTERCEPT_CHANNEL_INT_##OP(*rp); \
        *wp++=SOX_CONVERT(*rp++,clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
  else { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        x=scale*(*rp++); \
        INTERCEPT_CHANNEL_FLT_##OP(x); \
        x*=q; \
        *wp++=SOX_FLOAT_64BIT_TO_SAMPLE(x,clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
} while (0)

#define CONVERT_FLT_I(SOX_CONVERT,OP) do { \
  if (1.0==q) { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        INTERCEPT_CHANNEL_FLT_##OP(*rp); \
        *wp++=SOX_CONVERT(*rp++,clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
  else { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        INTERCEPT_CHANNEL_FLT_##OP(*rp); \
        *wp++=SOX_CONVERT(q*(*rp++),clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
} while (0)

#define CONVERT_I(sfx,T,convert,SOX_CONVERT) \
static void convert_##sfx##i(convert_t *p, sox_uint64_t *clipsp) \
{ \
  sox_uint64_t clips=*clipsp; \
  double q=p->q; \
  intercept_t *intercept=p->intercept; \
  int ch,channels=p->channels; \
  T *rp; \
  sox_sample_t *wp,*mp; \
  SOX_SAMPLE_LOCALS; \
 \
  (void)ch; \
 \
  rp=(T *)p->fr->frame->data[0]; \
  rp+=channels*p->fr->nb_samples.frame; \
 \
  wp=(sox_sample_t *)p->fw->frame->data[0]; \
  wp+=channels*p->fw->nb_samples.frame; \
  mp=wp+channels*p->nb_samples; \
 \
  if (NULL==intercept) \
    convert(SOX_CONVERT,NOP); \
  else \
    convert(SOX_CONVERT,OP); \
 \
  *clipsp=clips; \
}

CONVERT_I(s8,int8_t,CONVERT_INT_I,SOX_UNSIGNED_8BIT_TO_SAMPLE)
CONVERT_I(s16,int16_t,CONVERT_INT_I,SOX_SIGNED_16BIT_TO_SAMPLE)
CONVERT_I(s32,int32_t,CONVERT_INT_I,SOX_SIGNED_32BIT_TO_SAMPLE)
CONVERT_I(flt,float,CONVERT_FLT_I,SOX_FLOAT_32BIT_TO_SAMPLE)
CONVERT_I(dbl,double,CONVERT_FLT_I,SOX_FLOAT_64BIT_TO_SAMPLE)

/// planar ////////////////////////////////////////////////////////////////////
#define CONVERT_INT_P(SOX_CONVERT) do { \
  double scale=1.0/MAXOF(*rp[0]); \
  double x; \
 \
  if (1.0==q) { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        INTERCEPT_CHANNEL_INT_##OP(*rp[ch]); \
        *wp++=SOX_CONVERT(*rp[ch]++,clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
  else { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        x=scale*(*rp[ch]++); \
        INTERCEPT_CHANNEL_FLT_##OP(x); \
        x*=q; \
        *wp++=SOX_FLOAT_64BIT_TO_SAMPLE(x,clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
} while (0)

#define CONVERT_FLOAT_P(SOX_CONVERT) do { \
  if (1.0==q) { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        INTERCEPT_CHANNEL_FLT_##OP(*rp[ch]); \
        *wp++=SOX_CONVERT(*rp[ch]++,clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
  else { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) { \
        INTERCEPT_CHANNEL_FLT_##OP(*rp[ch]); \
        *wp++=SOX_CONVERT(q*(*rp[ch]++),clips); \
      } \
 \
      INTERCEPT_SAMPLE_##OP(); \
    } \
  } \
} while (0)

#define CONVERT_P(sfx,T,convert,SOX_CONVERT) \
static void convert_##sfx##p(convert_t *p, sox_uint64_t *clipsp) \
{ \
  sox_uint64_t clips=*clipsp; \
  double q=p->q; \
  intercept_t *intercept=p->intercept; \
  int channels=p->channels; \
  T *rp[AV_NUM_DATA_POINTERS]; \
  sox_sample_t *wp,*mp; \
  int ch; \
  SOX_SAMPLE_LOCALS; \
 \
  for (ch=0;ch<channels;++ch) { \
    rp[ch]=(T *)p->fr->frame->data[ch]; \
    rp[ch]+=p->fr->nb_samples.frame; \
  } \
 \
  wp=(sox_sample_t *)p->fw->frame->data[0]; \
  wp+=channels*p->fw->nb_samples.frame; \
  mp=wp+channels*p->nb_samples; \
 \
  convert(SOX_CONVERT); \
 \
  *clipsp=clips; \
}

CONVERT_P(s8,int8_t,CONVERT_INT_P,SOX_UNSIGNED_8BIT_TO_SAMPLE)
CONVERT_P(s16,int32_t,CONVERT_INT_P,SOX_SIGNED_16BIT_TO_SAMPLE)
CONVERT_P(s32,int32_t,CONVERT_INT_P,SOX_SIGNED_32BIT_TO_SAMPLE)
CONVERT_P(flt,float,CONVERT_FLOAT_P,SOX_FLOAT_32BIT_TO_SAMPLE)
CONVERT_P(dbl,double,CONVERT_FLOAT_P,SOX_FLOAT_64BIT_TO_SAMPLE)

///////////////////////////////////////////////////////////////////////////////
int ffsox_frame_convert_sox(frame_t *fr, frame_t *fw, double q,
    intercept_t *intercept, sox_uint64_t *clipsp)
{
  convert_t convert;

  ffsox_convert_setup(&convert,fr,fw,q,intercept);

  switch (fr->frame->format) {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    convert_s8i(&convert,clipsp);
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
    convert_s8p(&convert,clipsp);
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
    DMESSAGE("unsupported sample format");
    return -1;
  }

  // advance the read and write offsets
  fr->nb_samples.frame+=convert.nb_samples;
  fw->nb_samples.frame+=convert.nb_samples;

  return convert.nb_samples;
}
