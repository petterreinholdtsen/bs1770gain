/*
 * bs1770gain.h
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
#ifndef __BS1770GAIN_H__
#define __BS1770GAIN_H__ // {
#include <ffsox.h>
#include <dirent.h>
#ifdef __cpluplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_MESSAGE(m)   PBU_MESSAGE(m)

#define BS1770GAIN_GOTO(condition,message,label) do { \
  if (condition) { \
    BS1770GAIN_MESSAGE(message); \
    goto label; \
  } \
} while (0)

///////////////////////////////////////////////////////////////////////////////
typedef struct bs1770gain_block_options bs1770gain_block_options_t;
typedef struct bs1770gain_options bs1770gain_options_t;
typedef struct bs1770gain_tag bs1770gain_tag_t;
typedef struct bs1770gain_tree_vmt bs1770gain_tree_vmt_t;
typedef struct bs1770gain_tree bs1770gain_tree_t;
typedef struct bs1770gain_album bs1770gain_album_t;
typedef struct bs1770gain_track bs1770gain_track_t;

///////////////////////////////////////////////////////////////////////////////
char *bs1770gain_opath(const char *ipath, const char *odirname,
    const char *oext);
char *bs1770gain_opathx(int n, const char *title, const char *odirname,
    const char *oext);

int bs1770gain_transcode(bs1770gain_track_t *track,
    const bs1770gain_options_t *options);
// parse time in microseconds.
int64_t bs1770gain_parse_time(const char *s);

void bs1770gain_aggregate_print(ffsox_aggregate_t *aggregate,
    const bs1770gain_options_t *options);
double bs1770gain_aggregate_get_loudness(const ffsox_aggregate_t *aggregate,
    const bs1770gain_options_t *options);

///////////////////////////////////////////////////////////////////////////////
// *must* begin with 0 (defines default)
#define BS1770GAIN_MODE_RG_TAGS               0
#define BS1770GAIN_MODE_BWF_TAGS              1
#define BS1770GAIN_MODE_APPLY                 2

// *must* begin with 0 (defines default)
#define BS1770GAIN_METHOD_MOMENTARY_MEAN      0
#define BS1770GAIN_METHOD_MOMENTARY_MAXIMUM   1
#define BS1770GAIN_METHOD_SHORTTERM_MEAN      2
#define BS1770GAIN_METHOD_SHORTTERM_MAXIMUM   3

enum {
  BS1770GAIN_EXTENSION_RENAME=1<<1,
  BS1770GAIN_EXTENSION_CSV=1<<2,
  BS1770GAIN_EXTENSION_JPG=1<<3,
  BS1770GAIN_EXTENSION_TAGS=1<<4,
  BS1770GAIN_EXTENSION_ALL
      =BS1770GAIN_EXTENSION_RENAME
      |BS1770GAIN_EXTENSION_CSV
      |BS1770GAIN_EXTENSION_JPG
      |BS1770GAIN_EXTENSION_TAGS
};

struct bs1770gain_block_options {
  double ms;
  int partition;
  double mean_gate;
  double range_gate;
  double range_lower_bound;
  double range_upper_bound;
};

struct bs1770gain_options {
  FILE *f;
  double level;
  double preamp;
  double drc;
  int64_t begin;
  int64_t duration;
  int audio;
  int video;
  int flags;
  bs1770gain_block_options_t momentary;
  bs1770gain_block_options_t shortterm;
  int truepeak;
  int samplepeak;
  int mono2stereo;
  int mode;
  int method;
  int extensions;
  int dump;
  double apply;
  int time;
  const char *format;
  const char *video_ext;
  const char *audio_ext;
};

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_tag {
  const char *key;
  char val[32];
};

extern bs1770gain_tag_t bs1770gain_tags[];

///////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_TREE_STATE_INV   0
#define BS1770GAIN_TREE_STATE_REG   1
#define BS1770GAIN_TREE_STATE_DIR   2

struct bs1770gain_tree_vmt {
  void (*cleanup)(bs1770gain_tree_t *tree);
  int (*next)(bs1770gain_tree_t *tree, const bs1770gain_options_t *options);
};

struct bs1770gain_tree {
  const bs1770gain_tree_vmt_t *vmt;
  bs1770gain_tree_t *root;
  const bs1770gain_tree_t *parent;
  const char *path;
  const char *basename;
  int state;
  int vi,ai;

  union {
    struct {
      int argc;
      char **argv;
      int optind;
      int count;
    } cli;

    struct {
      const char *root;
      DIR *d;
      char *path;
    } dir;
  };
};

bs1770gain_tree_t *bs1770gain_tree_init(bs1770gain_tree_t *tree,
    const bs1770gain_tree_vmt_t *vmt, bs1770gain_tree_t *root,
    const bs1770gain_tree_t *parent);

int bs1770gain_tree_analyze(bs1770gain_tree_t *tree, const char *odirname,
    const bs1770gain_options_t *options);
int bs1770gain_tree_track(bs1770gain_tree_t *tree, bs1770gain_album_t *album,
    const bs1770gain_options_t *options);
int bs1770gain_tree_album(const bs1770gain_tree_t *root, const char *odirname,
    const bs1770gain_options_t *options);

int bs1770gain_tree_stat(bs1770gain_tree_t *tree, char *path,
    const bs1770gain_options_t *options);

///////////////////////////////////////////////////////////////////////////////
bs1770gain_tree_t *bs1770gain_tree_cli_init(bs1770gain_tree_t *tree,
    int argc, char **argv, int optind);
void bs1770gain_tree_cli_cleanup(bs1770gain_tree_t *tree);
int bs1770gain_tree_cli_next(bs1770gain_tree_t *tree,
    const bs1770gain_options_t *options);

///////////////////////////////////////////////////////////////////////////////
bs1770gain_tree_t *bs1770gain_tree_dir_init(bs1770gain_tree_t *tree,
    bs1770gain_tree_t *cli, const bs1770gain_tree_t *parent,
    const char *root);
void bs1770gain_tree_dir_cleanup(bs1770gain_tree_t *tree);
int bs1770gain_tree_dir_next(bs1770gain_tree_t *tree,
    const bs1770gain_options_t *options);
void bs1770gain_tree_free_path(bs1770gain_tree_t *tree);

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_album {
  char *ipath;
  char *opath;
  ffsox_aggregate_t aggregate;

  int n;
  bs1770gain_track_t *head;
  bs1770gain_track_t *tail;
};

bs1770gain_album_t *bs1770gain_album_new(const char *ipath, const char *opath,
    const bs1770gain_options_t *options);
void bs1770gain_album_close(bs1770gain_album_t *album);

void bs1770gain_album_copy_file(const bs1770gain_album_t *album,
    const char *basename);
void bs1770gain_album_renumber(const bs1770gain_album_t *album);

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_track {
  char *ipath;
  char *opath;
  ffsox_aggregate_t aggregate;

  int n;
  bs1770gain_album_t *album;
  bs1770gain_track_t *next;
};

bs1770gain_track_t *bs1770gain_track_new(const char *ipath,
    bs1770gain_album_t *album, const bs1770gain_options_t *options);
void bs1770gain_track_close(bs1770gain_track_t *track);

int bs1770gain_track_alloc_output(bs1770gain_track_t *track,
    const ffsox_source_t *si, const bs1770gain_options_t *options);

#ifdef __cpluplus
}
#endif
#endif // }
