/*
 * pbu_wstrtok.c
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
#if defined (_WIN32) // {
#include <pbutil.h>

static wchar_t *pbu_wcstok(wchar_t *str, const wchar_t *delim,
    wchar_t **saveptr)
{
  (void)saveptr;

  return wcstok(str,delim);
}

wchar_t *pbu_wcstok_r(wchar_t *str, const wchar_t *delim, wchar_t **saveptr)
{
  typedef wchar_t *(*wcstok_s_t)(wchar_t *,const wchar_t *,wchar_t **); 
  static wcstok_s_t wcstok_s=NULL;
  HANDLE hLib;

  if (NULL==wcstok_s) {
    if (NULL==(hLib=pbu_msvcrt()))
      goto wcstok;

    if (NULL==(wcstok_s=(wcstok_s_t)GetProcAddress(hLib,"wcstok_s")))
      goto wcstok;

    goto wcstok_s;
  wcstok:
    wcstok_s=pbu_wcstok;
    goto wcstok_s;
  }
wcstok_s:
  return wcstok_s(str,delim,saveptr);
}
#endif // }
