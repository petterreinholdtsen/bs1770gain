/*
 * ffsox_dynload_trace.c
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
#define FFSOX_DYNLOAD_PRIV
#include <ffsox_priv.h>
#if defined (FFSOX_DYNLOAD) && defined (PBU_MALLOC_DEBUG) // {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
AVCodec *ffsox_avcodec_find_decoder(enum AVCodecID id)
{
  pbu_trace_puts("ffsox_avcodec_find_decoder()");

  return ffsox_avcodec.avcodec_find_decoder(id);
}

AVCodec *ffsox_avcodec_find_decoder_by_name(const char *name)
{
  pbu_trace_puts("ffsox_avcodec_find_decoder_by_name()");

  return ffsox_avcodec.avcodec_find_decoder_by_name(name);
}

AVCodec *ffsox_avcodec_find_encoder(enum AVCodecID id)
{
  pbu_trace_puts("ffsox_avcodec_find_encoder()");

  return ffsox_avcodec.avcodec_find_encoder(id);
}

int ffsox_avcodec_open2(AVCodecContext *avctx, const AVCodec *codec,
    AVDictionary **options)
{
  pbu_trace_puts("ffsox_avcodec_open2()");

  return ffsox_avcodec.avcodec_open2(avctx,codec,options);
}

void ffsox_av_init_packet(AVPacket *pkt)
{
  pbu_trace_puts("ffsox_av_init_packet()");
  ffsox_avcodec.av_init_packet(pkt);
}

int ffsox_avcodec_decode_audio4(AVCodecContext *avctx, AVFrame *frame,
    int *got_frame_ptr, const AVPacket *avpkt)
{
  pbu_trace_puts("ffsox_avcodec_decode_audio4()");

  return ffsox_avcodec.avcodec_decode_audio4(avctx,frame,
      got_frame_ptr,avpkt);
}

int ffsox_avcodec_encode_audio2(AVCodecContext *avctx, AVPacket *avpkt,
    const AVFrame *frame, int *got_packet_ptr)
{
  pbu_trace_puts("ffsox_avcodec_encode_audio2()");

  return ffsox_avcodec.avcodec_encode_audio2(avctx,avpkt,
      frame,got_packet_ptr);
}

int ffsox_avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture,
    int *got_picture_ptr, const AVPacket *avpkt)
{
  pbu_trace_puts("ffsox_avcodec_decode_video2()");

  return ffsox_avcodec.avcodec_decode_video2(avctx,picture,
      got_picture_ptr,avpkt);
}

#if defined (FFSOX_DEPRECATED_AV_FREE_PACKET) // [
void ffsox_av_free_packet(AVPacket *pkt)
{
  pbu_trace_puts("ffsox_av_free_packet()");
  ffsox_avcodec.av_free_packet(pkt);
}
#else // ] [
void ffsox_av_packet_unref(AVPacket *pkt)
{
  pbu_trace_puts("ffsox_av_packet_unref()");
  ffsox_avcodec.av_packet_unref(pkt);
}
#endif // ]

int ffsox_avcodec_close(AVCodecContext *avctx)
{
  pbu_trace_puts("ffsox_avcodec_close()");

  return ffsox_avcodec.avcodec_close(avctx);
}

int ffsox_avcodec_copy_context(AVCodecContext *dest, const AVCodecContext *src)
{
  pbu_trace_puts("ffsox_avcodec_copy_context()");

  return ffsox_avcodec.avcodec_copy_context(dest,src);
}

void ffsox_av_packet_rescale_ts(AVPacket *pkt, AVRational tb_src,
    AVRational tb_dst)
{
  pbu_trace_puts("ffsox_av_packet_rescale_ts()");
  ffsox_avcodec.av_packet_rescale_ts(pkt,tb_src,tb_dst);
}

///////////////////////////////////////////////////////////////////////////////
void ffsox_av_register_all(void)
{
  pbu_trace_puts("av_register_all()");
  ffsox_avformat.av_register_all();
}

int ffsox_avformat_open_input(AVFormatContext **ps, const char *filename,
    AVInputFormat *fmt, AVDictionary **options)
{
  pbu_trace_puts("avformat_open_input()");

  return ffsox_avformat.avformat_open_input(ps,filename,fmt,options);
}

int ffsox_avformat_find_stream_info(AVFormatContext *ic,
    AVDictionary **options)
{
  pbu_trace_puts("avformat_find_stream_info()");

  return ffsox_avformat.avformat_find_stream_info(ic,options);
}

int ffsox_av_read_frame(AVFormatContext *s, AVPacket *pkt)
{
  pbu_trace_puts("av_read_frame()");

  return ffsox_avformat.av_read_frame(s,pkt);
}

void ffsox_avformat_close_input(AVFormatContext **s)
{
  pbu_trace_puts("avformat_close_input()");

  ffsox_avformat.avformat_close_input(s);
}

int ffsox_avformat_alloc_output_context2(AVFormatContext **ctx,
    AVOutputFormat *oformat, const char *format_name,
    const char *filename)
{
  pbu_trace_puts("avformat_alloc_output_context2()");

  return ffsox_avformat.avformat_alloc_output_context2(ctx,oformat,
      format_name,filename);
}

void ffsox_avformat_free_context(AVFormatContext *s)
{
  pbu_trace_puts("avformat_free_context()");

  ffsox_avformat.avformat_free_context(s);
}

AVStream *ffsox_avformat_new_stream(AVFormatContext *s, const AVCodec *c)
{
  pbu_trace_puts("avformat_new_stream()");

  return ffsox_avformat.avformat_new_stream(s,c);
}

int ffsox_avio_open(AVIOContext **s, const char *url, int flags)
{
  pbu_trace_puts("avio_open()");

  return ffsox_avformat.avio_open(s,url,flags);
}

int ffsox_avio_close(AVIOContext *s)
{
  pbu_trace_puts("avio_close()");

  return ffsox_avformat.avio_close(s);
}

int ffsox_avformat_write_header(AVFormatContext *s, AVDictionary **options)
{
  pbu_trace_puts("avformat_write_header()");

  return ffsox_avformat.avformat_write_header(s,options);
}

int ffsox_av_interleaved_write_frame(AVFormatContext *s, AVPacket *pkt)
{
  pbu_trace_puts("av_interleaved_write_frame()");

  return ffsox_avformat.av_interleaved_write_frame(s,pkt);
}

int ffsox_av_write_trailer(AVFormatContext *s)
{
  pbu_trace_puts("av_write_trailer()");

  return ffsox_avformat.av_write_trailer(s);
}

int ffsox_av_find_default_stream_index(AVFormatContext *s)
{
  pbu_trace_puts("av_find_default_stream_index()");

  return ffsox_avformat.av_find_default_stream_index(s);
}

int ffsox_avformat_seek_file(AVFormatContext *s, int stream_index,
    int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
{
  pbu_trace_puts("avformat_seek_file()");

  return ffsox_avformat.avformat_seek_file(s,stream_index,
      min_ts,ts,max_ts,flags);
}

void ffsox_av_dump_format(AVFormatContext *ic, int index, const char *url,
    int is_output)
{
  pbu_trace_puts("av_dump_format()");
  ffsox_avformat.av_dump_format(ic,index,url,is_output);
}

#endif // }
