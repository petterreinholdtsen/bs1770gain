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
#include <lib1770.h>
#include <dirent.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_MIN(x,y) \
  ((x)<(y)?(x):(y))
#define BS1770GAIN_MESSAGE(m) \
  FFSOX_MESSAGE(m)

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
typedef struct bs1770gain_head bs1770gain_head_t;
typedef struct bs1770gain_stats bs1770gain_stats_t;
typedef struct bs1770gain_album bs1770gain_album_t;
typedef struct bs1770gain_track bs1770gain_track_t;
typedef struct bs1770gain_read bs1770gain_read_t;
typedef struct bs1770gain_convert bs1770gain_convert_t;

///////////////////////////////////////////////////////////////////////////////
const char *bs1770gain_basename(const char *path);
const char *bs1770gain_ext(const char *path);
void bs1770gain_mkdir_dirname(char *path);
char *bs1770gain_extend_path(const char *dirname, const char *basename);
char *bs1770gain_opath(const char *ipath, const char *odirname,
    const char *oext);

int64_t bs1770gain_seek(AVFormatContext *ifc, const bs1770gain_options_t *o);
int bs1770gain_sox(const bs1770gain_options_t *options, const char *path,
    bs1770gain_stats_t *stats);

AVCodec *bs1770gain_find_decoder(enum AVCodecID id);

// parse time in microseconds.
int64_t bs1770gain_parse_time(const char *s);
int bs1770gain_oor(AVPacket *p, const AVFormatContext *ifc,
    const bs1770gain_options_t *options);
#if defined (WIN32) // {
wchar_t *bs1770gain_s2w(const char *s);
int bs1770gain_copy_file(const wchar_t *src, const wchar_t *dst);
#else // } {
int bs1770gain_copy_file(const char *src, const char *dst);
#endif // }
int bs1770gain_same_file(const char *path1, const char *path2);

int bs1770gain_transcode(bs1770gain_stats_t *track, bs1770gain_stats_t *album,
    const char *ipath, const char *opath, const bs1770gain_options_t *options);

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

#define BS1770GAIN_BLOCK_OPTIONS_EMPTY_METHOD(o) \
  (0==(o)->maximum&&0==(o)->mean)
#define BS1770GAIN_BLOCK_OPTIONS_EMPTY(o) \
  (0==(o)->maximum&&0==(o)->mean&&0==(o)->range)
#define BS1770GAIN_PEAK_OPTIONS_EMPTY(o) \
  (0==(o)->samplepeak&&0==(o)->truepeak)

struct bs1770gain_block_options {
  int maximum;
  int mean;
  int range;
  double length;
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
    bs1770gain_options_t *options);
int bs1770gain_tree_track(bs1770gain_tree_t *fs, const char *odirname,
    bs1770gain_options_t *options, bs1770gain_album_t *album);
int bs1770gain_tree_album(const bs1770gain_tree_t *root, const char *odirname,
    bs1770gain_options_t *options);

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
struct bs1770gain_stats {
  lib1770_stats_t *momentary,*shortterm;
  double peak_s,peak_t;
};

bs1770gain_stats_t *bs1770gain_stats_new(const bs1770gain_options_t *options);
void bs1770gain_stats_close(bs1770gain_stats_t *stats);

double bs1770gain_stats_get_loudness(const bs1770gain_stats_t *stats,
    const bs1770gain_options_t *options);
void bs1770gain_stats_merge(bs1770gain_stats_t *lhs, bs1770gain_stats_t *rhs);
void bs1770gain_stats_print(bs1770gain_stats_t *stats,
    bs1770gain_options_t *options);

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_head {
  bs1770gain_stats_t *stats;
  lib1770_block_t *momentary,*shortterm;
  lib1770_pre_t *pre;
};

bs1770gain_head_t *bs1770gain_head_new(bs1770gain_stats_t *stats,
    double sample_rate, int channels,
    const bs1770gain_options_t *options);
void bs1770gain_head_close(bs1770gain_head_t *head);

void bs1770gain_head_flush(bs1770gain_head_t *head);

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_album {
  char *ipath;
  char *opath;
  bs1770gain_stats_t *stats;

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
  bs1770gain_stats_t *stats;

  int n;
  bs1770gain_album_t *album;
  bs1770gain_track_t *next;
};

bs1770gain_track_t *bs1770gain_track_new(const char *ipath, const char *opath,
    const bs1770gain_options_t *options, bs1770gain_album_t *album);
