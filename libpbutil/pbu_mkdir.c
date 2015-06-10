/*
 * pbu_mkdir.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@snafu.de>
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
#if defined (WIN32) // {
#include <direct.h>
#else // } {
#include <sys/stat.h>
#endif // }

void pbu_mkdir(char *path)
{
#if defined (WIN32) // {
  size_t size;
  wchar_t *wpath,*p1,*p2;
#else // } {
  char *p1,*p2;
#endif // }
  int ch;

#if defined (WIN32) // {
  size=MultiByteToWideChar(
    CP_UTF8,      // __in       UINT CodePage,
    0,            // __in       DWORD dwFlags,
    path,         // __in       LPCSTR lpMultiByteStr,
    -1,           // __in       int cbMultiByte,
    NULL,         // __out_opt  LPWSTR lpWideCharStr,
    0             // __in       int cchWideChar
  );

  if (NULL==(wpath=malloc(size*(sizeof *wpath)))) {
  	MESSAGE("allocating wide path");
    goto wpath;
  }

  size=MultiByteToWideChar(
    CP_UTF8,      // __in       UINT CodePage,
    0,            // __in       DWORD dwFlags,
    path,         // __in       LPCSTR lpMultiByteStr,
    -1,           // __in       int cbMultiByte,
    wpath,        // __out_opt  LPWSTR lpWideCharStr,
    size          // __in       int cchWideChar
  );

  p1=wpath;
#else // } {
  p1=path;
#endif // }

  for (;;) {
    p2=p1;

    // TODO: unicode.
    while (0!=*p2&&'/'!=*p2&&'\\'!=*p2)
      ++p2;

    if (0==*p2) {
#if defined (WIN32) // {
      _wmkdir(wpath);
#else // } {
      mkdir(path,S_IRWXU|S_IRWXG|S_IRWXO);
#endif // }
      break;
    }
    else {
      ch=*p2;
      *p2=0;
#if defined (WIN32) // {
      _wmkdir(wpath);
#else // } {
      mkdir(path,S_IRWXU|S_IRWXG|S_IRWXO);
#endif // }
      *p2=ch;
      p1=p2+1;
    }
  }

#if defined (WIN32) // {
  free(wpath);
wpath:
  return;
#endif // }
}
