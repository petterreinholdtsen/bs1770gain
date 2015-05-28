/*
 * bs1770gain_album.c
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
#include <bs1770gain_priv.h>
#include <sys/types.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////
bs1770gain_album_t *bs1770gain_album_new(const char *ipath, const char *opath,
    const bs1770gain_options_t *options)
{
  bs1770gain_album_t *album;

  BS1770GAIN_GOTO(NULL==(album=malloc(sizeof *album)),
      "allocation album",album);

  if (NULL!=ipath) {
    BS1770GAIN_GOTO(NULL==(album->ipath=strdup(ipath)),
        "copying input path ",ipath);
  }
  else
    album->ipath=NULL;

  if (NULL!=opath) {
    BS1770GAIN_GOTO(NULL==(album->opath=strdup(opath)),
        "copying input path ",opath);
  }
  else
    album->opath=NULL;

  BS1770GAIN_GOTO(NULL==(album->stats=bs1770gain_stats_new(options)),
      "allocation album",stats);
  
  album->n=0;
  album->head=NULL;
  album->tail=NULL;

  return album;
// cleanup:
  bs1770gain_stats_close(album->stats);
stats:
  if (NULL!=album->opath);
    free(album->opath);
opath:
  if (NULL!=album->ipath);
    free(album->ipath);
ipath:
  free(album);
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


  bs1770gain_stats_close(album->stats);

  if (NULL!=album->opath)
    free(album->opath);

  if (NULL!=album->ipath)
    free(album->ipath);

  free(album);
}

void bs1770gain_album_renumber(const bs1770gain_album_t *album)
{
  int n=0;
  bs1770gain_track_t *track;

  for (track=album->head;NULL!=track;track=track->next)
    track->n=++n;
}

void bs1770gain_album_copy_file(const bs1770gain_album_t *album,
    const char *basename)
{
#if defined (WIN32) // {
  wchar_t *src,*dst;
  struct _stat buf;
#else // } {
  char *src,*dst;
  struct stat buf;
#endif // }

  // try to open the source file.
  if (NULL==(src=ffsox_path3(NULL,album->ipath,basename)))
    goto src;

#if defined (WIN32) // {
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
  pbu_copy_file(src,dst);
  free(dst);
dst:
stat:
  free(src);
src:
  return;
}

///////////////////////////////////////////////////////////////////////////////
bs1770gain_track_t *bs1770gain_track_new(const char *ipath,
    bs1770gain_album_t *album, const bs1770gain_options_t *options)
{
  bs1770gain_track_t *track;
  bs1770gain_track_t *prev,*cur;

  BS1770GAIN_GOTO(NULL==(track=malloc(sizeof *track)),
      "allocation track",track);
  BS1770GAIN_GOTO(NULL==(track->ipath=strdup(ipath)),
      "copying input path ",ipath);
  BS1770GAIN_GOTO(NULL==(track->stats=bs1770gain_stats_new(options)),
      "allocation album",stats);
  
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
  bs1770gain_stats_close(track->stats);
stats:
  free(track->ipath);
ipath:
  free(track);
track:
  return NULL;
}

void bs1770gain_track_close(bs1770gain_track_t *track)
{
  bs1770gain_stats_close(track->stats);

  if (NULL!=track->opath)
    free(track->opath);

  free(track->ipath);
  free(track);
}

int bs1770gain_track_alloc_output(track_t *track, const source_t *si,
    const options_t *options)
{
  int code=-1;
  const char *odirname=track->album->opath;
  AVDictionaryEntry *de=av_dict_get(si->f.fc->metadata,"TITLE",NULL,0);
  const char *ext;

  if (NULL!=options->format)
    ext=options->format;
  else if (0<=si->vi)
    ext=options->video_ext;
  else if (BS1770GAIN_MODE_APPLY==options->mode)
    ext=options->audio_ext;
  else
    ext=pbu_ext(track->ipath);

  if (0==options->extensions||NULL==de)
    track->opath=bs1770gain_opath(track->ipath,odirname,ext,options);
  else
    track->opath=bs1770gain_opathx(track->n,de->value,odirname,ext,options);

  BS1770GAIN_GOTO(NULL==track->opath,"allocating output path",path);
  code=0;
// cleanup:
path:
  return code;
}