void bs1770gain_track_close(bs1770gain_track_t *track);

///////////////////////////////////////////////////////////////////////////////
typedef int8_t bs1770gain_s8_t;
typedef int16_t bs1770gain_s16_t;
typedef int32_t bs1770gain_s32_t;
typedef float bs1770gain_flt_t;
typedef double bs1770gain_dbl_t;

#define BS1770GAIN_STRUCT_TYPEDEF_I(t) \
typedef struct bs1770_struct_##t##i { \
  bs1770gain_##t##_t *rp,*mp; \
} bs1770_struct_##t##i_t

#define BS1770GAIN_STRUCT_TYPEDEF_P(t) \
typedef struct bs1770_struct_##t##p { \
  bs1770gain_##t##_t *rp[AV_NUM_DATA_POINTERS],*mp; \
} bs1770_struct_##t##p_t

// interleaved.
BS1770GAIN_STRUCT_TYPEDEF_I(s8);
BS1770GAIN_STRUCT_TYPEDEF_I(s16);
BS1770GAIN_STRUCT_TYPEDEF_I(s32);
BS1770GAIN_STRUCT_TYPEDEF_I(flt);
BS1770GAIN_STRUCT_TYPEDEF_I(dbl);
// planar.
BS1770GAIN_STRUCT_TYPEDEF_P(s8);
BS1770GAIN_STRUCT_TYPEDEF_P(s16);
BS1770GAIN_STRUCT_TYPEDEF_P(s32);
BS1770GAIN_STRUCT_TYPEDEF_P(flt);
BS1770GAIN_STRUCT_TYPEDEF_P(dbl);

///////////////////////////////////////////////////////////////////////////////
#define BS1770GAIN_READ_READ   0
#define BS1770GAIN_READ_FLUSH  1
#define BS1770GAIN_READ_EOF    2

struct bs1770gain_read {
  const bs1770gain_options_t *options;
  int state;

  AVFormatContext *ifc;
  AVCodecContext *adc;
  AVFrame *frame;
  AVPacket p1,p2;
  int ai,got_frame;
  int64_t ts;

  sox_signalinfo_t signal;
  sox_uint64_t clips;
  bs1770gain_head_t *head;

  union {
    // interleaved.
    bs1770_struct_s8i_t s8i;
    bs1770_struct_s16i_t s16i;
    bs1770_struct_s32i_t s32i;
    bs1770_struct_flti_t flti;
    bs1770_struct_dbli_t dbli;
    // planar.
    bs1770_struct_s8p_t s8p;
    bs1770_struct_s16p_t s16p;
    bs1770_struct_s32p_t s32p;
    bs1770_struct_fltp_t fltp;
    bs1770_struct_dblp_t dblp;
  };
};

bs1770gain_read_t *bs1770gain_read_new(const bs1770gain_options_t *options,
    const char *path, bs1770gain_stats_t *stats);
void bs1770gain_read_close(bs1770gain_read_t *read);

size_t bs1770gain_read_decode(bs1770gain_read_t *read, sox_sample_t *obuf,
    size_t size);

int bs1770gain_read_decode_frame(bs1770gain_read_t *read);
int bs1770gain_read_eob(bs1770gain_read_t *read);
void bs1770gain_read_reset(bs1770gain_read_t *read);

size_t bs1770gain_read_convert_frame(bs1770gain_read_t *read,
    sox_sample_t *obuf, size_t size);

///////////////////////////////////////////////////////////////////////////////
struct bs1770gain_convert {
  double q;

  struct {
    AVFormatContext *fc;
    int ai;
    AVFrame *frame;
  } i;

  struct {
    AVFormatContext *fc;
    int ai;
    AVFrame *frame;
  } o;
};

bs1770gain_convert_t *bs1770gain_convert_new(AVFormatContext *ifc, int iai,
    AVFormatContext *ofc, int oai, const bs1770gain_options_t *options,
    const bs1770gain_stats_t *track, const bs1770gain_stats_t *album);
void bs1770gain_convert_close(bs1770gain_convert_t *convert);

int bs1770gain_convert_packet(bs1770gain_convert_t *convert, int *got_frame,
    AVPacket *p);

///////////////////////////////////////////////////////////////////////////////
sox_effect_handler_t const *bs1770gain_read_handler(void);
sox_effect_handler_t const *bs1770gain_sink_handler(void);

#ifdef __cpluplus
}
#endif
#endif // }
