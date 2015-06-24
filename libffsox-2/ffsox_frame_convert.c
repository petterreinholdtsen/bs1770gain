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
void ffsox_convert_int_int(uint8_t *rp, size_t rsize, uint8_t *wp,
    uint8_t *mp, size_t wsize, double q)
{
  int bits=8*(wsize-rsize);
  int32_t i;

//fprintf(stderr,"1.1 ...\n");
  if (0<bits)
    q*=1<<bits;
  else if (bits<0)
    q/=1<<-bits;

  while (wp<mp) {
    //i=(1<<7)&*rp?~0:0;
    i=0;
//fprintf(stderr,"%d, %d: %d -> ",rsize,wsize,i);
    memcpy(&i,rp,rsize);
    rp+=rsize;

    if (rp[-1]&(1<<7)) {
      rp[-1]&=~(1<<7);
      i=-i;
    }

//fprintf(stderr,"%d, %d: %d -> ",rsize,wsize,i);
    i=q*i+0.5;
//fprintf(stderr,"%d\n",i);
    memcpy(wp,&i,wsize);
    wp+=wsize;
  }
}

///////////////////////////////////////////////////////////////////////////////
// interleaved int -> interleaved int.
#if 1 // {
#define CONVERT_INT_INT_II() do { \
 int bits=sizeof *wp; \
 bits-=sizeof *rp; \
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
#else // } {
#define CONVERT_INT_INT_II() \
  ffsox_convert_int_int((uint8_t *)rp,sizeof *rp,(uint8_t *)wp, \
      (uint8_t *)mp,sizeof *wp,q)
#endif // }

// interleaved float -> interleaved int.
#define CONVERT_FLOAT_INT_II() do { \
 int bits=sizeof *wp; \
 bits*=8; \
 bits-=1; \
 \
  q=1.0==q?~(~0ll<<bits):q*~(~0ll<<bits); \
 \
  while (wp<mp) \
    *wp++=floor(q*(*rp++)+0.5); \
} while (0)

// interleaved int -> interleaved float.
#define CONVERT_INT_FLOAT_II() do { \
 int bits=sizeof *rp; \
 bits*=8; \
 bits-=1; \
 \
  q/=~(~0ll<<bits); \
 \
  while (wp<mp) \
    *wp++=q*(*rp++); \
} while (0)

// interleaved float -> interleaved float.
#define CONVERT_FLOAT_FLOAT_II() do { \
 \
  if (1.0==q) { \
    while (wp<mp) \
      *wp++=*rp++; \
  } \
  else { \
    while (wp<mp) \
      *wp++=q*(*rp++); \
  } \
} while (0)

// interleaved -> interleaved int.
#define CONVERT_II(r,w,R,W,convert) \
static int convert_##r##i##_##w##i(convert_t *p) \
{ \
  double q=p->q; \
  int channels=p->channels; \
  R *rp; \
  W *wp,*mp; \
 \
  rp=(void *)p->fr->frame->data[0]; \
  rp+=channels*p->fr->nb_samples.frame; \
 \
  wp=(void *)p->fw->frame->data[0]; \
  wp+=channels*p->fw->nb_samples.frame; \
  mp=wp+channels*p->nb_samples; \
 \
  convert(); \
 \
  return 0; \
}

////
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

////
CONVERT_II(s8,flt,int8_t,float,CONVERT_INT_FLOAT_II)
CONVERT_II(s16,flt,int16_t,float,CONVERT_INT_FLOAT_II)
CONVERT_II(s32,flt,int32_t,float,CONVERT_INT_FLOAT_II)
CONVERT_II(flt,flt,float,float,CONVERT_FLOAT_FLOAT_II)
CONVERT_II(dbl,flt,double,float,CONVERT_FLOAT_FLOAT_II)

