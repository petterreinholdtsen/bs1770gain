/*
 * pbu_same_file.c
 * Copyright (C) 2015 Peter Belkner <pbelkner@users.sf.net>
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
#include <pbutil.h>
#if defined (_WIN32) // {
#include <windows.h>
typedef BY_HANDLE_FILE_INFORMATION info_t;

static int b21770gain_get_file_info(const char *path, info_t *info)
{
  int code=-1;
  HANDLE hFile;
  BOOL b;

  memset(info,0,sizeof *info);

  hFile=CreateFileA(
    path,	 	        // __in      LPCTSTR lpFileName,
    GENERIC_READ,   // __in      DWORD dwDesiredAccess,
    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    // __in      DWORD dwShareMode,
    NULL,			      // __in_opt  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OPEN_EXISTING,  // __in      DWORD dwCreationDisposition,
    0,				      // __in      DWORD dwFlagsAndAttributes,
    NULL			      // __in_opt  HANDLE hTemplateFile
  );

  if (NULL==hFile)
    goto file;

  b=GetFileInformationByHandle(
    hFile,          // _In_   HANDLE hFile,
    info            // _Out_  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
  );

  if (0==b)
    goto info;

  code=0;
info:
  CloseHandle(hFile);
file:
  return code;
}

int pbu_same_file(const char *path1, const char *path2)
{
  int code=-1;
  info_t info1,info2;

  if (b21770gain_get_file_info(path1,&info1)<0)
    goto info1;

  if (b21770gain_get_file_info(path2,&info2)<0)
    goto info2;

  code=info1.nFileIndexHigh!=info2.nFileIndexHigh
      ||info1.nFileIndexLow!=info2.nFileIndexLow;
info2:
info1:
  return code;
}
#else // } {
#include <sys/types.h>
#include <sys/stat.h>

int pbu_same_file(const char *path1, const char *path2)
{
  int code=-1;
  struct stat buf1,buf2;

  if (stat(path1,&buf1)<0)
    goto buf1;

  if (stat(path2,&buf2)<0)
    goto buf2;

  code=buf1.st_ino!=buf2.st_ino||buf1.st_dev!=buf2.st_dev;
buf2:
buf1:
  return code;
}
#endif // }
