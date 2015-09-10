/*
 * bs1770gain.c
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
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
#include <bs1770gain_priv.h>
#include <getopt.h>
//#include <locale.h>

///////////////////////////////////////////////////////////////////////////////
#define CLOCKS_PER_MILLIS (CLOCKS_PER_SEC/1000)

#define BS1770GAIN_AGGREGATE_METHOD ( \
  FFSOX_AGGREGATE_MOMENTARY_MAXIMUM \
  &FFSOX_AGGREGATE_MOMENTARY_MEAN \
  &FFSOX_AGGREGATE_SHORTTERM_MAXIMUM \
  &FFSOX_AGGREGATE_SHORTTERM_MEAN \
)

#define BS1770GAIN_AGGREGATE_RANGE_PEAK ( \
  FFSOX_AGGREGATE_MOMENTARY_RANGE \
  &FFSOX_AGGREGATE_SHORTTERM_RANGE \
  &FFSOX_AGGREGATE_SAMPLEPEAK \
  &FFSOX_AGGREGATE_TRUEPEAK \
)

#define BS1770GAIN_OPTIONS_NO_METHOD(o) \
  (0==((o)->aggregate&BS1770GAIN_AGGREGATE_METHOD))
#define BS1770GAIN_OPTIONS_NO_RANGE_PEAK(o) \
  (0==((o)->aggregate&BS1770GAIN_AGGREGATE_RANGE_PEAK))

///////////////////////////////////////////////////////////////////////////////
void bs1770gain_usage(char **argv, int code)
{
#if defined (PACKAGE_VERSION) // {
  fprintf(stderr,"BS1770GAIN %s, Copyright (C) Peter Belkner 2014, 2015.\n",
      PACKAGE_VERSION);
#else // } {
  fprintf(stderr,"BS1770GAIN, Copyright (C) Peter Belkner 2014, 2015.\n");
#endif // }
#if defined (PACKAGE_URL) // {
  fprintf(stderr,"%s\n",PACKAGE_URL);
#endif // }
  fprintf(stderr,"\n");
  fprintf(stderr,"Usage:  %s [options] <file/dir> [<file/dir> ...]\n",
      pbu_basename(argv[0]));
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
      "   methods:  \"integrated\" (default), \"shortterm\", or\n"
      "   \"momentary\"\n"
      "   experimental methods:  \"momentary-mean\" (same as\n"
      "   \"integrated\"), \"momentary-maximum\" (same as \"momentary\"),\n"
      "   \"shortterm-mean\", or \"shortterm-maximum\" (same as\n"
      "   \"shortterm\")\n");
  fprintf(stderr," -a:  apply the EBU/ATSC/RG album gain to the output\n"
      "   (in conjunction with the -o/--output option)\n");
  fprintf(stderr," -o <folder>,--output <folder>:  write RG tags\n"
      "   or apply the EBU/ATSC/RG gain, respectively,\n"
      "   and output to folder\n");
  fprintf(stderr," -f <file>,--file <file>:  write analysis to file\n");
  fprintf(stderr," -x,--extensions:  enable following extensions/defaults:\n"
      "   1) rename files according to TITLE tag\n"
      "   2) read metadata from per-folder CSV file \"folder.csv\"\n"
      "   3) copy file \"folder.jpg\" from source to destination\n"
      "      folder\n"
      "   4) automatically add the TRACK and DISC tags\n"
      "   5) calculate maximum true peak\n"
      "   6) convert to stereo\n");
  fprintf(stderr," -l,--list:  print FFmpeg format/stream information\n");
  /////////////////////////////////////////////////////////////////////////////
  fprintf(stderr," --ebu:  calculate replay gain according to EBU R128\n"
      "   (-23.0 LUFS, default)\n");
  fprintf(stderr," --atsc:  calculate replay gain according to ATSC A/85\n"
      "   (-24.0 LUFS)\n");
  fprintf(stderr," --replaygain:  calculate replay gain according to\n"
      "   ReplayGain 2.0 (-18.0 LUFS)\n");
  fprintf(stderr," --track-tags:  write track tags\n");
  fprintf(stderr," --album-tags:  write album tags\n");
#if defined (BS1770GAIN_TAG_PREFIX) // {
#if 0 // {
  fprintf(stderr," --tag-prefix <prefix>:  use <prefix> as tag prefix\n"
      "   instead of \"REPLAYGAIN\"\n");
#else // } {
  fprintf(stderr," --tag-prefix <prefix>:  instead of \"REPLAYGAIN\",\n"
      "   use <prefix> as RG tag prefix\n");
#endif // }
#endif // }
  fprintf(stderr," --unit <unit>:  write tags with <unit>\n");
  fprintf(stderr," --apply <q>:  apply the EBU/ATSC/RG gain to the output\n"
      "   (in conjunction with the -o/--output option),\n"
      "   q=0.0 (album gain) ... 1.0 (track gain),\n"
      "   default:  0.0 (album gain)\n");
  fprintf(stderr," --audio <index>:  select audio index (corresponds to\n"
      "   [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  fprintf(stderr," --video <index>:  select video index (corresponds to\n"
      "   [0:<index>] in FFmpeg listing, cf. -l/--list option)\n");
  fprintf(stderr," --stereo:  convert to stereo\n");
  fprintf(stderr," --drc <drc>:  set AC3 dynamic range compression (DRC)\n");
  fprintf(stderr," --extension <extension>:  enable extension out of\n"
      "   \"rename\":  rename files according to TITLE tag\n"
      "   \"csv\":  read metadata from per-folder CSV file \"folder.csv\"\n"
      "   \"jpg\":  copy file \"folder.jpg\" from source to destination\n"
      "      folder\n"
      "   \"tags\":  automatically add the TRACK and DISC tags\n");
  fprintf(stderr," --format <format>:  convert to format\n");
  fprintf(stderr," --loglevel <level>:  set loglevel,\n"
      "   level:  \"quiet\", \"panic\", \"fatal\", \"error\", \"warning\",\n"
      "   \"info\", \"verbose\", \"debug\"\n");
  fprintf(stderr," --xml:  print results in xml format\n");
  fprintf(stderr," --time:  print out duration of program invocation\n");
  //fprintf(stderr," --level:\n");
  //fprintf(stderr," --preamp <preamp>:\n");
  //fprintf(stderr," --stero:\n");
  //fprintf(stderr," --rg-tags:\n");
  //fprintf(stderr," --bwf-tags:\n");
  /////////////////////////////////////////////////////////////////////////////
  fprintf(stderr,"\n");
  fprintf(stderr,"Experimental options:\n");
  ////////
  fprintf(stderr,"1) momentary block\n");
  fprintf(stderr," --momentary-mean:  calculate mean loudness based on\n"
      "   momentary block (same as --integrated)\n");
  fprintf(stderr," --momentary-maximum:  calculate maximum loudness based\n"
      "   on momentary block (same as --momentary)\n");
  fprintf(stderr," --momentary-range:  calculate loudness range based on\n"
      "   momentary block\n");
  fprintf(stderr," --momentary-length <ms>:  length of momentary block\n"
      "   in milliseconds (default: 400)\n");
  fprintf(stderr," --momentary-overlap <percent>:  overlap of momentary\n"
      "   block in percent (default: 75)\n");
  fprintf(stderr," --momentary-mean-gate <gate>:  silence gate for mean\n"
      "   measurement of momentary block (default: -10.0)\n");
  fprintf(stderr," --momentary-range-gate <gate>:  silence gate for range\n"
      "   measurement of momentary block (default: -20.0)\n");
  fprintf(stderr," --momentary-range-lower-bound <float>:  lower bound for\n"
      "   range measurement of momentary block (default: 0.1)\n");
  fprintf(stderr," --momentary-range-upper-bound <float>:  upper bound for\n"
      "   range measurement of momentary block (default: 0.95)\n");
  ////////
  fprintf(stderr,"2) shortterm block\n");
  fprintf(stderr," --shortterm-mean:  calculate mean loudness based on\n"
      "   shortterm block\n");
  fprintf(stderr," --shortterm-maximum:  calculate maximum loudness based\n"
      "   on shortterm block (same as --shortterm)\n");
  fprintf(stderr," --shortterm-range:  calculate loudness range based on\n"
      "   shortterm block (same as --range)\n");
  fprintf(stderr," --shortterm-length <ms>:  length of shortterm block\n"
      "   in milliseconds (default: 3000)\n");
  fprintf(stderr," --shortterm-overlap <percent>:  overlap of shortterm\n"
      "   block in percent (default: 67)\n");
  fprintf(stderr," --shortterm-mean-gate <gate>:  silence gate for mean\n"
      "   measurement of shortterm block (default: -10.0)\n");
  fprintf(stderr," --shortterm-range-gate <gate>:  silence gate for range\n"
      "   measurement of shortterm block (default: -20.0)\n");
  fprintf(stderr," --shortterm-range-lower-bound <float>:  lower bound for\n"
      "   range measurement of shortterm block (default: 0.1)\n");
  fprintf(stderr," --shortterm-range-upper-bound <float>:  upper bound for\n"
      "   range measurement of shortterm block (default: 0.95)\n");
  /////////////////////////////////////////////////////////////////////////////
  fprintf(stderr,"\n");
  fprintf(stderr,"Command line arguments can appear in any order.\n");
  exit(code);
}

///////////////////////////////////////////////////////////////////////////////
enum {
  ////////////////
  APPLY,
  UNIT,
  DRC,
  FORMAT,
  LOGLEVEL,
  LEVEL,
  PREAMP,
  ////////////////
  TIME,
  EBU,
  EXTENSION,
  ATSC,
  AUDIO,
  VIDEO,
  STEREO,
  REPLAYGAIN,
  RG_TAGS,
  BWF_TAGS,
  TRACK_TAGS,
  ALBUM_TAGS,
  XML,
#if defined (BS1770GAIN_TAG_PREFIX) // {
  TAG_PREFIX,
#endif // }
  ////////////////
  MOMENTARY_RANGE,
  MOMENTARY_LENGTH,
  MOMENTARY_OVERLAP,
  MOMENTARY_MEAN_GATE,
  MOMENTARY_RANGE_GATE,
  MOMENTARY_RANGE_LOWER_BOUND,
  MOMENTARY_RANGE_UPPER_BOUND,
  SHORTTERM_MEAN,
  SHORTTERM_LENGTH,
  SHORTTERM_OVERLAP,
  SHORTTERM_MEAN_GATE,
  SHORTTERM_RANGE_GATE,
  SHORTTERM_RANGE_LOWER_BOUND,
  SHORTTERM_RANGE_UPPER_BOUND
};

static const char *bs1770gain_flags="b:d:f:o:u:ahlimprstx";

static struct option bs1770gain_opts[]={
  ////
  { "begin",required_argument,NULL,'b' },
  { "duration",required_argument,NULL,'d' },
  { "file",required_argument,NULL,'f' },
  { "output",required_argument,NULL,'o' },
  { "use",required_argument,NULL,'u' },
  ////
  { "help",no_argument,NULL,'h' },
  { "integrated",no_argument,NULL,'i' },
  { "list",no_argument,NULL,'l' },
  { "momentary",no_argument,NULL,'m' },
  { "range",no_argument,NULL,'r' },
  { "samplepeak",no_argument,NULL,'p' },
  { "shortterm",no_argument,NULL,'s' },
  { "truepeak",no_argument,NULL,'t' },
  { "extensions",no_argument,NULL,'x' },
  ////
  { "apply",required_argument,NULL,APPLY },
  { "unit",required_argument,NULL,UNIT },
  { "audio",required_argument,NULL,AUDIO },
  { "drc",required_argument,NULL,DRC },
  { "extension",required_argument,NULL,EXTENSION },
  { "format",required_argument,NULL,FORMAT },
  { "loglevel",required_argument,NULL,LOGLEVEL },
  { "level",required_argument,NULL,LEVEL },
  { "preamp",required_argument,NULL,PREAMP },
  { "video",required_argument,NULL,VIDEO },
  ////
  { "time",no_argument,NULL,TIME },
  { "ebu",no_argument,NULL,EBU },
  { "atsc",no_argument,NULL,ATSC },
  { "stereo",no_argument,NULL,STEREO },
  { "replaygain",no_argument,NULL,REPLAYGAIN },
  { "rg-tags",no_argument,NULL,RG_TAGS },
  { "bwf-tags",no_argument,NULL,BWF_TAGS },
  { "track-tags",no_argument,NULL,TRACK_TAGS },
  { "album-tags",no_argument,NULL,ALBUM_TAGS },
  { "xml",no_argument,NULL,XML },
#if defined (BS1770GAIN_TAG_PREFIX) // {
  { "tag-prefix",required_argument,NULL,TAG_PREFIX },
#endif // }
  ////
  { "momentary-mean",no_argument,NULL,'i' },
  { "momentary-maximum",no_argument,NULL,'m' },
  { "momentary-range",no_argument,NULL,
      MOMENTARY_RANGE },
  { "momentary-length",required_argument,NULL,
      MOMENTARY_LENGTH },
  { "momentary-overlap",required_argument,NULL,
      MOMENTARY_OVERLAP },
  { "momentary-mean-gate",required_argument,NULL,
      MOMENTARY_MEAN_GATE },
  { "momentary-range-gate",required_argument,NULL,
      MOMENTARY_RANGE_GATE },
  { "momentary-range-lower-bound",required_argument,NULL,
      MOMENTARY_RANGE_LOWER_BOUND },
  { "momentary-range-upper-bound",required_argument,NULL,
      MOMENTARY_RANGE_UPPER_BOUND },
  ////
  { "shortterm-mean",no_argument,NULL,
      SHORTTERM_MEAN },
  { "shortterm-maximum",no_argument,NULL,'s' },
  { "shortterm-range",no_argument,NULL,'r' },
  { "shortterm-length",required_argument,NULL,
      SHORTTERM_LENGTH },
  { "shortterm-overlap",required_argument,NULL,
      SHORTTERM_OVERLAP },
  { "shortterm-mean-gate",required_argument,NULL,
      SHORTTERM_MEAN_GATE },
  { "shortterm-range-gate",required_argument,NULL,
      SHORTTERM_RANGE_GATE },
  { "shortterm-range-lower-bound",required_argument,NULL,
      SHORTTERM_RANGE_LOWER_BOUND },
  { "shortterm-range-upper-bound",required_argument,NULL,
      SHORTTERM_RANGE_UPPER_BOUND },
  ////
  { NULL,0,NULL,0 }
};

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  options_t options;
  FILE *f=stdout;
  tree_t root;
  char *fpath=NULL;
  char *odirname=NULL;
  int loglevel=AV_LOG_QUIET;
  double overlap;
  double t1,t2;
  int c;

  if (1==argc)
    bs1770gain_usage(argv,-1);

  TRACE_PUSH();

  //setlocale(LC_ALL,"C");
  memset(&options,0,sizeof options);
  options.unit="LU";
  options.audio=-1;
  options.video=-1;
  options.level=-23.0;
  options.audio_ext="flac";
  options.video_ext="mkv";
#if defined (BS1770GAIN_TAG_PREFIX) // {
  options.tag_prefix="REPLAYGAIN";
#endif // }

  options.momentary.ms=400.0;
  options.momentary.partition=4;
  options.momentary.mean_gate=-10.0;
  options.momentary.range_gate=-20.0;
  options.momentary.range_lower_bound=0.1;
  options.momentary.range_upper_bound=0.95;

  options.shortterm.ms=3000.0;
  options.shortterm.partition=3;
  options.shortterm.mean_gate=-10.0;
  options.shortterm.range_gate=-20.0;
  options.shortterm.range_lower_bound=0.1;
  options.shortterm.range_upper_bound=0.95;

  opterr=0;

  while (0<=(c=getopt_long(argc,argv,bs1770gain_flags,bs1770gain_opts,NULL))) {
    switch (c) {
    ///////////////////////////////////////////////////////////////////////////
    case '?':
      fprintf(stderr,"Error: Option \"%s\" not recognizsed.\n",argv[optind-1]);
      fprintf(stderr,"\n");
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
      if (0==strcasecmp("integrated",optarg)
          ||0==strcasecmp("momentary-mean",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MEAN;
        options.method=BS1770GAIN_METHOD_MOMENTARY_MEAN;
      }
      else if (0==strcasecmp("momentary",optarg)
          ||0==strcasecmp("momentary-maximum",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MAXIMUM;
        options.method=BS1770GAIN_METHOD_MOMENTARY_MAXIMUM;
      }
      else if (0==strcasecmp("shortterm-mean",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MEAN;
        options.method=BS1770GAIN_METHOD_SHORTTERM_MEAN;
      }
      else if (0==strcasecmp("shortterm",optarg)
          ||0==strcasecmp("shortterm-maximum",optarg)) {
        options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MAXIMUM;
        options.method=BS1770GAIN_METHOD_SHORTTERM_MAXIMUM;
      }
      else {
        fprintf(stderr,"Error: Method \"%s\" not recognized.\n",optarg);
        fprintf(stderr,"\n");
        bs1770gain_usage(argv,-1);
      }

      break;
    /// without argument //////////////////////////////////////////////////////
    case 'a':
      options.mode|=BS1770GAIN_MODE_APPLY;
      options.apply=0.0;
      break;
    case 'h':
      bs1770gain_usage(argv,0);
      break;
    case 'l':
      options.dump=1;
      break;
    case 'i':
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MEAN;
      break;
    case 's':
      options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MAXIMUM;
      break;
    case 'm':
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MAXIMUM;
      break;
    case 'r':
      options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_RANGE;
      break;
    case 'p':
      options.aggregate|=FFSOX_AGGREGATE_SAMPLEPEAK;
      break;
    case 't':
      options.aggregate|=FFSOX_AGGREGATE_TRUEPEAK;
      break;
    case 'x':
      options.extensions=BS1770GAIN_EXTENSION_ALL;
      options.aggregate|=FFSOX_AGGREGATE_TRUEPEAK;
      options.stereo=1;
      break;
    /// without flag //////////////////////////////////////////////////////////
    case AUDIO:
      options.audio=atoi(optarg);
      break;
    case VIDEO:
      options.video=atoi(optarg);
      break;
    case APPLY:
      options.mode|=BS1770GAIN_MODE_APPLY;
      options.apply=atof(optarg);
      break;
    case UNIT:
      options.unit=optarg;
      break;
    case DRC:
      options.drc=atof(optarg);
      break;
    case EXTENSION:
      if (0==strcasecmp("rename",optarg))
        options.extensions|=EXTENSION_RENAME;
      else if (0==strcasecmp("csv",optarg))
        options.extensions|=EXTENSION_CSV;
      else if (0==strcasecmp("jpg",optarg))
        options.extensions|=EXTENSION_JPG;
      else if (0==strcasecmp("tags",optarg))
        options.extensions|=EXTENSION_TAGS;
      else
        bs1770gain_usage(argv,-1);
      break;
#if defined (BS1770GAIN_TAG_PREFIX) // {
    case TAG_PREFIX:
      options.tag_prefix=optarg;
      break;
#endif // }
    case FORMAT:
      options.format=optarg;
      break;
    case LOGLEVEL:
      if (0==strcasecmp("quiet",optarg))
        loglevel=AV_LOG_QUIET;
      else if (0==strcasecmp("panic",optarg))
        loglevel=AV_LOG_PANIC;
      else if (0==strcasecmp("fatal",optarg))
        loglevel=AV_LOG_FATAL;
      else if (0==strcasecmp("error",optarg))
        loglevel=AV_LOG_ERROR;
      else if (0==strcasecmp("warning",optarg))
        loglevel=AV_LOG_WARNING;
      else if (0==strcasecmp("info",optarg))
        loglevel=AV_LOG_INFO;
      else if (0==strcasecmp("verbose",optarg))
        loglevel=AV_LOG_VERBOSE;
      else if (0==strcasecmp("debug",optarg))
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
      options.unit="dB";
      break;
    case RG_TAGS:
      options.mode|=BS1770GAIN_MODE_RG_TAGS;
      break;
    case BWF_TAGS:
      options.mode|=BS1770GAIN_MODE_BWF_TAGS;
      break;
    case TRACK_TAGS:
      options.mode|=BS1770GAIN_MODE_TRACK_TAGS;
      break;
    case ALBUM_TAGS:
      options.mode|=BS1770GAIN_MODE_ALBUM_TAGS;
      break;
    case XML:
      options.xml=1;
      break;
    case TIME:
      options.time=1;
      break;
    case STEREO:
      options.stereo=1;
      break;
    ///////////////////////////////////////////////////////////////////////////
    case MOMENTARY_RANGE:
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_RANGE;
      break;
    ////////
    case MOMENTARY_LENGTH:
      options.momentary.ms=atof(optarg);
      break;
    case MOMENTARY_OVERLAP:
      overlap=atof(optarg);

      if (0.0<=overlap&&overlap<100.0)
        options.momentary.partition=floor(100.0/(100.0-overlap)+0.5);
      else {
        fprintf(stderr,"Error: Overlap out of range [0..100).\n");
        fprintf(stderr,"\n");
        bs1770gain_usage(argv,-1);
      }

      break;
    case MOMENTARY_MEAN_GATE:
      options.momentary.mean_gate=atof(optarg);
      break;
    case MOMENTARY_RANGE_GATE:
      options.momentary.range_gate=atof(optarg);
      break;
    case MOMENTARY_RANGE_LOWER_BOUND:
      options.momentary.range_lower_bound=atof(optarg);
      break;
    case MOMENTARY_RANGE_UPPER_BOUND:
      options.momentary.range_upper_bound=atof(optarg);
      break;
    ////////
    case SHORTTERM_MEAN:
      options.aggregate|=FFSOX_AGGREGATE_SHORTTERM_MEAN;
      break;
    ////////
    case SHORTTERM_LENGTH:
      options.shortterm.ms=atof(optarg);
      break;
    case SHORTTERM_OVERLAP:
      overlap=atof(optarg);

      if (0.0<=overlap&&overlap<100.0)
        options.shortterm.partition=floor(100.0/(100.0-overlap)+0.5);
      else {
        fprintf(stderr,"Error: Overlap out of range [0..100).\n");
        fprintf(stderr,"\n");
        bs1770gain_usage(argv,-1);
      }

      break;
    case SHORTTERM_MEAN_GATE:
      options.shortterm.mean_gate=atof(optarg);
      break;
    case SHORTTERM_RANGE_GATE:
      options.shortterm.range_gate=atof(optarg);
      break;
    case SHORTTERM_RANGE_LOWER_BOUND:
      options.shortterm.range_lower_bound=atof(optarg);
      break;
    case SHORTTERM_RANGE_UPPER_BOUND:
      options.shortterm.range_upper_bound=atof(optarg);
      break;
    ///////////////////////////////////////////////////////////////////////////
    default:
      break;
    }
  }

  if (0!=options.method&&NULL==odirname) {
    fprintf(stderr,"Error: \"-u/--use\" requieres \"-o/--output\".\n");
    fprintf(stderr,"\n");
    bs1770gain_usage(argv,-1);
  }

  if (BS1770GAIN_IS_MODE_APPLY(options.mode)&&NULL!=options.format) {
    fprintf(stderr,"Warning: Format \"%s\" potentially not available.\n",
        options.format);
    options.format=NULL;
  }

  if (options.momentary.partition<1||options.shortterm.partition<1)
    bs1770gain_usage(argv,-1);

  if (options.momentary.range_lower_bound<=0.0
      ||options.momentary.range_upper_bound
          <=options.momentary.range_lower_bound
      ||1.0<=options.momentary.range_upper_bound
      ||options.shortterm.range_lower_bound<=0.0
      ||options.shortterm.range_upper_bound
          <=options.shortterm.range_lower_bound
      ||1.0<=options.shortterm.range_upper_bound) {
    fprintf(stderr,"Error: Range bounds out of range "
        " 0.0 < lower < upper < 1.0.\n");
    fprintf(stderr,"\n");
    bs1770gain_usage(argv,-1);
  }

  if (!BS1770GAIN_IS_MODE_APPLY(options.mode)) {
    if (!BS1770GAIN_IS_MODE_RG_BWF_TAGS(options.mode))
      options.mode|=BS1770GAIN_MODE_RG_TAGS;

    if (!BS1770GAIN_IS_MODE_TRACK_ALBUM_TAGS(options.mode))
      options.mode|=BS1770GAIN_MODE_TRACK_ALBUM_TAGS;
  }

  if (BS1770GAIN_OPTIONS_NO_METHOD(&options)) {
    if (NULL!=odirname||BS1770GAIN_OPTIONS_NO_RANGE_PEAK(&options))
      options.aggregate|=FFSOX_AGGREGATE_MOMENTARY_MEAN;
  }

  // load the FFmpeg and SoX libraries from "bs1770gain-tools".
  if (ffsox_dynload("bs1770gain-tools")<0) {
    PBU_DMESSAGE("loading shared libraries");
    goto dynload;
  }

  av_register_all();
  sox_init();

  if (0==options.dump||av_log_get_level()<loglevel)
    av_log_set_level(loglevel);

  if (fpath&&NULL==(f=fopen(fpath,"w")))
    goto file;

  bs1770gain_tree_cli_init(&root,argc,argv,optind);

  if (options.xml)
    bs1770gain_print_xml(&options.p,f);
  else
    bs1770gain_print_classic(&options.p,f);

  options.p.vmt->session.head(&options.p);
  t1=clock();
  bs1770gain_tree_analyze(&root,odirname,&options);
  t2=clock();

  if (0==root.cli.count)
    fprintf(stderr,"Warning: No valid input files/folders given.\n");

  options.p.vmt->session.tail(&options.p);
  root.vmt->cleanup(&root);

  if (options.time)
    fprintf(stderr, "Duration: %.0f ms.\n",(t2-t1)/CLOCKS_PER_MILLIS);
// cleanup:
  sox_quit();
  // still reachable: 9,689 bytes in 51 blocks
  // TODO: Cleanup FFmpeg. But how?
dynload:
  if (NULL!=fpath)
    fclose(f);
file:
  TRACE_POP();
  PBU_HEAP_PRINT();

  return 0;
}
