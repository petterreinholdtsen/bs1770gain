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
#include <pbutil.h>
#include <ffsox_dynload.h>
#ifdef __cpluplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
typedef union ffsox_read_ptr ffsox_read_ptr_t;
typedef struct ffsox_convert ffsox_convert_t;
typedef struct ffsox_format ffsox_format_t;
typedef struct ffsox_stream ffsox_stream_t;
typedef struct ffsox_frame ffsox_frame_t;
typedef struct ffsox_machine ffsox_machine_t;
typedef struct ffsox_read_list ffsox_read_list_t;
typedef struct ffsox_packet_consumer_list ffsox_packet_consumer_list_t;
typedef struct ffsox_stream_list ffsox_stream_list_t;
typedef struct ffsox_sink ffsox_sink_t;

///////////////////////////////////////////////////////////////////////////////
typedef struct ffsox_node_vmt ffsox_node_vmt_t;
  typedef struct ffsox_source_vmt ffsox_source_vmt_t;
  typedef struct ffsox_packet_consumer_vmt ffsox_packet_consumer_vmt_t;
    typedef struct ffsox_packet_writer_vmt ffsox_packet_writer_vmt_t;
    typedef struct ffsox_frame_reader_vmt ffsox_frame_reader_vmt_t;
  typedef struct ffsox_frame_consumer_vmt ffsox_frame_consumer_vmt_t;
    typedef struct ffsox_frame_writer_vmt ffsox_frame_writer_vmt_t;
    typedef struct ffsox_sox_reader_vmt ffsox_sox_reader_vmt_t;

typedef struct ffsox_node ffsox_node_t;
  typedef struct ffsox_source ffsox_source_t;
  typedef struct ffsox_packet_consumer ffsox_packet_consumer_t;
    typedef struct ffsox_packet_writer ffsox_packet_writer_t;
    typedef struct ffsox_frame_reader ffsox_frame_reader_t;
  typedef struct ffsox_frame_consumer ffsox_frame_consumer_t;
    typedef struct ffsox_frame_writer ffsox_frame_writer_t;
    typedef struct ffsox_sox_reader ffsox_sox_reader_t;

/// utilities /////////////////////////////////////////////////////////////////
#if defined (WIN32) // {
wchar_t *ffsox_path3(const wchar_t *ws1, const char *s2, const char *s3);
#else // } {
char *ffsox_path3(const char *s1, const char *s2, const char *s3);
#endif // }
int ffsox_csv2avdict(const char *file, char sep, AVDictionary **metadata);

AVCodec *ffsox_find_decoder(enum AVCodecID id);
int ffsox_audiostream(AVFormatContext *ic, int *aip, int *vip);

/// ffsox_read_pointer ////////////////////////////////////////////////////////
union ffsox_read_ptr {
  // interleaved.
  struct { const uint8_t *rp; } u8i;
  struct { const int8_t  *rp; } s8i;
  struct { const int16_t *rp; } s16i;
  struct { const int32_t *rp; } s32i;
  struct { const float   *rp; } flti;
  struct { const double  *rp; } dbli;
  // planar.
  struct { const uint8_t *rp[AV_NUM_DATA_POINTERS]; } u8p;
  struct { const int8_t  *rp[AV_NUM_DATA_POINTERS]; } s8p;
  struct { const int16_t *rp[AV_NUM_DATA_POINTERS]; } s16p;
  struct { const int32_t *rp[AV_NUM_DATA_POINTERS]; } s32p;
  struct { const float   *rp[AV_NUM_DATA_POINTERS]; } fltp;
  struct { const double  *rp[AV_NUM_DATA_POINTERS]; } dblp;
};

/// convert ///////////////////////////////////////////////////////////////////
struct ffsox_convert {
  ffsox_frame_t *fr;
  ffsox_frame_t *fw;
  int channels;
  int nb_samples;
};

void ffsox_convert_setup(ffsox_convert_t *convert, ffsox_frame_t *fr,
    ffsox_frame_t *fw);

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

int ffsox_stream_new(ffsox_stream_t *s, ffsox_sink_t *so, AVCodec *codec);
int ffsox_stream_interleaved_write(ffsox_stream_t *s, AVPacket *pkt);

/// frame /////////////////////////////////////////////////////////////////////
struct ffsox_frame {
  AVFrame *frame;

  struct {
    int frame;
    int64_t stream;
  } nb_samples;
};

int ffsox_frame_create(ffsox_frame_t *f);
int ffsox_frame_create_cc(ffsox_frame_t *f, AVCodecContext *cc);
void ffsox_frame_cleanup(ffsox_frame_t *f);