CONVERT_II(s8,dbl,int8_t,double,CONVERT_INT_FLOAT_II)
CONVERT_II(s16,dbl,int16_t,double,CONVERT_INT_FLOAT_II)
CONVERT_II(s32,dbl,int32_t,double,CONVERT_INT_FLOAT_II)
CONVERT_II(flt,dbl,float,double,CONVERT_FLOAT_FLOAT_II)
CONVERT_II(dbl,dbl,double,double,CONVERT_FLOAT_FLOAT_II)

///////////////////////////////////////////////////////////////////////////////
// planar int -> interleaved int.
#define CONVERT_INT_INT_PI() do { \
 int bits=sizeof *wp; \
 bits-=sizeof *rp[0]; \
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
 int bits=sizeof *wp; \
 bits*=8; \
 bits-=1; \
 \
  q=1.0==q?~(~0ll<<bits):q*~(~0ll<<bits); \
 \
  while (wp<mp) { \
    for (ch=0;ch<channels;++ch) \
      *wp++=floor(q*(*rp[ch]++)+0.5); \
  } \
} while (0)

// planar int -> interleaved float.
#define CONVERT_INT_FLOAT_PI() do { \
 int bits=sizeof *rp[0]; \
 bits*=8; \
 bits-=1; \
 \
  q/=~(~0ll<<bits); \
 \
  while (wp<mp) { \
    for (ch=0;ch<channels;++ch) \
      *wp++=q*(*rp[ch]++); \
  } \
} while (0)

// planar float -> interleaved float.
#define CONVERT_FLOAT_FLOAT_PI() do { \
  if (1.0==q) { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) \
        *wp++=*rp[ch]++; \
    } \
  } \
  else { \
    while (wp<mp) { \
      for (ch=0;ch<channels;++ch) \
        *wp++=q*(*rp[ch]++); \
    } \
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
 \
  for (ch=0;ch<channels;++ch) { \
    rp[ch]=(void *)p->fr->frame->data[ch]; \
    rp[ch]+=p->fr->nb_samples.frame; \
  } \
 \
  wp=(void *)p->fw->frame->data[0]; \
  wp+=channels*p->fw->nb_samples.frame; \
  mp=wp+channels*p->nb_samples; \
 \
  convert(); \
 \
  return 0; \
}

//////
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

//////
CONVERT_PI(s8,flt,int8_t,float,CONVERT_INT_FLOAT_PI)
CONVERT_PI(s16,flt,int16_t,float,CONVERT_INT_FLOAT_PI)
CONVERT_PI(s32,flt,int32_t,float,CONVERT_INT_FLOAT_PI)
CONVERT_PI(flt,flt,float,float,CONVERT_FLOAT_FLOAT_PI)
CONVERT_PI(dbl,flt,double,float,CONVERT_FLOAT_FLOAT_PI)

CONVERT_PI(s8,dbl,int8_t,double,CONVERT_INT_FLOAT_PI)
CONVERT_PI(s16,dbl,int16_t,double,CONVERT_INT_FLOAT_PI)
CONVERT_PI(s32,dbl,int32_t,double,CONVERT_INT_FLOAT_PI)
CONVERT_PI(flt,dbl,float,double,CONVERT_FLOAT_FLOAT_PI)
CONVERT_PI(dbl,dbl,double,double,CONVERT_FLOAT_FLOAT_PI)

///////////////////////////////////////////////////////////////////////////////
// interleaved/planar int/float -> interleaved int/float.
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
  case AV_SAMPLE_FMT_FLT: \
    return convert_##sfx##_flti(convert); \
  case AV_SAMPLE_FMT_DBL: \
    return convert_##sfx##_dbli(convert); \
  /* default */ \
  default: \
    DMESSAGE("output sample format not supported yet"); \
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
    DMESSAGE("unsupported sample format");
    return -1;
  }

  if (code<0) {
    DMESSAGE("converion not supported");
    return -1;
  }

  // advance the read and write offsets
  fr->nb_samples.frame+=convert.nb_samples;
  fw->nb_samples.frame+=convert.nb_samples;

  return convert.nb_samples;
}
