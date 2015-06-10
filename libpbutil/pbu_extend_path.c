/*
 * pbu_extend_path.c
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
#include <pbutil_priv.h>

char *pbu_extend_path(const char *dirname, const char *basename)
{
  const char *p1;
  char *path,*p2;
  int len1,len2;

  path=NULL;
  p1=dirname+strlen(dirname);

  // TODO: unicode.
  while (dirname<p1&&('/'==p1[-1]||'\\'==p1[-1]))
    --p1;

  len1=p1-dirname;
  len2=strlen(basename);

  if (NULL==(path=malloc((len1+1)+(len2+1)))) {
    MESSAGE("allocating path");
    goto path;
  }

  p2=path;
  memcpy(p2,dirname,len1);
  p2+=len1;
  *p2++='/';
  strcpy(p2,basename);
path:
  return path;
}