int ffsox_frame_complete(ffsox_frame_t *f);
void ffsox_frame_reset(ffsox_frame_t *f);
int ffsox_frame_convert(ffsox_frame_t *fr, ffsox_frame_t *fw, double q);
int ffsox_frame_convert_sox(ffsox_frame_t *fr, ffsox_frame_t *fw, double q,
    sox_uint64_t *clipsp);

/// machine ///////////////////////////////////////////////////////////////////
#define FFSOX_MACHINE_PUSH   0
#define FFSOX_MACHINE_POP    1

struct ffsox_machine {
  ffsox_source_t *source;
  ffsox_node_t *node;
};

int ffsox_machine_run(ffsox_machine_t *m, ffsox_node_t *node);

/// stream_list ///////////////////////////////////////////////////////////////
struct ffsox_stream_list {
#define FFSOX_STREAM_LIST_MEM(T) \
  PBU_LIST_MEM(T) \
  ffsox_stream_t *si; \
  ffsox_stream_t *so;
  FFSOX_STREAM_LIST_MEM(ffsox_stream_list_t)
};

/// packet_consumer_list //////////////////////////////////////////////////////
struct ffsox_packet_consumer_list {
#define FFSOX_PACKET_CONSUMER_LIST_MEM(T) \
  PBU_LIST_MEM(T) \
  ffsox_packet_consumer_t *consumer;
  FFSOX_PACKET_CONSUMER_LIST_MEM(ffsox_packet_consumer_list_t)
};

void ffsox_packet_consumer_list_free(ffsox_packet_consumer_list_t *n);

//// sink /////////////////////////////////////////////////////////////////////
struct ffsox_sink {
  ffsox_format_t f;
  ffsox_stream_list_t *streams;
};

int ffsox_sink_create(ffsox_sink_t *s, const char *path);
void ffsox_sink_cleanup(ffsox_sink_t *s);

int ffsox_sink_append(ffsox_sink_t *sink, ffsox_stream_t *si,
    ffsox_stream_t *so);

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
#define FFSOX_NODE_VMT(T,P) \
      const P *parent; \
      const char *name; \
      void (*cleanup)(T *n); \
      ffsox_node_t *(*prev)(T *n); \
      ffsox_node_t *(*next)(T *n); \
      int (*run)(T *n);
      FFSOX_NODE_VMT(ffsox_node_t,void)
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
typedef void (*ffsox_source_callback_t)(const ffsox_source_t *, void *);

struct ffsox_source_vmt {
  union {
#define FFSOX_SOURCE_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_SOURCE_PARENT_VMT

    struct {
#define FFSOX_SOURCE_VMT(T,P) \
      FFSOX_NODE_VMT(T,P)
      FFSOX_SOURCE_VMT(ffsox_source_t,ffsox_node_vmt_t)
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
      int ai,vi; \
      ffsox_source_callback_t cb; \
      void *data; \
 \
      struct { \
        ffsox_packet_consumer_list_t *h; \
        ffsox_packet_consumer_list_t *n; \
      } consumer; \
 \
      ffsox_packet_consumer_t *next; \
      int64_t ts; \
      AVPacket pkt;

      FFSOX_SOURCE_MEM(ffsox_source_vmt_t)
    };
  };
};

int ffsox_source_create(ffsox_source_t *n, const char *path, int ai, int vi,
    ffsox_source_callback_t cb, void *data);
const ffsox_source_vmt_t *ffsox_source_get_vmt(void);

int ffsox_source_append(ffsox_source_t *si, ffsox_packet_consumer_t *pc);
int ffsox_source_seek(ffsox_source_t *n, int64_t ts);

int ffsox_source_link(ffsox_source_t *si, ffsox_sink_t *so, double drc,
    int codec_id, int sample_fmt, double q);

/// packet_consumer ///////////////////////////////////////////////////////////
struct ffsox_packet_consumer_vmt {
  union {
#define FFSOX_PACKET_CONSUMER_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_PACKET_CONSUMER_PARENT_VMT

    struct {
#define FFSOX_PACKET_CONSUMER_VMT(T,P) \
      FFSOX_NODE_VMT(T,P) \
      int (*set_packet)(T *n, AVPacket *pkt);
      FFSOX_PACKET_CONSUMER_VMT(ffsox_packet_consumer_t,ffsox_node_vmt_t)
    };
  };
};

struct ffsox_packet_consumer {
  union {
#define FFSOX_PACKET_CONSUMER_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_PACKET_CONSUMER_PARENT_MEM

