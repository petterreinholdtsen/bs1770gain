CFLAGS+=-Wall
CFLAGS+=-Wextra
CFLAGS+=-Werror

CPPFLAGS+=-I$(top_srcdir)/libpbutil
CPPFLAGS+=-I$(top_srcdir)/lib1770-2

$(libffsox_2_a_OBJECTS): $(CONFIG_HEADER)
$(libffsox_2_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil.h
$(libffsox_2_a_OBJECTS): $(top_srcdir)/libpbutil/pbutil_priv.h
$(libffsox_2_a_OBJECTS): $(top_srcdir)/lib1770-2/lib1770.h

noinst_LIBRARIES=libffsox_2.a
libffsox_2_a_SOURCES=
libffsox_2_a_SOURCES+=ffsox.h
libffsox_2_a_SOURCES+=ffsox_priv.h
libffsox_2_a_SOURCES+=ffsox_dynload.h
libffsox_2_a_SOURCES+=ffsox_aggregate.c
libffsox_2_a_SOURCES+=ffsox_analyze.c
libffsox_2_a_SOURCES+=ffsox_audio_player.c
libffsox_2_a_SOURCES+=ffsox_audiostream.c
libffsox_2_a_SOURCES+=ffsox_av_time_base_q.c
libffsox_2_a_SOURCES+=ffsox_collect.c
libffsox_2_a_SOURCES+=ffsox_convert.c
libffsox_2_a_SOURCES+=ffsox_csv2avdict.c
libffsox_2_a_SOURCES+=ffsox_dynload.c
libffsox_2_a_SOURCES+=ffsox_dynload_trace.c
libffsox_2_a_SOURCES+=ffsox_find_decoder.c
libffsox_2_a_SOURCES+=ffsox_frame.c
libffsox_2_a_SOURCES+=ffsox_frame_consumer.c
libffsox_2_a_SOURCES+=ffsox_frame_convert.c
libffsox_2_a_SOURCES+=ffsox_frame_convert_sox.c
libffsox_2_a_SOURCES+=ffsox_frame_reader.c
libffsox_2_a_SOURCES+=ffsox_frame_writer.c
libffsox_2_a_SOURCES+=ffsox_machine.c
libffsox_2_a_SOURCES+=ffsox_msvcrt.c
libffsox_2_a_SOURCES+=ffsox_node.c
libffsox_2_a_SOURCES+=ffsox_packet_consumer.c
libffsox_2_a_SOURCES+=ffsox_packet_consumer_list.c
libffsox_2_a_SOURCES+=ffsox_packet_writer.c
libffsox_2_a_SOURCES+=ffsox_path3.c
libffsox_2_a_SOURCES+=ffsox_sink.c
libffsox_2_a_SOURCES+=ffsox_source.c
libffsox_2_a_SOURCES+=ffsox_source_link.c
libffsox_2_a_SOURCES+=ffsox_source_progress.c
libffsox_2_a_SOURCES+=ffsox_sox_add_effect.c
libffsox_2_a_SOURCES+=ffsox_sox_pull_handler.c
libffsox_2_a_SOURCES+=ffsox_sox_reader.c
libffsox_2_a_SOURCES+=ffsox_sox_read_handler.c
libffsox_2_a_SOURCES+=ffsox_stream.c
