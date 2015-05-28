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

///////////////////////////////////////////////////////////////////////////////
// interleaved int -> interleaved int.
#define CONVERT_INT_INT_II() do { \
 bits=sizeof *rp; \
 bits-=sizeof *wp; \
 bits*=8; \
 \
  if (1.0==q) { \
    if (0<bits) { \
      while (wp<mp) { \
        *wp=*rp++; \
        *wp++<<=bits; \
      } \
    } \
    else if (bits<0) { \
      while (wp<mp) { \
        *wp=*rp++; \
        *wp++>>=-bits; \
      } \
    } \
    else { \
      while (wp<mp) \
        *wp++=*rp++; \
    } \
  } \
  else { \
    if (0<bits) \
      q*=1<<bits; \
    else if (bits<0) \
      q/=1<<-bits; \
 \
    while (wp<mp) \
      *wp++=floor(q*(*rp++)+0.5); \
  } \
} while (0)

// interleaved float -> interleaved int.
#define CONVERT_FLOAT_INT_II() do { \
 bits=sizeof *wp; \
 bits*=8; \
 \
  q=1.0==q?~(~0<<bits):q*~(~0<<bits); \
 \
  while (wp<mp) \
    *wp++=floor(q*(*rp++)+0.5); \
} while (0)

// interleaved -> interleaved int.
#define CONVERT_II(r,w,R,W,convert) \
static int convert_##r##i##_##w##i(convert_t *p) \
{ \
  double q=p->q; \
  int channels=p->channels; \
  R *rp; \
  W *wp,*mp; \
  int bits; \
 \
  rp=(R *)p->fr->frame->data[0]; \
  rp+=channels*p->fr->nb_samples.frame; \
 \
  wp=(W *)p->fw->frame->data[0]; \
  wp+=channels*p->fw->nb_samples.frame; \
  mp=wp+channels*p->nb_samples; \
 \
  convert(); \
 \
  return 0; \
}

CONVERT_II(s8,s8,int8_t,int8_t,CONVERT_INT_INT_II)
CONVERT_II(s16,s8,int16_t,int8_t,CONVERT_INT_INT_II)
CONVERT_II(s32,s8,int32_t,int8_t,CONVERT_INT_INT_II)
CONVERT_II(flt,s8,float,int8_t,CONVERT_FLOAT_INT_II)
CONVERT_II(dbl,s8,double,int8_t,CONVERT_FLOAT_INT_II)

CONVERT_II(s8,s16,int8_t,int16_t,CONVERT_INT_INT_II)
CONVERT_II(s16,s16,int16_t,int16_t,CONVERT_INT_INT_II)
CONVERT_II(s32,s16,int32_t,int16_t,CONVERT_INT_INT_II)
CONVERT_II(flt,s16,float,int16_t,CONVERT_FLOAT_INT_II)
CONVERT_II(dbl,s16,double,int16_t,CONVERT_FLOAT_INT_II)

CONVERT_II(s8,s32,int8_t,int32_t,CONVERT_INT_INT_II)
CONVERT_II(s16,s32,int16_t,int32_t,CONVERT_INT_INT_II)
CONVERT_II(s32,s32,int32_t,int32_t,CONVERT_INT_INT_II)
CONVERT_II(flt,s32,float,int32_t,CONVERT_FLOAT_INT_II)
CONVERT_II(dbl,s32,double,int32_t,CONVERT_FLOAT_INT_II)

///////////////////////////////////////////////////////////////////////////////
// planar int -> interleaved int.
#define CONVERT_INT_INT_PI() do { \
 bits=sizeof *rp[0]; \
 bits-=sizeof *wp; \
 bits*=8; \
 \
  if (1.0==q) { \
    if (0<bits) { \
      while (wp<mp) { \
        for (ch=0;ch<channels;++ch) { \
          *wp=*rp[ch]++; \
          *wp++<<=bits; \
        } \
      } \
    } \
    else if (bits<0) { \
      while (wp<mp) { \
        for (ch=0;ch<channels;++ch) { \
          *wp=*rp[ch]++; \
          *wp++>>=-bits; \
        } \
      } \
    } \
    else { \
      while (wp<mp) { \
        for (ch=0;ch<channels;++ch) \
          *wp++=*rp[ch]++; \
      } \
    } \
  } \
  else { \
    if (0<bits) \
      q*=1<<bits; \
    else if (bits<0) \
      q/=1<<-bits; \
 \
    while (wp<mp) \
      for (ch=0;ch<channels;++ch) \
        *wp++=floor(q*(*rp[ch]++)+0.5); \
  } \
} while (0)

// planar float -> interleaved int.
#define CONVERT_FLOAT_INT_PI() do { \
 bits=sizeof *wp; \
 bits*=8; \
 \
  q=1.0==q?~(~0<<bits):q*~(~0<<bits); \
 \
  while (wp<mp) { \
    for (ch=0;ch<channels;++ch) \
      *wp++=floor(q*(*rp[ch]++)+0.5); \
  } \
} while (0)

