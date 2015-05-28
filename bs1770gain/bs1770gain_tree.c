/*
 * bs1770gain_tree.c
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
#include <bs1770gain.h>
//#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////
bs1770gain_tree_t *bs1770gain_tree_init(bs1770gain_tree_t *tree,
    const bs1770gain_tree_vmt_t *vmt, bs1770gain_tree_t *root,
    const bs1770gain_tree_t *parent)
{
  tree->vmt=vmt;
  tree->root=root;
  tree->parent=parent;
  tree->path=NULL;
  tree->basename=NULL;
  tree->state=BS1770GAIN_TREE_STATE_INV;
  tree->ai=-1;
  tree->vi=-1;

  return tree;
}

///////////////////////////////////////////////////////////////////////////////
static void bs1770gain_ffmpeg_dump(const char *path)
{
  AVFormatContext *ifc;

  ifc=NULL;

  if (avformat_open_input(&ifc,path,NULL,NULL)<0)
    goto ifc;

  if (avformat_find_stream_info(ifc, NULL)<0)
    goto find;

  av_dump_format(ifc,0,path,0);
find:
  avformat_close_input(&ifc);
ifc:
  return;
}

int bs1770gain_tree_analyze(bs1770gain_tree_t *tree, const char *odirname,
    bs1770gain_options_t *options)
{
  int code=-1;
  const bs1770gain_tree_t *parent=tree->parent;
  const char *ibasename=NULL==parent?NULL:parent->basename;
  FILE *f=options->f;
  bs1770gain_album_t *album;
  bs1770gain_track_t *track;
  const char *label;

  album=bs1770gain_album_new(NULL==parent?NULL:parent->path,odirname,options);
  BS1770GAIN_GOTO(NULL==album,"allocation album",album);

  while (0<tree->vmt->next(tree,options)) {
    switch (tree->state) {
    case BS1770GAIN_TREE_STATE_REG:
      BS1770GAIN_GOTO(bs1770gain_tree_track(tree,odirname,options,album)<0,
          "initializing track",track);
      break;
    case BS1770GAIN_TREE_STATE_DIR:
      BS1770GAIN_GOTO(bs1770gain_tree_album(tree,odirname,options)<0,
          "initializing album",track);
      break;
    default:
      break;
    }
  }

  bs1770gain_album_renumber(album);

  if (NULL==album->head)
    ;
  else if (0==options->dump) {
    if (NULL==ibasename||0==*ibasename)
      fprintf(f,"analyzing ...\n");
    else
      fprintf(f,"analyzing \"%s\" ...\n",ibasename);

    for (track=album->head;NULL!=track;track=track->next) {
      fprintf(f,"  [%d/%d] \"%s\"",track->n,album->n,
          bs1770gain_basename(track->ipath));
      fprintf(f,": ");
      fflush(f);

      if (bs1770gain_sox(options,track->ipath,track->stats)<0)
        fprintf(f,"Error gathering track statistics.\n");
      else {
        fprintf(f,stdout==f?"        \n":"\n");
        bs1770gain_stats_print(track->stats,options);
        bs1770gain_stats_merge(album->stats,track->stats);
      }
    }

    fprintf(f,"  [ALBUM]:\n");
    bs1770gain_stats_print(album->stats,options);

    if (NULL!=album->opath) {
      label=BS1770GAIN_MODE_APPLY==options->mode?"transcoding":"remuxing";

      // print a massage.
      if (NULL==ibasename||0==*ibasename)
        fprintf(f,"%s ...\n",label);
      else
        fprintf(f,"%s \"%s\" ...\n",label,ibasename);

      // mkdir.
      bs1770gain_mkdir_dirname(album->opath);

      // copy the tracks.
      for (track=album->head;NULL!=track;track=track->next) {
        // print a start massage.
        if (stdout==f) {
          fprintf(f,"  [%d/%d] \"%s\" ",track->n,album->n,
              bs1770gain_basename(track->opath));
          fflush(f);
        }

        if (0!=bs1770gain_same_file(track->ipath,track->opath)) {
          if (stdout==f) {
            fprintf(f,"... ");
            fflush(f);
          }

          // copy the track.
          if (bs1770gain_transcode(track->stats,album->stats,track->ipath,
              track->opath,options)<0) {

            if (stdout==f)
              fprintf(f,"Error %s track.",label);
          }
          else {
            // print a done massage.
            if (stdout==f)
              fprintf(f,"done.\n");
          }
        }
        else if (stdout==f)
          fprintf(f,"not written.\n");
      }

      // copy "folder.jpg" if requested.
      if (0!=options->extensions&&NULL!=album->ipath)
        bs1770gain_album_copy_file(album,"folder.jpg");
    }

    fprintf(f,"done.\n");
  }
  else {
    for (track=album->head;NULL!=track;track=track->next)
      bs1770gain_ffmpeg_dump(track->ipath);
  }

  code=0;
track:
  bs1770gain_album_close(album);
album:
  return code;
}

int bs1770gain_tree_track(bs1770gain_tree_t *tree, const char *odirname,
    bs1770gain_options_t *options, bs1770gain_album_t *album)
{
  int code;
  const char *ext;
  char *path;

  code =-1;

  if (NULL!=odirname) {
    if (NULL!=options->format)
      ext=options->format;
    else if (0<=tree->vi)
      ext=options->video_ext;
    else if (BS1770GAIN_MODE_APPLY==options->mode)
      ext=options->audio_ext;
    else
      ext=bs1770gain_ext(tree->path);

    path=bs1770gain_opath(tree->basename,odirname,ext);
    BS1770GAIN_GOTO(NULL==path,"extending output path",path);
  }
  else
    path=NULL;

  BS1770GAIN_GOTO(NULL==bs1770gain_track_new(tree->path,path,options,album),
      "allocation album",track);
  code=0;
// cleanup:
track:
  if (NULL!=path)
    free(path);
path:
  return code;
}

int bs1770gain_tree_album(const bs1770gain_tree_t *root, const char *odirname,
    bs1770gain_options_t *options)
{
  int code;
  char *path;
  bs1770gain_tree_t tree;

  code =-1;

  if (NULL!=odirname) {
    path=bs1770gain_extend_path(odirname,root->basename);
    BS1770GAIN_GOTO(NULL==path,"extending output path",path);
  }
  else
    path=NULL;

  BS1770GAIN_GOTO(NULL==bs1770gain_tree_dir_init(&tree,root->root,root,
      root->path),"initilaizing directory",dir);
  fprintf(options->f,"%s\n",root->path);
  bs1770gain_tree_analyze(&tree,path,options);
  code=0;
// cleanp:
  tree.vmt->cleanup(&tree);
dir:
  if (NULL!=path)
    free(path);
path:
  return code;
}

static int bs1770gain_invalid_ext(const char *path)
{
  static const char *exts[]={
    "txt",
    "csv",
    "jpg",
    "jpeg",
    "png",
    "gif",
    NULL
  };

  const char *ext,**ep;

  ext=bs1770gain_ext(path);

  for (ep=exts;NULL!=*ep;++ep) {
    if (0==strcasecmp(*ep,ext))
      return 1;
  }

  return 0;
}

static int bs1770gain_tree_multimedia(bs1770gain_tree_t *tree,
    const bs1770gain_options_t *options)
{
  int code=0;
  AVFormatContext *ifc=NULL;

  if (avformat_open_input(&ifc,tree->path,NULL,NULL)<0)
    goto ifc;

  if (avformat_find_stream_info(ifc,NULL)<0)
    goto find;

  tree->ai=options->audio;
  tree->vi=options->video;

  if (ffsox_audiostream(ifc,&tree->ai,&tree->vi)<0) {
    BS1770GAIN_MESSAGE("finding streams");
    goto find;
  }

  code=1;
// cleanup:
find:
  avformat_close_input(&ifc);
ifc:
  return code;
}

int bs1770gain_tree_stat(bs1770gain_tree_t *tree, char *path,
    const bs1770gain_options_t *options)
{
#if defined (WIN32) // {
  struct _stat buf;
  wchar_t *wpath;
#else // } {
  struct stat buf;
#endif // }
  char *p;

  if (NULL!=path) {
    p=path+strlen(path);

    // TODO unicode.
    while (path<p&&('/'==p[-1]||'\\'==p[-1]))
      *--p=0;
  }

  tree->path=path;
  tree->basename=bs1770gain_basename(path);
#if defined (WIN32) // {
  wpath=bs1770gain_s2w(path);

  if (NULL==wpath)
    tree->state=BS1770GAIN_TREE_STATE_INV;
  else if (_wstat(wpath,&buf)<0)
    tree->state=BS1770GAIN_TREE_STATE_INV;
#else // } {
  if (NULL==path)
    tree->state=BS1770GAIN_TREE_STATE_INV;
  else if (stat(path,&buf)<0)
    tree->state=BS1770GAIN_TREE_STATE_INV;
#endif // }
  else if (S_ISREG(buf.st_mode)) {
    if (bs1770gain_invalid_ext(bs1770gain_ext(tree->path)))
      tree->state=BS1770GAIN_TREE_STATE_INV;
    else if (bs1770gain_tree_multimedia(tree,options))
      tree->state=BS1770GAIN_TREE_STATE_REG;
    else
      tree->state=BS1770GAIN_TREE_STATE_INV;
  }
  else if (S_ISDIR(buf.st_mode))
    tree->state=BS1770GAIN_TREE_STATE_DIR;
  else
    tree->state=BS1770GAIN_TREE_STATE_INV;

#if defined (WIN32) // {
  if (NULL!=wpath)
    free(wpath);
#endif // }

  if (BS1770GAIN_TREE_STATE_REG==tree->state)
    ++tree->root->cli.count;

  return tree->state;
}

///////////////////////////////////////////////////////////////////////////////
bs1770gain_tree_vmt_t bs1770gain_tree_cli_vmt={
  .cleanup=bs1770gain_tree_cli_cleanup,
  .next=bs1770gain_tree_cli_next
};

bs1770gain_tree_t *bs1770gain_tree_cli_init(bs1770gain_tree_t *tree,
    int argc, char **argv, int optind)
{
  bs1770gain_tree_init(tree,&bs1770gain_tree_cli_vmt,tree,NULL);
  tree->cli.argc=argc;
  tree->cli.argv=argv;
  tree->cli.optind=optind;
  tree->cli.count=0;

  return tree;
}

void bs1770gain_tree_cli_cleanup(bs1770gain_tree_t *tree)
{
}

int bs1770gain_tree_cli_next(bs1770gain_tree_t *tree,
    const bs1770gain_options_t *options)
{
  char *path;

  while (tree->cli.optind<tree->cli.argc) {
    path=tree->cli.argv[tree->cli.optind++];

    if (0<bs1770gain_tree_stat(tree,path,options))
      return tree->state;
  }

  return BS1770GAIN_TREE_STATE_INV;
}

///////////////////////////////////////////////////////////////////////////////
bs1770gain_tree_vmt_t bs1770gain_tree_dir_vmt={
  .cleanup=bs1770gain_tree_dir_cleanup,
  .next=bs1770gain_tree_dir_next
};

bs1770gain_tree_t *bs1770gain_tree_dir_init(bs1770gain_tree_t *tree,
    bs1770gain_tree_t *cli, const bs1770gain_tree_t *parent,
    const char *root)
{
  bs1770gain_tree_init(tree,&bs1770gain_tree_dir_vmt,cli,parent);
  tree->dir.root=root;
  tree->dir.path=NULL;

  if (NULL==(tree->dir.d=opendir(root)))
    goto dir;

  return tree;
dir:
  return NULL;
}

void bs1770gain_tree_dir_cleanup(bs1770gain_tree_t *tree)
{
  bs1770gain_tree_free_path(tree);
  closedir(tree->dir.d);
}

int bs1770gain_tree_dir_next(bs1770gain_tree_t *tree,
    const bs1770gain_options_t *options)
{
  struct dirent *e;

  bs1770gain_tree_free_path(tree);

  while (NULL!=(e=readdir(tree->dir.d))) {
    if (0==strcmp(".",e->d_name)||0==strcmp("..",e->d_name))
      continue;
    else if (NULL==(tree->dir.path=bs1770gain_extend_path(tree->dir.root,
          e->d_name)))
      continue;
    else if (0<bs1770gain_tree_stat(tree,tree->dir.path,options))
      return tree->state;

    bs1770gain_tree_free_path(tree);
  }

  return BS1770GAIN_TREE_STATE_INV;
}

void bs1770gain_tree_free_path(bs1770gain_tree_t *tree)
{
  if (NULL!=tree->dir.path) {
    free(tree->dir.path);
    tree->dir.path=NULL;
    tree->basename=NULL;
    tree->path=NULL;
    tree->state=BS1770GAIN_TREE_STATE_INV;
  }
}