    struct {
#define FFSOX_PACKET_CONSUMER_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_stream_t si; \
      ffsox_source_t *prev;
      FFSOX_PACKET_CONSUMER_MEM(ffsox_packet_consumer_vmt_t)
    };
  };
};

int ffsox_packet_consumer_create(ffsox_packet_consumer_t *n,
    ffsox_source_t *si, int stream_index);
const ffsox_packet_consumer_vmt_t *ffsox_packet_consumer_get_vmt(void);

/// packet_writer /////////////////////////////////////////////////////////////
struct ffsox_packet_writer_vmt {
  union {
#define FFSOX_PACKET_WRITER_PARENT_VMT \
    FFSOX_PACKET_CONSUMER_PARENT_VMT \
    ffsox_packet_consumer_vmt_t packet_consumer;
    FFSOX_PACKET_WRITER_PARENT_VMT

    struct {
#define FFSOX_PACKET_WRITER_VMT(T,P) \
      FFSOX_PACKET_CONSUMER_VMT(T,P)
      FFSOX_PACKET_WRITER_VMT(ffsox_packet_writer_t,
          ffsox_packet_consumer_vmt_t)
    };
  };
};

struct ffsox_packet_writer {
  union {
#define FFSOX_PACKET_WRITER_PARENT_MEM \
    FFSOX_PACKET_CONSUMER_PARENT_MEM \
    ffsox_packet_consumer_t packet_consumer;
    FFSOX_PACKET_WRITER_PARENT_MEM

    struct {
#define FFSOX_PACKET_WRITER_MEM(T) \
      FFSOX_PACKET_CONSUMER_MEM(T) \
      ffsox_stream_t so;
      FFSOX_PACKET_WRITER_MEM(ffsox_packet_writer_vmt_t)
    };
  };
};

int ffsox_packet_writer_create(ffsox_packet_writer_t *n, ffsox_source_t *si,
    int stream_index, ffsox_sink_t *so);
ffsox_packet_writer_t *ffsox_packet_writer_new(ffsox_source_t *si,
    int stream_index, ffsox_sink_t *so);
const ffsox_packet_writer_vmt_t *ffsox_packet_writer_get_vmt(void);

/// frame_reader //////////////////////////////////////////////////////////////
struct ffsox_frame_reader_vmt {
  union {
#define FFSOX_FRAME_READER_PARENT_VMT \
    FFSOX_PACKET_CONSUMER_PARENT_VMT \
    ffsox_packet_consumer_vmt_t packet_consumer;
    FFSOX_FRAME_READER_PARENT_VMT

    struct {
#define FFSOX_FRAME_READER_VMT(T,P) \
      FFSOX_PACKET_CONSUMER_VMT(T,P)
      FFSOX_FRAME_READER_VMT(ffsox_frame_reader_t,
          ffsox_packet_consumer_vmt_t)
    };
  };
};

struct ffsox_frame_reader {
  union {
#define FFSOX_FRAME_READER_PARENT_MEM \
    FFSOX_PACKET_CONSUMER_PARENT_MEM \
    ffsox_packet_consumer_t packet_consumer;
    FFSOX_FRAME_READER_PARENT_MEM

    struct {
#define FFSOX_FRAME_READER_MEM(T) \
      FFSOX_PACKET_CONSUMER_MEM(T) \
      AVPacket pkt; \
      ffsox_frame_t fo; \
      ffsox_frame_consumer_t *next;
      FFSOX_FRAME_READER_MEM(ffsox_frame_reader_vmt_t)
    };
  };
};

int ffsox_frame_reader_create(ffsox_frame_reader_t *fr, ffsox_source_t *si,
    int stream_index, double drc);
ffsox_frame_reader_t *ffsox_frame_reader_new(ffsox_source_t *si,
    int stream_index, double drc);
const ffsox_frame_reader_vmt_t *ffsox_frame_reader_get_vmt(void);

/// frame_consumer ////////////////////////////////////////////////////////////
struct ffsox_frame_consumer_vmt {
  union {
#define FFSOX_FRAME_CONSUMER_PARENT_VMT \
    FFSOX_NODE_PARENT_VMT \
    ffsox_node_vmt_t node;
    FFSOX_FRAME_CONSUMER_PARENT_VMT

    struct {
#define FFSOX_FRAME_CONSUMER_VMT(T,P) \
      FFSOX_NODE_VMT(T,P) \
      int (*set_frame)(T *n, ffsox_frame_t *fi);
      FFSOX_FRAME_CONSUMER_VMT(ffsox_frame_consumer_t,ffsox_node_vmt_t)
    };
  };
};

