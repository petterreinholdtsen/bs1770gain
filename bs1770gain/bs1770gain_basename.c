/*
 * bs1770gain.c
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
#include <windows.h>
#include <direct.h>
#endif // }
#include <bs1770gain.h>

const char *bs1770gain_basename(const char *path)
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

const char *bs1770gain_ext(const char *path)
{
  const char *p=path+strlen(path);

  // TODO: unicode.
  while (path<p&&'.'!=p[-1])
    --p;

  return p;
}

void bs1770gain_mkdir_dirname(char *path)
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

  wpath=malloc(size*(sizeof *wpath));
  BS1770GAIN_GOTO(NULL==wpath,"allocating wide path",wpath);

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
      mkdir(path);
#endif // }
      break;
    }
    else {
      ch=*p2;
      *p2=0;
#if defined (WIN32) // {
      _wmkdir(wpath);
#else // } {
      mkdir(path);
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

char *bs1770gain_extend_path(const char *dirname, const char *basename)
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

  BS1770GAIN_GOTO(NULL==(path=malloc((len1+1)+(len2+1))),
      "allocating path",path);
  p2=path;
  memcpy(p2,dirname,len1);
  p2+=len1;
  *p2++='/';
  strcpy(p2,basename);
path:
  return path;
}

char *bs1770gain_opath(const char *ipath, const char *odirname,
    const char *oext)
{
  const char *p1,*p2;;
  size_t len1,len2,len3;
  char *opath,*p;

  len1=strlen(odirname);
  p1=bs1770gain_basename(ipath);
  p2=p1+strlen(p1);

  // TODO: unicode.
  while (p1<p2&&'.'!=*p2)
    --p2;

  len2=p2-p1;
  len3=strlen(oext);

  if (NULL==(opath=malloc((len1+1)+(len2+1)+(len3+1))))
    goto opath;

  p=opath;
  
  memcpy(p,odirname,len1);
  p+=len1;
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

