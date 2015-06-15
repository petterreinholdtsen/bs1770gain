/*
 * bs1770gain_opath.c
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
#include <bs1770gain_priv.h>
#include <ctype.h>

static char *bs177gain_norm_title(char *tp)
{
  static const char tok[]=" /-_()[]{}?'`´";
  char *p,*saveptr;

  // TODO: unicode.
  for (p=strtok_r(tp,tok,&saveptr);NULL!=p;p=strtok_r(NULL,tok,&saveptr)) {
    if (tp<p)
      *tp++='_';

    while (0!=*p)
      *tp++=tolower(*p++);
  }

  return tp;
}

char *bs1770gain_opathx(int n, const char *title, const char *odirname,
    const char *oext)
{
  char buf[32];
  int len1,len2,len3,len4;
  char *p,*opath=NULL;

  sprintf(buf,"%02d",n);
  len1=strlen(odirname);
  len2=strlen(buf);
  len3=strlen(title);
  len4=strlen(oext);

  if (NULL==(opath=p=MALLOC((len1+1)+(len2+1)+(len3+1)+(len4+1)))) {
    DMESSAGE("allocating output path");
    goto opath;
  }

  memcpy(p,odirname,len1);
  p+=len1;

  if ('/'!=p[-1]&&'\\'!=p[-1])
    *p++='/';

  memcpy(p,buf,len2);
  p+=len2;
  *p++='_';

  strcpy(p,title);
  p=bs177gain_norm_title(p);
  *p++='.';

  memcpy(p,oext,len4);
  p+=len4;
  *p++=0;
opath:
  return opath;
}

char *bs1770gain_opath(const char *ipath, const char *odirname,
    const char *oext)
{
  const char *p1,*p2;
  size_t len1,len2,len3;
  char *opath,*p;

  ////////
  len1=strlen(odirname);

  ////////
  p1=pbu_basename(ipath);
  len2=strlen(p1);
  p2=p1+len2;

  // TODO: unicode.
  while (p1<p2&&'.'!=*p2)
    --p2;

  if (p1<p2)
    len2=p2-p1;

  ////////
  len3=strlen(oext);

  if (NULL==(opath=MALLOC((len1+1)+(len2+1)+(len3+1))))
    goto opath;

  p=opath;
  
  memcpy(p,odirname,len1);
  p+=len1;

  if ('/'!=p[-1]&&'\\'!=p[-1])
    *p++='/';

  memcpy(p,p1,len2);
  p+=len2;
  *p++='.';

  memcpy(p,oext,len3);
  p+=len3;
  *p++=0;
opath:
  return opath;
}
