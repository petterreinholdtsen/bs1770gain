/*
 * pbu_s2w.c
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
#include <pbutil_priv.h>

char *pbu_w2s(const wchar_t *w)
{
  size_t size;
  char *s;

  s=NULL;

  if (NULL==w)
    goto s;

  size=WideCharToMultiByte(
    CP_UTF8,      // _In_      UINT    CodePage,
    0,            // _In_      DWORD   dwFlags,
    w,            // _In_      LPCWSTR lpWideCharStr,
    -1,           // _In_      int     cchWideChar,
    NULL,         // _Out_opt_ LPSTR   lpMultiByteStr,
    0,            // _In_      int     cbMultiByte,
    NULL,         // _In_opt_  LPCSTR  lpDefaultChar,
    NULL          // _Out_opt_ LPBOOL  lpUsedDefaultChar
  );

  if (NULL==(s=MALLOC(size*(sizeof *s))))
    goto s;

  size=WideCharToMultiByte(
    CP_UTF8,      // _In_      UINT    CodePage,
    0,            // _In_      DWORD   dwFlags,
    w,            // _In_      LPCWSTR lpWideCharStr,
    -1,           // _In_      int     cchWideChar,
    s,            // _Out_opt_ LPSTR   lpMultiByteStr,
    size,         // _In_      int     cbMultiByte,
    NULL,         // _In_opt_  LPCSTR  lpDefaultChar,
    NULL          // _Out_opt_ LPBOOL  lpUsedDefaultChar
  );
s:
  return s;
}
#endif // }
