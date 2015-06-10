/*
 * pbu_copy_file.c
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
#include <pbutil.h>

#if defined (WIN32) // {
int pbu_copy_file(const wchar_t *src, const wchar_t *dst)
{
  BOOL b;
  
#if 1 // {
  b=CopyFileW(
    src,  // _In_  LPCTSTR lpExistingFileName,
    dst,  // _In_  LPCTSTR lpNewFileName,
    1     // _In_  BOOL bFailIfExists
  );
#else // } {
  b=CopyFileExW(
    src,    // _In_      LPCTSTR lpExistingFileName,
    dst,    // _In_      LPCTSTR lpNewFileName,
    NULL,   // _In_opt_  LPPROGRESS_ROUTINE lpProgressRoutine,
    NULL,   // _In_opt_  LPVOID lpData,
    FALSE,  // _In_opt_  LPBOOL pbCancel,
    0       // _In_      DWORD dwCopyFlags
);
#endif // }

  return b?0:-1;
}
#else // } {
int pbu_copy_file(const char *src, const char *dst)
{
  enum { SIZE=4096 };
  int code=-1;
  FILE *f1,*f2;
  void *buf;
  size_t size;

  if (NULL==(f1=fopen(src,"rb")))
    goto f1;

  if (NULL==(f2=fopen(dst,"wb")))
    goto f2;

  if (NULL==(buf=malloc(SIZE)))
    goto buf;

  do {
    size=fread(buf,1,SIZE,f1);
  } while (size==fwrite(buf,1,size,f2)&&!feof(f1));

  code=0;
//read:
  free(buf);
buf:
  fclose(f2);
f2:
  fclose(f1);
f1:
  return code;
}
#endif // }