struct ffsox_frame_consumer {
  union {
#define FFSOX_FRAME_CONSUMER_PARENT_MEM \
    FFSOX_NODE_PARENT_MEM \
    ffsox_node_t node;
    FFSOX_FRAME_CONSUMER_PARENT_MEM

    struct {
#define FFSOX_FRAME_CONSUMER_MEM(T) \
      FFSOX_NODE_MEM(T) \
      ffsox_node_t *prev; \
      ffsox_frame_t *fi;
      FFSOX_FRAME_CONSUMER_MEM(ffsox_frame_consumer_vmt_t)
    };
  };
};

int ffsox_frame_consumer_create(ffsox_frame_consumer_t *n);
const ffsox_frame_consumer_vmt_t *ffsox_frame_consumer_get_vmt(void);

/// frame_writer ////////////////////////////////////////////////////////////
struct ffsox_frame_writer_vmt {
  union {
#define FFSOX_FRAME_WRITER_PARENT_VMT \
    FFSOX_FRAME_CONSUMER_PARENT_VMT \
    ffsox_frame_consumer_vmt_t frame_consumer;
    FFSOX_FRAME_WRITER_PARENT_VMT

    struct {
#define FFSOX_FRAME_WRITER_VMT(T,P) \
      FFSOX_FRAME_CONSUMER_VMT(T,P)
      FFSOX_FRAME_WRITER_VMT(ffsox_frame_writer_t,ffsox_frame_consumer_vmt_t)
    };
  };
};

struct ffsox_frame_writer {
  union {
#define FFSOX_FRAME_WRITER_PARENT_MEM \
    FFSOX_FRAME_CONSUMER_PARENT_MEM \
    ffsox_frame_consumer_t frame_consumer;
    FFSOX_FRAME_WRITER_PARENT_MEM

    struct {
#define FFSOX_FRAME_WRITER_MEM(T) \
      FFSOX_FRAME_CONSUMER_MEM(T) \
      double q; \
      ffsox_stream_t so; \
      ffsox_frame_t fo; \
      AVPacket pkt;
      FFSOX_FRAME_WRITER_MEM(ffsox_frame_writer_vmt_t)
    };
  };
};

int ffsox_frame_writer_create(ffsox_frame_writer_t *fc, ffsox_sink_t *so,
    ffsox_frame_reader_t *fr, int codec_id, int sample_fmt, double q);
ffsox_frame_writer_t *ffsox_frame_writer_new(ffsox_sink_t *so,
    ffsox_frame_reader_t *fr, int codec_id, int sample_fmt, double q);
const ffsox_frame_writer_vmt_t *ffsox_frame_writer_get_vmt(void);

/// sox_reader ////////////////////////////////////////////////////////////
struct ffsox_sox_reader_vmt {
  union {
#define FFSOX_SOX_READER_PARENT_VMT \
    FFSOX_FRAME_CONSUMER_PARENT_VMT \
    ffsox_frame_consumer_vmt_t frame_consumer;
    FFSOX_SOX_READER_PARENT_VMT

    struct {
#define FFSOX_SOX_READER_VMT(T,P) \
      FFSOX_FRAME_CONSUMER_VMT(T,P)
      FFSOX_SOX_READER_VMT(ffsox_sox_reader_t,ffsox_frame_consumer_vmt_t)
    };
  };
};

struct ffsox_sox_reader {
  union {
#define FFSOX_SOX_READER_PARENT_MEM \
    FFSOX_FRAME_CONSUMER_PARENT_MEM \
    ffsox_frame_consumer_t frame_consumer;
    FFSOX_SOX_READER_PARENT_MEM

    struct {
#define FFSOX_SOX_READER_MEM(T) \
      FFSOX_FRAME_CONSUMER_MEM(T) \
      sox_encodinginfo_t encoding; \
      sox_signalinfo_t signal; \
      ffsox_frame_t fo; \
      ffsox_frame_consumer_t *next; \
      double q; \
      sox_uint64_t clips; \
      int sox_errno;
      FFSOX_SOX_READER_MEM(ffsox_sox_reader_vmt_t)
    };
  };
};

int ffsox_sox_reader_create(ffsox_sox_reader_t *sa,
    ffsox_frame_reader_t *fr, double q);
ffsox_sox_reader_t *ffsox_sox_reader_new(ffsox_frame_reader_t *fr,
    double q);
const ffsox_sox_reader_vmt_t *ffsox_sox_reader_get_vmt(void);

size_t ffsox_sox_reader_read(ffsox_sox_reader_t *sa, sox_sample_t *buf,
    size_t len);

#ifdef __cpluplus
}
#endif
#endif // }
