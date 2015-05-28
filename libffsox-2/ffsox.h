/*
 * ffsox.h
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
#ifndef __FFSOX_H__
#define __FFSOX_H__ // {
#include <ffsox_dynload.h>
#if defined (WIN32) // {
#include <windows.h>
#endif // }
#include <stdlib.h>
#include <string.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
#define FFSOX_MESSAGE(m) fprintf(stderr,"Error " m ": \"%s\" (%d).\n", \
    ffsox_basename(__FILE__),__LINE__)

///////////////////////////////////////////////////////////////////////////////
typedef struct ffsox_list ffsox_list_t;
typedef struct ffsox_format ffsox_format_t;
typedef struct ffsox_stream ffsox_stream_t;
typedef struct ffsox_frame ffsox_frame_t;
typedef struct ffsox_machine ffsox_machine_t;
typedef struct ffsox_read_list ffsox_read_list_t;
typedef struct ffsox_sink ffsox_sink_t;

///////////////////////////////////////////////////////////////////////////////
typedef struct ffsox_node_vmt ffsox_node_vmt_t;
  typedef struct ffsox_source_vmt ffsox_source_vmt_t;
  typedef struct ffsox_read_vmt ffsox_read_vmt_t;
    typedef struct ffsox_read_copy_vmt ffsox_read_copy_vmt_t;
    typedef struct ffsox_read_decode_vmt ffsox_read_decode_vmt_t;
  typedef struct ffsox_write_vmt ffsox_write_vmt_t;
    typedef struct ffsox_write_copy_vmt ffsox_write_copy_vmt_t;
    typedef struct ffsox_write_encode_vmt ffsox_write_encode_vmt_t;
  typedef struct ffsox_filter_vmt ffsox_filter_vmt_t;

typedef struct ffsox_node ffsox_node_t;
  typedef struct ffsox_source ffsox_source_t;
  typedef struct ffsox_read ffsox_read_t;
    typedef struct ffsox_read_copy ffsox_read_copy_t;
    typedef struct ffsox_read_decode ffsox_read_decode_t;
  typedef struct ffsox_write ffsox_write_t;
    typedef struct ffsox_write_copy ffsox_write_copy_t;
    typedef struct ffsox_write_encode ffsox_write_encode_t;
  typedef struct ffsox_filter ffsox_filter_t;

///////////////////////////////////////////////////////////////////////////////
#if defined (WIN32) // {
wchar_t *ffsox_path3(const wchar_t *ws1, const char *s2, const char *s3);
wchar_t *ffsox_wcstok_r(wchar_t *str, const wchar_t *delim,
    wchar_t **saveptr);
char *ffsox_strtok_r(char *str, const char *delim, char **saveptr);
HANDLE ffsox_msvcrt(void);
#else // } {
char *ffsox_path3(const char *s1, const char *s2, const char *s3);
#endif // }
int ffsox_csv2avdict(const char *file, char sep, AVDictionary **metadata);
const char *ffsox_basename(const char *path);

int ffsox_audiostream(AVFormatContext *ic, int *aip, int *vip);

/// list //////////////////////////////////////////////////////////////////////
#define FFSOX_LIST_APPEND(l,n) \
  ffsox_list_append(&(l),&(n),sizeof (n))
#define FFSOX_LIST_NEXT(n,l) \
  (*(n)=(*(n)==NULL||(l)==(*(n))->next?NULL:(*(n))->next))
#define FFSOX_LIST_FOREACH(n,l) \
  for (*(n)=(l);NULL!=*(n);FFSOX_LIST_NEXT(n,l))

struct ffsox_list {
#define FFSOX_LIST_MEM(T) \
  T *prev; \
  T *next;
  FFSOX_LIST_MEM(ffsox_list_t)
};

void *ffsox_list_create(void *node);
int ffsox_list_append(void *head, void *node, size_t size);
void *ffsox_list_remove_link(void *head, void *node);
void ffsox_list_free_full(void *head, void *free_func);
void ffsox_list_free(void *head);

/// format ////////////////////////////////////////////////////////////////////
struct ffsox_format {
  const char *path;
  AVFormatContext *fc;
};

/// stream ////////////////////////////////////////////////////////////////////
struct ffsox_stream {
  AVFormatContext *fc;
  int stream_index;
  AVStream *st;
  AVCodecContext *cc;
  const AVCodec *codec;
};

/// frame /////////////////////////////////////////////////////////////////////
struct ffsox_frame {
  AVFrame *frame;

  struct {
    int64_t frame;
    int64_t stream;
  } nb_samples;
};

int ffsox_frame_create(ffsox_frame_t *f);
void ffsox_frame_cleanup(ffsox_frame_t *f);

int ffsox_frame_complete(ffsox_frame_t *f);
void ffsox_frame_reset(ffsox_frame_t *f);
int ffsox_frame_convert(ffsox_frame_t *fr, ffsox_frame_t *fw, double q);

/// machine ///////////////////////////////////////////////////////////////////
#define FFSOX_MACHINE_STAY   0
#define FFSOX_MACHINE_PUSH   1
#define FFSOX_MACHINE_POP    2

struct ffsox_machine {
  ffsox_source_t *source;
  ffsox_node_t *node;
};

int ffsox_machine_create(ffsox_machine_t *m, ffsox_source_t *s);
void ffsox_machine_cleanup(ffsox_machine_t *m);

int ffsox_machine_loop(ffsox_machine_t *m);

/// read_list /////////////////////////////////////////////////////////////////
struct ffsox_read_list {
#define FFSOX_READ_LIST_MEM(T) \
  FFSOX_LIST_MEM(T) \
  ffsox_read_t *read;
  FFSOX_READ_LIST_MEM(ffsox_read_list_t)
};

void ffsox_read_list_free(ffsox_read_list_t *n);

//// sink /////////////////////////////////////////////////////////////////////
struct ffsox_sink {
  ffsox_format_t f;
};

int ffsox_sink_create(ffsox_sink_t *s, const char *path);
void ffsox_sink_cleanup(ffsox_sink_t *s);

int ffsox_sink_open(ffsox_sink_t *s);
void ffsox_sink_close(ffsox_sink_t *s);

/// node //////////////////////////////////////////////////////////////////////
#define FFSOX_STATE_RUN      0
#define FFSOX_STATE_FLUSH    1
#define FFSOX_STATE_END      2

struct ffsox_node_vmt {
  union {
#define FFSOX_NODE_PARENT_VMT
    FFSOX_NODE_PARENT_VMT

    struct {
      const void *parent;
#define FFSOX_NODE_VMT(T) \
      const char *name; \
      void (*cleanup)(T *n); \
      ffsox_node_t *(*prev)(T *n); \
      ffsox_node_t *(*next)(T *n); \
      int (*run)(T *n);
      FFSOX_NODE_VMT(ffsox_node_t)
    };
  };
};

struct ffsox_node {
  union {
#define FFSOX_NODE_PARENT_MEM
    FFSOX_NODE_PARENT_MEM

    struct {
#define FFSOX_NODE_MEM(T) \
      const T *vmt; \
      int state;
      FFSOX_NODE_MEM(ffsox_node_vmt_t)
    };
  };
};

int ffsox_node_create(ffsox_node_t *node);
void ffsox_node_destroy(ffsox_node_t *n);
const ffsox_node_vmt_t *ffsox_node_get_vmt(void);

/// source ////////////////////////////////////////////////////////////////////
struct ffsox_source_vmt {
  union {
#define FFSOX_SOURCE_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_SOURCE_PARENT_VMT

    struct {
      const ffsox_node_vmt_t *parent;
#define FFSOX_SOURCE_VMT(T) \
      FFSOX_NODE_VMT(T)
      FFSOX_SOURCE_VMT(ffsox_source_t)
    };
  };
};

struct ffsox_source {
  union {
#define FFSOX_SOURCE_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_SOURCE_PARENT_MEM

    struct {
#define FFSOX_SOURCE_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_format_t f; \
 \
      struct { \
        ffsox_read_list_t *h; \
        ffsox_read_list_t *n; \
      } reads; \
 \
      ffsox_read_t *next; \
      int64_t ts; \
      AVPacket pkt;

      FFSOX_SOURCE_MEM(ffsox_source_vmt_t)
    };
  };
};

int ffsox_source_create(ffsox_source_t *n, const char *path);
const ffsox_source_vmt_t *ffsox_source_get_vmt(void);

int ffsox_source_seek(ffsox_source_t *n, int64_t ts);

int ffsox_source_link_create(ffsox_source_t *si, ffsox_sink_t *so, double drc,
    int codec_id, int sample_fmt, double q, int ai, int vi);
void ffsox_source_link_cleanup(ffsox_source_t *n);

/// read //////////////////////////////////////////////////////////////////////
struct ffsox_read_vmt {
  union {
#define FFSOX_READ_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_READ_PARENT_VMT

    struct {
      const ffsox_node_vmt_t *parent;
#define FFSOX_READ_VMT(T) \
      FFSOX_NODE_VMT(T) \
      void (*set_packet)(T *n, AVPacket *pkt);
      FFSOX_READ_VMT(ffsox_read_t)
    };
  };
};

struct ffsox_read {
  union {
#define FFSOX_READ_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_READ_PARENT_MEM

    struct {
#define FFSOX_READ_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_stream_t s; \
      ffsox_write_t *write; \
      ffsox_source_t *prev;
      FFSOX_READ_MEM(ffsox_read_vmt_t)
    };
  };
};

int ffsox_read_create(ffsox_read_t *n, ffsox_source_t *s, int stream_index);
const ffsox_read_vmt_t *ffsox_read_get_vmt(void);

/// read_copy /////////////////////////////////////////////////////////////////
struct ffsox_read_copy_vmt {
  union {
#define FFSOX_READ_COPY_PARENT_VMT \
    FFSOX_READ_PARENT_VMT \
    ffsox_read_vmt_t read;
    FFSOX_READ_COPY_PARENT_VMT

    struct {
      const ffsox_read_vmt_t *parent;
#define FFSOX_READ_COPY_VMT(T) \
      FFSOX_READ_VMT(T)
      FFSOX_READ_COPY_VMT(ffsox_read_copy_t)
    };
  };
};

struct ffsox_read_copy {
  union {
#define FFSOX_READ_COPY_PARENT_MEM \
    FFSOX_READ_PARENT_MEM \
    ffsox_read_t read;
    FFSOX_READ_COPY_PARENT_MEM

    struct {
#define FFSOX_READ_COPY_MEM(T) \
      FFSOX_READ_MEM(T) \
      ffsox_write_copy_t *next; \
      AVPacket *pkt;
      FFSOX_READ_COPY_MEM(ffsox_read_copy_vmt_t)
    };
  };
};

int ffsox_read_copy_create(ffsox_read_copy_t *n, ffsox_source_t *s,
    int stream_index);
ffsox_read_copy_t *ffsox_read_copy_new(ffsox_source_t *s, int stream_index);
const ffsox_read_copy_vmt_t *ffsox_read_copy_get_vmt(void);

/// read_decode ///////////////////////////////////////////////////////////////
struct ffsox_read_decode_vmt {
  union {
#define FFSOX_READ_DECODE_PARENT_VMT \
    FFSOX_READ_PARENT_VMT \
    ffsox_read_vmt_t read;
    FFSOX_READ_DECODE_PARENT_VMT

    struct {
      const ffsox_read_vmt_t *parent;
#define FFSOX_READ_DECODE_VMT(T) \
      FFSOX_READ_VMT(T)
      FFSOX_READ_DECODE_VMT(ffsox_read_decode_t)
    };
  };
};

struct ffsox_read_decode {
  union {
#define FFSOX_READ_DECODE_PARENT_MEM \
    FFSOX_READ_PARENT_MEM \
    ffsox_read_t read;
    FFSOX_READ_DECODE_PARENT_MEM

    struct {
#define FFSOX_READ_DECODE_MEM(T) \
      FFSOX_READ_MEM(T) \
      ffsox_filter_t *next; \
      AVPacket pkt; \
      ffsox_frame_t f;
      FFSOX_READ_DECODE_MEM(ffsox_read_decode_vmt_t)
    };
  };
};

int ffsox_read_decode_create(ffsox_read_decode_t *n, ffsox_source_t *s,
    int stream_index, double drc);
ffsox_read_decode_t *ffsox_read_decode_new(ffsox_source_t *s,
    int stream_index, double drc);
const ffsox_read_decode_vmt_t *ffsox_read_decode_get_vmt(void);

/// write /////////////////////////////////////////////////////////////////////
struct ffsox_write_vmt {
  union {
#define FFSOX_WRITE_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_WRITE_PARENT_VMT

    struct {
      const ffsox_node_vmt_t *parent;
#define FFSOX_WRITE_VMT(T) \
      FFSOX_NODE_VMT(T)
      FFSOX_WRITE_VMT(ffsox_write_t)
    };
  };
};

struct ffsox_write {
  union {
#define FFSOX_WRITE_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_WRITE_PARENT_MEM

    struct {
#define FFSOX_WRITE_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_stream_t s;
      FFSOX_WRITE_MEM(ffsox_write_vmt_t)
    };
  };
};

int ffsox_write_create(ffsox_write_t *n, ffsox_sink_t *s,
    const AVCodec *codec);
int ffsox_write_interleaved(ffsox_write_t *n, AVPacket *pkt);
const ffsox_write_vmt_t *ffsox_write_get_vmt(void);

/// write_copy ////////////////////////////////////////////////////////////////
struct ffsox_write_copy_vmt {
  union {
#define FFSOX_WRITE_COPY_PARENT_VMT \
    FFSOX_WRITE_PARENT_VMT \
    ffsox_write_vmt_t write;
    FFSOX_WRITE_COPY_PARENT_VMT

    struct {
      const ffsox_write_vmt_t *parent;
#define FFSOX_WRITE_COPY_VMT(T) \
      FFSOX_WRITE_VMT(T)
      FFSOX_WRITE_COPY_VMT(ffsox_write_copy_t)
    };
  };
};

struct ffsox_write_copy {
  union {
#define FFSOX_WRITE_COPY_PARENT_MEM \
    FFSOX_WRITE_PARENT_MEM \
    ffsox_write_t write;
    FFSOX_WRITE_COPY_PARENT_MEM

    struct {
#define FFSOX_WRITE_COPY_MEM(T) \
      FFSOX_WRITE_MEM(T) \
      ffsox_read_copy_t *prev; \
      AVPacket **pkt;
      FFSOX_WRITE_COPY_MEM(ffsox_write_copy_vmt_t)
    };
  };
};

int ffsox_write_copy_create(ffsox_write_copy_t *no, ffsox_sink_t *so,
    ffsox_read_copy_t *ni);
ffsox_write_copy_t *ffsox_write_copy_new(ffsox_sink_t *so,
    ffsox_read_copy_t *ni);
const ffsox_write_copy_vmt_t *ffsox_write_copy_get_vmt(void);

/// write_encode //////////////////////////////////////////////////////////////
struct ffsox_write_encode_vmt {
  union {
#define FFSOX_WRITE_ENCODE_PARENT_VMT \
    FFSOX_WRITE_PARENT_VMT \
    ffsox_write_vmt_t write;
    FFSOX_WRITE_ENCODE_PARENT_VMT

    struct {
      const ffsox_write_vmt_t *parent;
#define FFSOX_WRITE_ENCODE_VMT(T) \
      FFSOX_WRITE_VMT(T)
      FFSOX_WRITE_ENCODE_VMT(ffsox_write_encode_t)
    };
  };
};

struct ffsox_write_encode {
  union {
#define FFSOX_WRITE_ENCODE_PARENT_MEM \
    FFSOX_WRITE_PARENT_MEM \
    ffsox_write_t write;
    FFSOX_WRITE_ENCODE_PARENT_MEM

    struct {
#define FFSOX_WRITE_ENCODE_MEM(T) \
      FFSOX_WRITE_MEM(T) \
      ffsox_filter_t *prev; \
      AVPacket pkt;
      FFSOX_WRITE_ENCODE_MEM(ffsox_write_encode_vmt_t)
    };
  };
};

int ffsox_write_encode_create(ffsox_write_encode_t *we, ffsox_sink_t *so,
    ffsox_read_decode_t *rd, int codec_id, int sample_fmt);
ffsox_write_encode_t *ffsox_write_encode_new(ffsox_sink_t *so,
    ffsox_read_decode_t *rd, int codec_id, int sample_fmt);
const ffsox_write_encode_vmt_t *ffsox_write_encode_get_vmt(void);

/// filter ////////////////////////////////////////////////////////////////////
struct ffsox_filter_vmt {
  union {
#define FFSOX_FILTER_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_FILTER_PARENT_VMT

    struct {
      const ffsox_node_vmt_t *parent;
#define FFSOX_FILTER_VMT(T) \
      FFSOX_NODE_VMT(T)
      FFSOX_FILTER_VMT(ffsox_filter_t)
    };
  };
};

struct ffsox_filter {
  union {
#define FFSOX_FILTER_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_FILTER_PARENT_MEM

    struct {
#define FFSOX_FILTER_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_frame_t f; \
      double q; \
      ffsox_read_decode_t *prev; \
      ffsox_write_encode_t *next;
      FFSOX_FILTER_MEM(ffsox_filter_vmt_t)
    };
  };
};

int ffsox_filter_create(ffsox_filter_t *en, ffsox_write_encode_t *we,
    double q);
ffsox_filter_t *ffsox_filter_new(ffsox_write_encode_t *we, double q);
const ffsox_filter_vmt_t *ffsox_filter_get_vmt(void);

#ifdef __cpluplus
}
#endif
#endif // }
