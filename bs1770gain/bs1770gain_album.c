/*
 * bs1770gain_album.c
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
#include <bs1770gain_priv.h>
#include <sys/types.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////
bs1770gain_album_t *bs1770gain_album_new(const char *ipath, const char *opath,
    const bs1770gain_options_t *options)
{
  bs1770gain_album_t *album;

  if (NULL==(album=MALLOC(sizeof *album))) {
    DMESSAGE("allocating album");
    goto album;
  }

  if (NULL==ipath)
    album->ipath=NULL;
  else if (NULL==(album->ipath=STRDUP(ipath))) {
     DMESSAGE("copying input path");
     goto ipath;
  }

  if (NULL==opath)
    album->opath=NULL;
  else if (NULL==(album->opath=STRDUP(opath))) {
    DMESSAGE("copying output path");
    goto opath;
  }

  if (ffsox_aggregate_create(&album->aggregate,options->aggregate)<0) {
    DMESSAGE("creating album aggregator");
    goto aggregate;
  }
  
  album->n=0;
  album->head=NULL;
  album->tail=NULL;

  return album;
// cleanup:
  ffsox_aggregate_cleanup(&album->aggregate);
aggregate:
  if (NULL!=album->opath)
    FREE(album->opath);
opath:
  if (NULL!=album->ipath)
    FREE(album->ipath);
ipath:
  FREE(album);
album:
  return NULL;
}

void bs1770gain_album_close(bs1770gain_album_t *album)
{
  bs1770gain_track_t *track;

  while (NULL!=album->head) {
    track=album->head;

    if (NULL==(album->head=track->next))
      album->tail=NULL;

    bs1770gain_track_close(track);
  }


  ffsox_aggregate_cleanup(&album->aggregate);

  if (NULL!=album->opath)
    FREE(album->opath);

  if (NULL!=album->ipath)
    FREE(album->ipath);

  FREE(album);
}

void bs1770gain_album_renumber(const bs1770gain_album_t *album)
{
  int n=0;
  bs1770gain_track_t *track;

  for (track=album->head;NULL!=track;track=track->next)
    track->n=++n;
}

int bs1770gain_album_copy_file(const bs1770gain_album_t *album,
    const char *basename)
{
  int code=-1;
#if defined (_WIN32) // {
  wchar_t *src,*dst;
  struct _stat buf;
#else // } {
  char *src,*dst;
  struct stat buf;
#endif // }

  // try to open the source file.
  if (NULL==(src=ffsox_path3(NULL,album->ipath,basename)))
    goto src;

#if defined (_WIN32) // {
  // if the file do not exist exit.
  if (_wstat(src,&buf)<0)
    goto stat;
#else // } {
  if (stat(src,&buf)<0)
    goto stat;
#endif // }

  // try to open the destination file.
  if (NULL==(dst=ffsox_path3(NULL,album->opath,basename)))
    goto dst;

  // copy the file.
  code=pbu_copy_file(src,dst);
  FREE(dst);
dst:
stat:
  FREE(src);
src:
  return code;
}

///////////////////////////////////////////////////////////////////////////////
#if defined (BS1770GAIN_OVERWRITE) // [
bs1770gain_track_t *bs1770gain_track_new(const char *idir, const char *ipath,
    bs1770gain_album_t *album, const bs1770gain_options_t *options)
#else // ] [
bs1770gain_track_t *bs1770gain_track_new(const char *ipath,
    bs1770gain_album_t *album, const bs1770gain_options_t *options)
#endif // ]
{
  bs1770gain_track_t *track;
  bs1770gain_track_t *prev,*cur;

  if (NULL==(track=MALLOC(sizeof *track))) {
    DMESSAGE("allocation track");
    goto track;
  }

#if defined (BS1770GAIN_OVERWRITE) // [
  if (!options->overwrite||NULL==idir)
    track->idir=NULL;
  else if (NULL==(track->idir=STRDUP(idir))) {
    DMESSAGE("copying input directory");
    goto idir;
  }
#endif // ]

  if (NULL==(track->ipath=STRDUP(ipath))) {
    DMESSAGE("copying input path");
    goto ipath;
  }

  if (ffsox_aggregate_create(&track->aggregate,options->aggregate)<0) {
    DMESSAGE("creating album aggregator");
    goto aggregate;
  }
  
  track->opath=NULL;
  track->album=album;
  track->n=++album->n;

  prev=NULL;
  cur=album->head;

  while (NULL!=cur&&strcmp(cur->ipath,track->ipath)<0) {
    prev=cur;
    cur=cur->next;
  }

  if (NULL==prev)
    album->head=track;
  else
    prev->next=track;

  if (NULL==cur) {
    album->tail=track;
    track->next=NULL;
  }
  else
    track->next=cur;

  return track;
// cleanup:
  ffsox_aggregate_cleanup(&track->aggregate);
aggregate:
  FREE(track->ipath);
ipath:
#if defined (BS1770GAIN_OVERWRITE) // [
  if (track->idir)
    FREE(track->idir);
idir:
#endif // ]
  FREE(track);
track:
  return NULL;
}

void bs1770gain_track_close(bs1770gain_track_t *track)
{
  ffsox_aggregate_cleanup(&track->aggregate);

  if (NULL!=track->opath)
    FREE(track->opath);

  FREE(track->ipath);
#if defined (BS1770GAIN_OVERWRITE) // [
  if (track->idir)
    FREE(track->idir);
#endif // ]
  FREE(track);
}

int bs1770gain_track_alloc_output(track_t *track, const source_t *si,
    const options_t *options)
{
  int code=-1;
  const char *odirname=track->album->opath;
  AVDictionaryEntry *de=av_dict_get(si->f.fc->metadata,"TITLE",NULL,0);
  const char *ext;
#if defined (BS1770GAIN_OVERWRITE) // [
#if defined (_WIN32) // [
  static const char *const TMPDIR="temp";
  const char *tmpdir;
#else // ] [
  const const char *tmpdir="/tmp";
#endif // ]
#endif // ]

  if (options->format)
    ext=options->format;
  else if (0<=si->vi)
    ext=options->video_ext;
  else if (BS1770GAIN_IS_MODE_APPLY(options->mode))
    ext=options->audio_ext;
  else
    ext=pbu_ext(track->ipath);

#if defined (BS1770GAIN_OVERWRITE) // [
  if (options->overwrite) {
#if defined (_WIN32) // [
    if (NULL==(tmpdir=getenv(TMPDIR))) {
      DMESSAGEV("missing \"%s\" environment variable",TMPDIR);
      goto path;
    }
#endif // ]

    track->opath=bs1770gain_opath(track->ipath,tmpdir,ext);
  }
  else
#endif // ]
  if (0==(EXTENSION_RENAME&options->extensions)||NULL==de)
    track->opath=bs1770gain_opath(track->ipath,odirname,ext);
  else
    track->opath=bs1770gain_opathx(track->n,de->value,odirname,ext);

  if (NULL==track->opath) {
    DMESSAGE("allocating output path");
    goto path;
  }

  code=0;
// cleanup:
path:
  return code;
}

#if defined (BS1770GAIN_OVERWRITE) // [
void bs1770gain_track_rename(track_t *track, const options_t *options)
{
  static const char EXT[]="BS1770GAIN";
  const char *obasename=pbu_basename(track->opath);
#if defined (_WIN32) // [
  BOOL code;
  wchar_t *wipath;
  wchar_t *wirename;
  wchar_t *wopath;
  wchar_t *worename;
#else // ] [
  char *irename;
  char *orename;
  struct stat ibuf;
  struct stat obuf;
#endif // ]

  if (options->overwrite) {
#if defined (_WIN32) // [
    if (NULL==(wipath=pbu_s2w(track->ipath))) {
      DMESSAGE("allocating wide character path name\n");
      goto wipath;
    }

    if (NULL==(wirename=ffsox_path3sep(NULL,track->ipath,EXT,0,L'.'))) {
      DMESSAGE("allocating wide character path name\n");
      goto wirename;
    }

    if (NULL==(wopath=pbu_s2w(track->opath))) {
      DMESSAGE("allocating wide character path name\n");
      goto wopath;
    }

    if (NULL==(worename=ffsox_path3sep(NULL,track->idir,obasename,0,L'\\'))) {
      DMESSAGE("allocating wide character path name\n");
      goto worename;
    }

    code=MoveFileW(
      wipath,                 // _In_ LPCTSTR lpExistingFileName,
      wirename                 // _In_ LPCTSTR lpNewFileName
    );

    if (!code) {
      DMESSAGEV("re-naming files - error code %ld",GetLastError());
      goto rename;
    }

    code=MoveFileExW(
      wopath,                 // _In_ LPCTSTR lpExistingFileName,
      wipath,                 // _In_ LPCTSTR lpNewFileName,
      MOVEFILE_COPY_ALLOWED   // _In_     DWORD   dwFlags
    );

    if (!code) {
      DMESSAGEV("re-naming files - error code %ld",GetLastError());
      goto copy;
    }
  copy:
  rename:
    FREE(worename);
  worename:
    FREE(wopath);
  wopath:
    FREE(wirename);
  wirename:
    FREE(wipath);
  wipath:
#else // ] [
    ///////////////////////////////////////////////////////////////////////////
    if (NULL==(irename=ffsox_path3sep(NULL,track->ipath,EXT,0,L'.'))) {
      DMESSAGE("allocating path name\n");
      goto imalloc;
    }

    if (NULL==(orename=ffsox_path3sep(NULL,track->idir,obasename,0,L'/'))) {
      DMESSAGE("allocating path name\n");
      goto omalloc;
    }

    ///////////////////////////////////////////////////////////////////////////
    if (stat(track->ipath,&ibuf)<0) {
      DMESSAGE("getting file information");
      goto istat;
    }

    if (stat(track->opath,&obuf)<0) {
      DMESSAGE("getting file information");
      goto ostat;
    }

    ///////////////////////////////////////////////////////////////////////////
    if (rename(track->ipath,irename)<0) {
      DMESSAGE("re-naming files");
      goto irename;
    }

    ///////////////////////////////////////////////////////////////////////////
    if (obuf.st_dev==ibuf.st_dev&&rename(track->opath,orename)<0) {
      DMESSAGE("re-naming files");
      goto orename;
    }
    else if (pbu_copy_file(track->opath,orename)<0) {
      DMESSAGE("copying files");
      goto ocopy;
    }
  ocopy:
  orename:
  irename:
  ostat:
  istat:
    FREE(orename);
  omalloc:
    FREE(irename);
  imalloc:
#endif // ]
    ;
  }
}
#endif // ]