// planar -> interleaved int.
#define CONVERT_PI(r,w,R,W,convert) \
static int convert_##r##p##_##w##i(convert_t *p) \
{ \
  double q=p->q; \
  int ch,channels=p->channels; \
  R *rp[AV_NUM_DATA_POINTERS]; \
  W *wp,*mp; \
  int bits; \
 \
  for (ch=0;ch<channels;++ch) { \
    rp[ch]=(R *)p->fr->frame->data[ch]; \
    rp[ch]+=p->fr->nb_samples.frame; \
  } \
 \
  wp=(W *)p->fw->frame->data[0]; \
  wp+=channels*p->fw->nb_samples.frame; \
  mp=wp+channels*p->nb_samples; \
 \
  convert(); \
 \
  return 0; \
}

CONVERT_PI(s8,s8,int8_t,int8_t,CONVERT_INT_INT_PI)
CONVERT_PI(s16,s8,int16_t,int8_t,CONVERT_INT_INT_PI)
CONVERT_PI(s32,s8,int32_t,int8_t,CONVERT_INT_INT_PI)
CONVERT_PI(flt,s8,float,int8_t,CONVERT_FLOAT_INT_PI)
CONVERT_PI(dbl,s8,double,int8_t,CONVERT_FLOAT_INT_PI)

CONVERT_PI(s8,s16,int8_t,int16_t,CONVERT_INT_INT_PI)
CONVERT_PI(s16,s16,int16_t,int16_t,CONVERT_INT_INT_PI)
CONVERT_PI(s32,s16,int32_t,int16_t,CONVERT_INT_INT_PI)
CONVERT_PI(flt,s16,float,int16_t,CONVERT_FLOAT_INT_PI)
CONVERT_PI(dbl,s16,double,int16_t,CONVERT_FLOAT_INT_PI)

CONVERT_PI(s8,s32,int8_t,int32_t,CONVERT_INT_INT_PI)
CONVERT_PI(s16,s32,int16_t,int32_t,CONVERT_INT_INT_PI)
CONVERT_PI(s32,s32,int32_t,int32_t,CONVERT_INT_INT_PI)
CONVERT_PI(flt,s32,float,int32_t,CONVERT_FLOAT_INT_PI)
CONVERT_PI(dbl,s32,double,int32_t,CONVERT_FLOAT_INT_PI)

///////////////////////////////////////////////////////////////////////////////
// interleaved/planar -> interleaved int.
#define CONVERT(sfx) \
static int convert_##sfx(convert_t *convert) \
{ \
  switch (convert->fw->frame->format) { \
  /* interleaved */ \
  case AV_SAMPLE_FMT_U8: \
    return convert_##sfx##_s8i(convert); \
  case AV_SAMPLE_FMT_S16: \
    return convert_##sfx##_s16i(convert); \
  case AV_SAMPLE_FMT_S32: \
    return convert_##sfx##_s32i(convert); \
  /* default */ \
  default: \
    MESSAGE("output sample format not supported yet"); \
    break; \
  } \
 \
  return -1; \
}

CONVERT(s8i)
CONVERT(s16i)
CONVERT(s32i)
CONVERT(flti)
CONVERT(dbli)

CONVERT(s8p)
CONVERT(s16p)
CONVERT(s32p)
CONVERT(fltp)
CONVERT(dblp)

///////////////////////////////////////////////////////////////////////////////
int ffsox_frame_convert(frame_t *fr, frame_t *fw, double q)
{
  convert_t convert;
  int code;

  ffsox_convert_setup(&convert,fr,fw,q,NULL);

  switch (convert.fr->frame->format) {
  /// interleaved /////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8:
    code=convert_s8i(&convert);
    break;
  case AV_SAMPLE_FMT_S16:
    code=convert_s16i(&convert);
    break;
  case AV_SAMPLE_FMT_S32:
    code=convert_s32i(&convert);
    break;
  case AV_SAMPLE_FMT_FLT:
    code=convert_flti(&convert);
    break;
  case AV_SAMPLE_FMT_DBL:
    code=convert_dbli(&convert);
    break;
  ///planar ///////////////////////////////////////////////////////////////////
  case AV_SAMPLE_FMT_U8P:
    code=convert_s8p(&convert);
    break;
  case AV_SAMPLE_FMT_S16P:
    code=convert_s16p(&convert);
    break;
  case AV_SAMPLE_FMT_S32P:
    code=convert_s32p(&convert);
    break;
  case AV_SAMPLE_FMT_FLTP:
    code=convert_fltp(&convert);
    break;
  case AV_SAMPLE_FMT_DBLP:
    code=convert_dblp(&convert);
    break;
  /////////////////////////////////////////////////////////////////////////////
  default:
    MESSAGE("unsupported sample format");
    return -1;
  }

  if (code<0) {
    MESSAGE("converion not supported");
    return -1;
  }

  // advance the read and write offsets
  fr->nb_samples.frame+=convert.nb_samples;
  fw->nb_samples.frame+=convert.nb_samples;

  return convert.nb_samples;
}
