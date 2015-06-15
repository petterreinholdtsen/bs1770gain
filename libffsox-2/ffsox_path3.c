/*
 * ffsox_path3.c
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

#if defined (_WIN32) // {
wchar_t *ffsox_path3(const wchar_t *ws1, const char *s2, const char *s3)
{
  wchar_t *path,*pp;
  int size1,size2,size3,size;

  path=NULL;
  size1=NULL==ws1||0==*ws1?0:wcslen(ws1)+1;

  if (NULL==s2||0==*s2)
    size2=0;
  else {
    size2=MultiByteToWideChar(
      CP_UTF8,    // _In_       UINT CodePage,
      0,          // _In_       DWORD dwFlags,
      s2,         // _In_       LPCSTR lpMultiByteStr,
      -1,         // _In_       int cbMultiByte,
      NULL,       // _Out_opt_  LPWSTR lpWideCharStr,
      0           // _In_       int cchWideChar
    );
  }

  if (NULL==s3||0==*s3)
    size3=0;
  else {
    size3=MultiByteToWideChar(
      CP_UTF8,    // _In_       UINT CodePage,
      0,          // _In_       DWORD dwFlags,
      s3,         // _In_       LPCSTR lpMultiByteStr,
      -1,         // _In_       int cbMultiByte,
      NULL,       // _Out_opt_  LPWSTR lpWideCharStr,
      0           // _In_       int cchWideChar
    );
  }

  if (0==(size=size1+size2+size3))
    goto empty;

  if (NULL==(path=MALLOC(size*sizeof *path)))
    goto malloc;

  pp=path;

  if (0<size1) {
    wcscpy(pp,ws1);
    pp+=size1;
    pp[-1]=L'\\';
  }

  if (0<size2) {
    pp+=MultiByteToWideChar(
      CP_UTF8,  // _In_       UINT CodePage,
      0,        // _In_       DWORD dwFlags,
      s2,       // _In_       LPCSTR lpMultiByteStr,
      -1,       // _In_       int cbMultiByte,
      pp,       // _Out_opt_  LPWSTR lpWideCharStr,
      size2     // _In_       int cchWideChar
    );

    pp[-1]=L'\\';
  }

  if (0<size3) {
    pp+=MultiByteToWideChar(
      CP_UTF8,  // _In_       UINT CodePage,
      0,        // _In_       DWORD dwFlags,
      s3,       // _In_       LPCSTR lpMultiByteStr,
      -1,       // _In_       int cbMultiByte,
      pp,       // _Out_opt_  LPWSTR lpWideCharStr,
      size3     // _In_       int cchWideChar
    );
  }
malloc:
empty:
  return path;
}
#else // } {
char *ffsox_path3(const char *s1, const char *s2, const char *s3)
{
  char *path,*pp;
  int size1,size2,size3,size;

  path=NULL;
  size1=NULL==s1||0==*s1?0:strlen(s1)+1;
  size2=NULL==s2||0==*s2?0:strlen(s2)+1;
  size3=NULL==s3||0==*s3?0:strlen(s3)+1;
  
  if (0==(size=size1+size2+size3))
    goto empty;

  if (NULL==(path=MALLOC(size)))
    goto path;

  pp=path;

  if (0<size1) {
    strcpy(pp,s1);
    pp+=size1;
    pp[-1]='/';
  }

  if (0<size2) {
    strcpy(pp,s2);
    pp+=size2;
    pp[-1]='/';
  }

  if (0<size3)
    strcpy(pp,s3);
path:
empty:
  return path;
}
#endif // }
