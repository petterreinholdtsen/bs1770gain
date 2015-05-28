/*
 * bs1770gain.c
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
#include <getopt.h>
//#include <locale.h>

///////////////////////////////////////////////////////////////////////////////
#define CLOCKS_PER_MILLIS (CLOCKS_PER_SEC/1000)

///////////////////////////////////////////////////////////////////////////////
void bs1770gain_usage(char **argv, int code)
{
  fprintf(stderr,"Usage:  %s [options] <file/dir> [<file/dir> ...]\n",
      bs1770gain_basename(argv[0]));
  fprintf(stderr,"\n");
  fprintf(stderr,"Options:\n");
  fprintf(stderr," -h,--help:  print this message and exit\n");
  /////////////////////////////////////////////////////////////////////////////
  fprintf(stderr," -i,--integrated:  calculate integrated loudness\n");
  fprintf(stderr," -s,--shorterm:  calculate maximum shortterm loudness\n");
  fprintf(stderr," -m,--momentary:  claculate maximum momentary loudness\n");
  fprintf(stderr," -r,--range:  calculate loudness range\n");
  fprintf(stderr," -p,--samplepeak:  calculate maximum sample peak\n");
  fprintf(stderr," -t,--truepeak:  calculate maximum true peak\n");
  fprintf(stderr," -b <timestamp>,--begin <timestamp>:  begin decoding at\n"
      "   timestamp (in microseconds, format: hh:mm:ss.mus)\n");
  fprintf(stderr," -d <duration>,--duration <duration>:  let decoding\n"
      "   last duration (in microseconds, format: hh:mm:ss.mus)\n");
  fprintf(stderr," -u <method>,--use <method>:  base replay gain calculation\n"
      "   on method (with respect to the -a/--apply and -o/--output\n"
      "   options),\n"
      "   method:  integrated (default), shortterm, or momentary\n");
  fprintf(stderr," -a:  apply the EBU/ATSC/RG album gain to the output\n"
      "   (in conjunction with the -o/--output option)\n");
  fprintf(stderr," -o <folder>,--output <folder>:  write RG tags\n"
      "   or apply the EBU/ATSC/RG gain, respectively,\n"
      "   and output to folder\n");
  fprintf(stderr," -f <file>,--file <file>:  write analysis to file\n");
  fprintf(stderr," -x,--extensions:  enable extensions:\n"
      "   1) read metadata from per-folder CSV file \"folder.csv\"\n"
      "   2) copy file \"folder.jpg\" from source to destination\n"
      "      folder\n");
  fprintf(stderr," -l,--list:  print FFmpeg format/stream information\n");
  /////////////////////////////////////////////////////////////////////////////
  fprintf(stderr," --ebu:  calculate replay gain according to EBU R128\n"
      "   (-23.0 LUFS, default)\n");
  fprintf(stderr," --atsc:  calculate replay gain according to ATSC A/85\n"
      "   (-24.0 LUFS)\n");
  fprintf(stderr," --replaygain:  calculate replay gain according to\n"
      "   ReplayGain 2.0 (-18.0 LUFS)\n");
  fprintf(stderr," --apply <q>:  apply the EBU/ATSC/RG gain to the output\n"
      "   (in conjunction with the -o/--output option),\n"
      "   q=0.0 (album gain) ... 1.0 (track gain),\n"
      "   default:  0.0 (album gain)\n");
  fprintf(stderr," --audio <index>:  select audio index (corresponds to\n"
      "   [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  fprintf(stderr," --video <index>:  select video index (corresponds to\n"
      "   [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  fprintf(stderr," --drc <drc>:  set AC3 dynamic range compression (DRC)\n");
  fprintf(stderr," --format <format>:  convert to format\n");
  fprintf(stderr," --loglevel <level>:  set loglevel,\n"
      "   level:  quiet, panic, fatal, error, warning, info,\n"
      "   verbose, debug\n");
  fprintf(stderr," --time:  print out duration of program invocation\n");
  //fprintf(stderr," --level:\n");
  //fprintf(stderr," --preamp <preamp>:\n");
  //fprintf(stderr," --mono2stero:\n");
  //fprintf(stderr," --rg-tags:\n");
  //fprintf(stderr," --bwf-tags:\n");
  /////////////////////////////////////////////////////////////////////////////
  fprintf(stderr,"\n");
  fprintf(stderr,"Command line arguments can appear in any order.\n");
  exit(code);
}

///////////////////////////////////////////////////////////////////////////////
enum {
  ////////////////
  APPLY,
  DRC,
  EXTENSION,
  LOGLEVEL,
  LEVEL,
  PREAMP,
  ////////////////
  TIME,
  EBU,
  ATSC,
  AUDIO,
  VIDEO,
  MONO2STEREO,
  REPLAYGAIN,
  RG_TAGS,
  BWF_TAGS
};

static const const char *bs1770gain_flags="b:d:f:o:u:ahlimprstx";

static struct option bs1770gain_opts[]={
  ////
  { name:"begin",has_arg:required_argument,flag:NULL,val:'b' },
  { name:"duration",has_arg:required_argument,flag:NULL,val:'d' },
  { name:"file",has_arg:required_argument,flag:NULL,val:'f' },
  { name:"output",has_arg:required_argument,flag:NULL,val:'o' },
  { name:"use",has_arg:required_argument,flag:NULL,val:'u' },
  ////
  { name:"help",has_arg:no_argument,flag:NULL,val:'h' },
  { name:"list",has_arg:no_argument,flag:NULL,val:'l' },
  { name:"momentary",has_arg:no_argument,flag:NULL,val:'m' },
  { name:"range",has_arg:no_argument,flag:NULL,val:'r' },
  { name:"samplepeak",has_arg:no_argument,flag:NULL,val:'p' },
  { name:"shortterm",has_arg:no_argument,flag:NULL,val:'s' },
  { name:"truepeak",has_arg:no_argument,flag:NULL,val:'t' },
  { name:"extensions",has_arg:no_argument,flag:NULL,val:'x' },
  ////
  { name:"apply",has_arg:required_argument,flag:NULL,val:APPLY },
  { name:"audio",has_arg:required_argument,flag:NULL,val:AUDIO },
  { name:"drc",has_arg:required_argument,flag:NULL,val:DRC },
  { name:"format",has_arg:required_argument,flag:NULL,val:EXTENSION },
  { name:"loglevel",has_arg:required_argument,flag:NULL,val:LOGLEVEL },
  { name:"level",has_arg:required_argument,flag:NULL,val:LEVEL },
  { name:"preamp",has_arg:required_argument,flag:NULL,val:PREAMP },
  { name:"video",has_arg:required_argument,flag:NULL,val:VIDEO },
  ////
  { name:"time",has_arg:no_argument,flag:NULL,val:TIME },
  { name:"ebu",has_arg:no_argument,flag:NULL,val:EBU },
  { name:"atsc",has_arg:no_argument,flag:NULL,val:ATSC },
  { name:"mono2stereo",has_arg:no_argument,flag:NULL,val:MONO2STEREO },
  { name:"replaygain",has_arg:no_argument,flag:NULL,val:REPLAYGAIN },
  { name:"rg-tags",has_arg:no_argument,flag:NULL,val:RG_TAGS },
  { name:"bwf-tags",has_arg:no_argument,flag:NULL,val:BWF_TAGS },
  { name:NULL,has_arg:0,flag:NULL,val:0 }
};

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  bs1770gain_options_t options;
  bs1770gain_tree_t root;
  char *fpath=NULL;
  char *odirname=NULL;
  int loglevel=AV_LOG_QUIET;
  clock_t t1,t2;
  int c;

  //setlocale(LC_ALL,"C");
  memset(&options,0,sizeof options);
  options.f=stdout;
  options.audio=-1;
  options.video=-1;
  options.level=-23.0;
  options.audio_ext="flac";
  options.video_ext="mkv";

  opterr=0;

  while (0<=(c=getopt_long(argc,argv,bs1770gain_flags,bs1770gain_opts,NULL))) {
    switch (c) {
    ///////////////////////////////////////////////////////////////////////////
    case '?':
      bs1770gain_usage(argv,-1);
      break;
    /// with argument /////////////////////////////////////////////////////////
    case 'b':
      options.begin=bs1770gain_parse_time(optarg);
      break;
    case 'd':
      options.duration=bs1770gain_parse_time(optarg);
      break;
    case 'f':
      fpath=optarg;
      break;
    case 'o':
      odirname=optarg;
      break;
    case 'u':
      if (0==strcasecmp("integrated",optarg)) {
        options.integrated=1;
        options.method=BS1770GAIN_METHOD_INTEGRATED;
      }
      else if (0==strcasecmp("shortterm",optarg)) {
        options.shortterm=1;
        options.method=BS1770GAIN_METHOD_SHORTTERM;
      }
      else if (0==strcasecmp("momentary",optarg)) {
        options.momentary=1;
        options.method=BS1770GAIN_METHOD_MOMENTARY;
      }
      else
        bs1770gain_usage(argv,-1);
      break;
    /// without argument //////////////////////////////////////////////////////
    case 'a':
      options.mode=BS1770GAIN_MODE_APPLY;
      options.apply=0.0;
      break;
    case 'h':
      bs1770gain_usage(argv,0);
      break;
    case 'l':
      options.dump=1;
      break;
    case 'i':
      options.integrated=1;
      break;
    case 'm':
      options.momentary=1;
      break;
    case 'r':
      options.range=1;
      break;
    case 'p':
      options.samplepeak=1;
      break;
    case 's':
      options.shortterm=1;
      break;
    case 't':
      options.truepeak=1;
      break;
    case 'x':
      options.extensions=1;
      break;
    /// without flag //////////////////////////////////////////////////////////
    case AUDIO:
      options.audio=atoi(optarg);
      break;
    case VIDEO:
      options.video=atoi(optarg);
      break;
    case APPLY:
      options.mode=BS1770GAIN_MODE_APPLY;
      options.apply=atof(optarg);
      break;
    case DRC:
      options.drc=atof(optarg);
      break;
    case EXTENSION:
      options.format=optarg;
      break;
    case LOGLEVEL:
      if (0==strcmp("quiet",optarg))
        loglevel=AV_LOG_QUIET;
      else if (0==strcmp("panic",optarg))
        loglevel=AV_LOG_PANIC;
      else if (0==strcmp("fatal",optarg))
        loglevel=AV_LOG_FATAL;
      else if (0==strcmp("error",optarg))
        loglevel=AV_LOG_ERROR;
      else if (0==strcmp("warning",optarg))
        loglevel=AV_LOG_WARNING;
      else if (0==strcmp("info",optarg))
        loglevel=AV_LOG_INFO;
      else if (0==strcmp("verbose",optarg))
        loglevel=AV_LOG_VERBOSE;
      else if (0==strcmp("debug",optarg))
        loglevel=AV_LOG_DEBUG;
      else
        bs1770gain_usage(argv,-1);
      break;
    case LEVEL:
      options.level=atof(optarg);
      break;
    case PREAMP:
      options.preamp=atof(optarg);
      break;
    /// without flag //////////////////////////////////////////////////////////
    case ATSC:
      options.level=-24.0;
      break;
    case EBU:
      options.level=-23.0;
      break;
    case REPLAYGAIN:
      options.level=-18.0;
      break;
    case RG_TAGS:
      options.mode=BS1770GAIN_MODE_RG_TAGS;
      break;
    case BWF_TAGS:
      options.mode=BS1770GAIN_MODE_BWF_TAGS;
      break;
    case TIME:
      options.time=1;
      break;
    case MONO2STEREO:
      options.mono2stereo=1;
      break;
    ///////////////////////////////////////////////////////////////////////////
    default:
      break;
    }
  }

  if (argc==optind)
    bs1770gain_usage(argv,-1);

  if (0==options.integrated&&0==options.shortterm&&0==options.momentary) {
    if (NULL!=odirname||0==options.range)
      options.integrated=1;
  }

  // load the FFmpeg and SoX libraries from "bs1770gain-tools".
  if (bs1770gain_dynload("bs1770gain-tools")<0) {
#if defined (WIN32) // {
    fprintf(stderr,"Error loading dynamic link libraries: \"%s\" (%d).\n",
        bs1770gain_basename(__FILE__),__LINE__);
#else // } {
    fprintf(stderr,"Error loading shared objects: \"%s\" (%d).\n",
        bs1770gain_basename(__FILE__),__LINE__);
#endif // }
    goto dynload;
  }

  av_register_all();
  sox_init();

  if (0==options.dump||av_log_get_level()<loglevel)
    av_log_set_level(loglevel);

  if (NULL!=fpath&&NULL==(options.f=fopen(fpath,"w")))
    goto file;

  bs1770gain_tree_cli_init(&root,argc,argv,optind);
  t1=clock();
  bs1770gain_tree_analyze(&root,odirname,&options);
  t2=clock();
  root.vmt->cleanup(&root);

  if (options.time)
    fprintf(stderr, "Duration: %ld ms.\n",(t2-t1)/CLOCKS_PER_MILLIS);
dynload:
  if (NULL!=fpath)
    fclose(options.f);
file:
  return 0;
}
