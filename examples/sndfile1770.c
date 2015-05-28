/*
 * sndfile1770.c
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
#include <lib1770.h>
#include <time.h>
#include <sndfile.h>

#define BUFFER_LEN 1024

char *basename(char *path)
{
  char *p=path+strlen(path);

  while (path<p&&'/'!=p[-1]&&'\\'!=p[-1])
    --p;

  return p;
}

void usage(char **argv)
{
  fprintf(stderr,"usage: %s <wav> [<wav> ...]\n",basename(argv[0]));
  exit(1);
}

int main(int argc, char **argv)
{
  static double data[BUFFER_LEN];

  SNDFILE *infile;
  SF_INFO sfinfo;
  int readcount;
  double *dp;
  int i,ch;
  lib1770_stats_t *stats_im,*stats_rs;
  lib1770_block_t *block_04,*block_30;
  lib1770_pre_t *pre;
  lib1770_sample_t sample;
  clock_t t1,t2;

  // if there are not suficcient arguments exit.
  if (argc<2)
    usage(argv);

  t1=clock();

  // for each file ...
  for (i=1;i<argc;++i) {
    ///////////////////////////////////////////////////////////////////////////
    // open the input file.
    if (NULL==(infile=sf_open(argv[i],SFM_READ,&sfinfo)))
      goto infile;

    // if the input file has other than 1 channels continue.
    if (2!=sfinfo.channels)
      goto channels;

    // print out the file name.
    fprintf(stdout,"\"%s\" ...",basename(argv[i]));
    fflush(stdout);

    ///////////////////////////////////////////////////////////////////////////
    // open a history statistics for integrated loudness.
    // open a maximum statistics for momentary loudness.
    if (NULL==(stats_im=lib1770_stats_new()))
      goto stats_im;

    // open a 0.4 s / 0.75 overlap block for integrated and momentary loudness.
    if (NULL==(block_04=lib1770_block_new(sfinfo.samplerate,400.0,4)))
      goto block_04;

    // add the integrated and the momentary statistics to the respective block.
    lib1770_block_add_stats(block_04,stats_im);

    ///////////////////////////////////////////////////////////////////////////
    // open a history statistics for loudness range.
    // open a maximum statistics for short term loudness.
    if (NULL==(stats_rs=lib1770_stats_new()))
      goto stats_rs;

    // open a 3 ms / 0.66 overlap block for loudness range and short term
    // loudness.
    if (NULL==(block_30=lib1770_block_new(sfinfo.samplerate,3000.0,3)))
      goto block_30;

    // add the loudness range and the sort term statistics to the respective
    // block.
    lib1770_block_add_stats(block_30,stats_rs);

    ///////////////////////////////////////////////////////////////////////////
    // open a pre-filter.
    if (NULL==(pre=lib1770_pre_new(sfinfo.samplerate,sfinfo.channels)))
      goto pre;

    // add the 0.4 s / 0.75 overlap and the 3 ms / 0.66 overlap block to the
    // pre-filter.
    lib1770_pre_add_block(pre,block_04);
    lib1770_pre_add_block(pre,block_30);

    ///////////////////////////////////////////////////////////////////////////
    // add samples.
    while (0<(readcount=sf_read_double(infile,data,BUFFER_LEN))) {
      dp=data;

      while (0<readcount) {
        for (ch=0;ch<sfinfo.channels;++ch)
          if (ch<LIB1770_MAX_CHANNELS)
            sample[ch]=*dp++;
          else
            ++dp;

        lib1770_pre_add_sample(pre,sample);
        readcount-=sfinfo.channels;
      }
    }

    lib1770_pre_flush(pre);

    ///////////////////////////////////////////////////////////////////////////
    // print out the results.
    fprintf(stdout,"\b\b\b   \n");
    fprintf(stdout,"  M %.1f, S %.1f, I %.1f, R %.1f\n",
        lib1770_stats_get_max(stats_im),
        lib1770_stats_get_max(stats_rs),
        lib1770_stats_get_mean(stats_im,-10.0),
        lib1770_stats_get_range(stats_rs,-20.0,0.1,0.95));

    ///////////////////////////////////////////////////////////////////////////
    // cleanup.
    lib1770_pre_close(pre);
  pre:
    lib1770_block_close(block_30);
  block_30:
    lib1770_stats_close(stats_rs);
  stats_rs:
    lib1770_block_close(block_04);
  block_04:
    lib1770_stats_close(stats_im);
  stats_im:
  channels:
    sf_close(infile);
  infile:
    continue;
  }

  t2=clock();
  fprintf(stderr, "Duration: %.0f ms.\n", (double)(t2-t1));

  return 0;
}
