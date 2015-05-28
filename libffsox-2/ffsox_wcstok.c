/*
 * ffsox_wstrtok.c
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
#if defined (WIN32) // {
#include <ffsox.h>

static wchar_t *ffsox_wcstok(wchar_t *str, const wchar_t *delim,
    wchar_t **saveptr)
{
  return wcstok(str,delim);
}

wchar_t *ffsox_wcstok_r(wchar_t *str, const wchar_t *delim,
    wchar_t **saveptr)
{
  typedef typeof (wcstok_s) *wcstok_s_t;
  static wcstok_s_t wcstok_s=NULL;
  HANDLE hLib;

  if (NULL==wcstok_s) {
    if (NULL==(hLib=ffsox_msvcrt()))
      goto wcstok;

    if (NULL==(wcstok_s=(wcstok_s_t)GetProcAddress(hLib,"wcstok_s")))
      goto wcstok;

    goto wcstok_s;
  wcstok:
    wcstok_s=ffsox_wcstok;
    goto wcstok_s;
  }
wcstok_s:
  return wcstok_s(str,delim,saveptr);
}

#endif // }
