/*
 * pbu_extend_path.c
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
#include <pbutil_priv.h>

char *pbu_extend_path(const char *dirname, const char *basename)
{
#if 0 // [
  const char *p1;
  char *path,*p2;
  int len1,len2;

  if (dirname) {
    path=NULL;
    p1=dirname+strlen(dirname);

    // TODO: unicode.
    while (dirname<p1&&('/'==p1[-1]||'\\'==p1[-1]))
      --p1;

    len1=p1-dirname;
  else
    len1=0;

  len2=strlen(basename);

  if (NULL==(path=MALLOC((len1?len1+1:0)+(len2+1)))) {
    DMESSAGE("allocating path");
    goto path;
  }

  p2=path;

  if (dirname) {
    memcpy(p2,dirname,len1);
    p2+=len1;
    *p2++='/';
  }

  strcpy(p2,basename);
path:
  return path;
#else // ] [
  char *path,*wp;
  size_t len1,len2;

  len1=dirname?strlen(dirname):0;
  len2=basename?strlen(basename):0;

  if (NULL==(path=wp=MALLOC((len1?len1+1:0)+(len2?len2+1:0)))) {
    DMESSAGE("allocating path");
    goto path;
  }

  if (len1) {
    memcpy(wp,dirname,len1);
    wp+=len1;
    *wp++='/';
  }

  if (len2)
    strcpy(wp,basename);

  return path;
path:
  return NULL;
#endif // ]
}
