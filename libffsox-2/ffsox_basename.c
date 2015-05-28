/*
 * ffsox_basename.c
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

const char *ffsox_basename(const char *path)
{
  const char *p;

  if (NULL==path)
    return NULL;

  p=path+strlen(path);

  // TODO: unicode.
  while (path<p&&('/'==p[-1]||'\\'==p[-1]))
    --p;

  // TODO: unicode.
  while (path<p&&('/'!=p[-1]&&'\\'!=p[-1]))
    --p;

  return p;
}